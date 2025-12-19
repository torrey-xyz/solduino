/**
 * Solduino Custom Program Demo
 *
 * Demonstrates interacting with an arbitrary Solana smart contract
 * using the instruction builder API.
 *
 * This example shows three patterns:
 *   1. Using SystemProgram helpers  (SOL transfer)
 *   2. Using TokenProgram helpers   (SPL token transfer)
 *   3. Building a fully custom instruction for any on-chain program
 *   4. Composing multiple instructions in a single transaction
 *   5. Deriving a PDA (Program Derived Address)
 *
 * Hardware: ESP32
 *
 * Required Libraries:
 *   - ArduinoJson (via Library Manager)
 *   - Solduino
 *
 * Setup:
 *   1. Update WiFi credentials below
 *   2. Update RPC_ENDPOINT to point at your validator / devnet
 *   3. Upload to ESP32, open Serial Monitor at 115200 baud
 */

#include <WiFi.h>
#include <solduino.h>
#include <ArduinoJson.h>

// ============================================================================
// Configuration
// ============================================================================

const char* ssid     = "YOUR_WIFI_SSID";
const char* password = "YOUR_WIFI_PASSWORD";

// Use localnet (solana-test-validator) or devnet
const String RPC_ENDPOINT = SOLDUINO_DEVNET_RPC;

const uint32_t POLL_INTERVAL_MS  = 1000;
const uint32_t CONFIRM_TIMEOUT_MS = 30000;

// ============================================================================
// Global Objects
// ============================================================================

Solduino solduino;
RpcClient rpcClient(RPC_ENDPOINT);

// Reusable buffer for encoded transactions
static char g_txBuf[2048];

// ============================================================================
// Helper: send a transaction and print the result
// ============================================================================

bool sendAndConfirm(Transaction& tx,
                    const uint8_t* signerPrivateKey,
                    const uint8_t* signerPublicKey) {
    // 1. Fresh blockhash
    uint8_t blockhash[BLOCKHASH_SIZE];
    if (!rpcClient.getLatestBlockhashBytes(blockhash)) {
        Serial.println("  [ERROR] Failed to get blockhash");
        return false;
    }
    tx.setRecentBlockhash(blockhash);

    // 2. Sign
    if (!tx.sign(signerPrivateKey, signerPublicKey)) {
        Serial.println("  [ERROR] Failed to sign transaction");
        return false;
    }

    // 3. Encode
    if (!TransactionSerializer::encodeTransactionBase58(tx, g_txBuf, sizeof(g_txBuf))) {
        Serial.println("  [ERROR] Failed to encode transaction");
        return false;
    }

    // 4. Send
    String resp = rpcClient.sendTransactionBase58(g_txBuf);
    DynamicJsonDocument doc(512);
    deserializeJson(doc, resp);

    if (!doc.containsKey("result")) {
        String err = doc.containsKey("error")
                         ? doc["error"]["message"].as<String>()
                         : "Unknown error";
        Serial.print("  [ERROR] ");
        Serial.println(err);
        return false;
    }

    String sig = doc["result"].as<String>();
    Serial.print("  Signature: ");
    Serial.println(sig);

    // 5. Poll confirmation
    uint32_t start = millis();
    while (millis() - start < CONFIRM_TIMEOUT_MS) {
        TransactionResponse txResp;
        if (rpcClient.getTransaction(sig, txResp)) {
            Serial.println("  Confirmed!");
            return true;
        }
        delay(POLL_INTERVAL_MS);
    }
    Serial.println("  Confirmation timed out (may still finalize)");
    return true;
}

// ============================================================================
// Demo 1: SOL Transfer using SystemProgram helper
// ============================================================================

void demoSystemTransfer() {
    Serial.println("\n--- Demo 1: SystemProgram::transfer ---\n");

    Keypair sender, receiver;
    sender.generate();
    receiver.generate();

    uint8_t senderPub[SOLDUINO_PUBKEY_SIZE], senderPriv[SOLDUINO_SECRETKEY_SIZE];
    uint8_t receiverPub[SOLDUINO_PUBKEY_SIZE];
    sender.getPublicKey(senderPub);
    sender.getPrivateKey(senderPriv);
    receiver.getPublicKey(receiverPub);

    char senderAddr[64];
    sender.getPublicKeyAddress(senderAddr, sizeof(senderAddr));
    Serial.print("  Sender:   ");
    Serial.println(senderAddr);

    // Airdrop to sender (devnet/localnet only)
    rpcClient.requestAirdrop(senderAddr, 1000000000ULL);
    Serial.println("  Airdrop requested, waiting...");
    delay(5000);

    // Build and send the transfer -- ONE line!
    Transaction tx;
    tx.add(SystemProgram::transfer(senderPub, receiverPub, 500000)); // 0.0005 SOL

    sendAndConfirm(tx, senderPriv, senderPub);
    memset(senderPriv, 0, sizeof(senderPriv));
}

// ============================================================================
// Demo 2: Custom Instruction for any smart contract
// ============================================================================

void demoCustomInstruction() {
    Serial.println("\n--- Demo 2: Custom Instruction Builder ---\n");

    // Suppose we have a custom on-chain program that accepts:
    //   Accounts: [authority (signer, writable), data_account (writable)]
    //   Data:     8-byte Anchor discriminator + u64 value
    //
    // This demonstrates how you'd build that instruction.

    Keypair authority;
    authority.generate();

    uint8_t authPub[SOLDUINO_PUBKEY_SIZE], authPriv[SOLDUINO_SECRETKEY_SIZE];
    authority.getPublicKey(authPub);
    authority.getPrivateKey(authPriv);

    // Fake program ID and data account for demonstration
    uint8_t programId[SOLDUINO_PUBKEY_SIZE];
    uint8_t dataAccount[SOLDUINO_PUBKEY_SIZE];
    memset(programId, 0xAA, SOLDUINO_PUBKEY_SIZE);   // placeholder
    memset(dataAccount, 0xBB, SOLDUINO_PUBKEY_SIZE);  // placeholder

    // Example Anchor discriminator (first 8 bytes of SHA-256 of "global:initialize")
    uint8_t discriminator[8] = {0xaf, 0xaf, 0x6d, 0x1f, 0x0d, 0x98, 0x9b, 0xed};

    // Build the instruction
    Instruction ix;
    ix.setProgram(programId);
    ix.addKey(authPub, true, true);       // authority: signer + writable
    ix.addKey(dataAccount, false, true);  // data account: writable

    // Encode instruction data
    ix.writeBytes(discriminator, 8);      // 8-byte Anchor discriminator
    ix.writeU64LE(42);                    // our custom u64 value

    Serial.print("  Instruction data length: ");
    Serial.print(ix.getDataLength());
    Serial.println(" bytes");
    Serial.print("  Account keys: ");
    Serial.println(ix.getKeyCount());

    // Add to transaction
    Transaction tx;
    tx.add(ix);

    Serial.println("  Transaction built successfully");
    Serial.println("  (Not sending -- program ID is a placeholder)");

    memset(authPriv, 0, sizeof(authPriv));
}

// ============================================================================
// Demo 3: Multi-instruction transaction
// ============================================================================

void demoMultiInstruction() {
    Serial.println("\n--- Demo 3: Multi-Instruction Transaction ---\n");

    Keypair payer, recipient1, recipient2;
    payer.generate();
    recipient1.generate();
    recipient2.generate();

    uint8_t payerPub[SOLDUINO_PUBKEY_SIZE];
    uint8_t r1Pub[SOLDUINO_PUBKEY_SIZE];
    uint8_t r2Pub[SOLDUINO_PUBKEY_SIZE];
    payer.getPublicKey(payerPub);
    recipient1.getPublicKey(r1Pub);
    recipient2.getPublicKey(r2Pub);

    // Build a transaction with TWO transfer instructions
    Transaction tx;
    tx.add(SystemProgram::transfer(payerPub, r1Pub, 100000));  // 0.0001 SOL
    tx.add(SystemProgram::transfer(payerPub, r2Pub, 200000));  // 0.0002 SOL

    Serial.print("  Instructions in tx: ");
    Serial.println(tx.getMessage().getInstructionCount());
    Serial.print("  Accounts in tx:     ");
    Serial.println(tx.getMessage().getAccountCount());
    Serial.println("  Multi-instruction transaction built successfully!");
}

// ============================================================================
// Demo 4: PDA Derivation
// ============================================================================

void demoPDA() {
    Serial.println("\n--- Demo 4: PDA Derivation ---\n");

    // Derive a PDA for a hypothetical program with seeds ["data", userPubkey]
    Keypair user;
    user.generate();
    uint8_t userPub[SOLDUINO_PUBKEY_SIZE];
    user.getPublicKey(userPub);

    uint8_t programId[SOLDUINO_PUBKEY_SIZE];
    memset(programId, 0xCC, SOLDUINO_PUBKEY_SIZE); // placeholder program

    const uint8_t seed1[] = "data";
    const uint8_t* seeds[] = { seed1, userPub };
    const size_t seedLens[] = { 4, SOLDUINO_PUBKEY_SIZE };

    uint8_t pda[SOLDUINO_PUBKEY_SIZE];
    uint8_t bump;

    if (findProgramAddress(seeds, seedLens, 2, programId, pda, &bump)) {
        char pdaAddr[64];
        publicKeyToAddress(pda, pdaAddr, sizeof(pdaAddr));
        Serial.print("  PDA:  ");
        Serial.println(pdaAddr);
        Serial.print("  Bump: ");
        Serial.println(bump);
    } else {
        Serial.println("  [ERROR] PDA derivation failed");
    }
}

// ============================================================================
// Setup & Loop
// ============================================================================

void connectToWiFi() {
    WiFi.mode(WIFI_STA);
    WiFi.begin(ssid, password);
    int attempts = 0;
    while (WiFi.status() != WL_CONNECTED && attempts < 20) {
        delay(500);
        Serial.print(".");
        attempts++;
    }
    Serial.println(WiFi.status() == WL_CONNECTED ? " Connected!" : " Failed!");
}

void setup() {
    Serial.begin(115200);
    delay(1000);

    Serial.println("\n=== Solduino Custom Program Demo ===\n");

    solduino.begin();
    Serial.println("Connecting to WiFi...");
    connectToWiFi();

    if (WiFi.status() != WL_CONNECTED) {
        Serial.println("[ERROR] WiFi connection failed!");
        return;
    }

    rpcClient.begin();
    rpcClient.setTimeout(15000);

    // Run demos
    demoSystemTransfer();    // Pattern 1: SystemProgram helper
    demoCustomInstruction(); // Pattern 2: Custom instruction builder
    demoMultiInstruction();  // Pattern 3: Multiple instructions
    demoPDA();               // Pattern 4: PDA derivation

    Serial.println("\n=== All Demos Complete ===");
}

void loop() {
    delay(1000);
}

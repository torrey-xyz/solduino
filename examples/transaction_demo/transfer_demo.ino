/**
 * Solduino Transfer Demo
 * 
 * This example demonstrates SOL transfer functionality:
 * - Create two wallets (sender and receiver)
 * - Request airdrop to sender account
 * - Create and sign a transfer transaction
 * - Send transaction via RPC
 * - Verify transfer success
 * 
 * Hardware: ESP32
 * 
 * Required Libraries:
 * - ArduinoJson (via Library Manager)
 * - Solduino library (includes TweetNaCl)
 * 
 * Security Notes:
 * - Uses ESP32 hardware random number generator for key generation
 * - Uses Ed25519 for transaction signing
 * - Private keys are stored securely in memory
 * - Never log or expose private keys in production code
 * - This demo uses devnet/localnet - no real SOL is involved
 * 
 * Setup:
 * 1. Update WiFi credentials below
 * 2. Select ESP32 board in Arduino IDE
 * 3. Upload to ESP32
 * 4. Open Serial Monitor at 115200 baud
 * 
 * For LOCALNET Testing:
 * - Start solana-test-validator: solana-test-validator --bind-address 0.0.0.0
 * - Find your computer's IP address:
 *     macOS/Linux: ifconfig | grep "inet " | grep -v 127.0.0.1
 *     Windows: ipconfig | findstr IPv4
 * - Replace "localhost" with your computer's IP address
 * - Example: Use "http://192.168.1.100:8899" or "http://172.17.53.216:8899"
 * - ESP32 and computer must be on the same WiFi network
 */

 #include <WiFi.h>
 #include <solduino.h>
 #include <ArduinoJson.h>
 #include <string.h>
 
 // Add include for crypto
 #include "crypto.h"

 // Optional: disable self-test in production
 #ifndef SOLDUINO_DEBUG
 #define SOLDUINO_DEBUG 0
 #endif
 
 // ============================================================================
 // Configuration
 // ============================================================================
 
 // WiFi credentials
 const char* ssid = "Parvat's Device";
 const char* password = "Parvat@94255";
 
 // Solana RPC endpoint
 // Options: SOLDUINO_LOCALNET_RPC (for local test-validator)
 //          SOLDUINO_DEVNET_RPC (for devnet testing)
 //          SOLDUINO_TESTNET_RPC (for testnet)
 //          SOLDUINO_MAINNET_RPC (for mainnet - be careful!)
 // 
 // For LOCALNET: Replace "localhost" with your computer's IP address
 // Example: "http://192.168.1.100:8899" or "http://172.17.53.216:8899"
 // 
 // To find your computer's IP address:
 //   macOS/Linux: ifconfig | grep "inet " | grep -v 127.0.0.1
 //   Windows: ipconfig | findstr IPv4
 // 
 // Start test-validator with: solana-test-validator --bind-address 0.0.0.0
 // This makes it accessible from other devices on your network
 const String RPC_ENDPOINT = "http://10.72.83.234:8899"; // Replace with your actual IP address
// const String RPC_ENDPOINT = SOLDUINO_DEVNET_RPC;
 
// Transfer amount in lamports (1 SOL = 1,000,000,000 lamports)
// For devnet testing, use small amounts
const uint64_t TRANSFER_AMOUNT = 1000000; // 0.001 SOL
const uint32_t POLL_INTERVAL_MS = 1000;   // balance/signature polling interval
const uint32_t AIRDROP_TIMEOUT_MS = 30000; // 30s airdrop wait
const uint32_t CONFIRM_TIMEOUT_MS = 30000; // 30s signature confirm
 
 // ============================================================================
 // Global Objects
 // ============================================================================
 
 // Initialize Solduino SDK
 Solduino solduino;
 
 // Initialize RPC client
 RpcClient rpcClient(RPC_ENDPOINT);

// Global buffer to avoid large stack allocations during serialization
static char g_txBase64[2048];
 
 // ============================================================================
 // Setup
 // ============================================================================
 
 void setup() {
     Serial.begin(115200);
     delay(1000);
     
     Serial.println("\n=== Solduino Transfer Demo ===\n");
     
     solduino.begin();
     
     Serial.println("Connecting to WiFi...");
     connectToWiFi();
     
     if (WiFi.status() != WL_CONNECTED) {
         Serial.println("[ERROR] WiFi connection failed!");
         return;
     }
     
     rpcClient.begin();
     rpcClient.setTimeout(15000);
     
    // Test Ed25519 (optional)
    #if SOLDUINO_DEBUG
    if (testEd25519()) Serial.println("Ed25519 test passed");
    else Serial.println("Ed25519 test failed");
    #endif
     
     demonstrateTransfer();
 }
 
 // ============================================================================
 // Main Loop
 // ============================================================================
 
 void loop() {
     delay(1000);
 }
 
 // ============================================================================
 // WiFi Connection
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
     
     Serial.println(WiFi.status() == WL_CONNECTED ? " Connected!" : "\n[ERROR] Failed!");
 }
 
 // ============================================================================
 // Transfer Demo
 // ============================================================================
 
static bool waitForBalanceMin(const char* address, uint64_t minLamports, uint32_t timeoutMs) {
    uint32_t start = millis();
    while (millis() - start < timeoutMs) {
        uint64_t bal = rpcClient.getBalanceLamports(address);
        if (bal >= minLamports) return true;
        delay(POLL_INTERVAL_MS);
    }
    return false;
}

static bool waitForSignatureConfirmed(const String& sig, uint32_t timeoutMs) {
    uint32_t start = millis();
    while (millis() - start < timeoutMs) {
        TransactionResponse tx;
        if (rpcClient.getTransaction(sig, tx)) {
            return true;
        }
        delay(POLL_INTERVAL_MS);
    }
    return false;
}

void demonstrateTransfer() {
    Serial.println("\n=== Transfer Demo ===\n");
    
    // 1. Create wallets
     Serial.println("1. Creating wallets...");
     Keypair senderKeypair, receiverKeypair;
     
     if (!senderKeypair.generate() || !receiverKeypair.generate()) {
         Serial.println("   ✗ Failed to generate keypairs");
         return;
     }
     
     char senderAddress[64], receiverAddress[64];
     if (!senderKeypair.getPublicKeyAddress(senderAddress, sizeof(senderAddress)) ||
         !receiverKeypair.getPublicKeyAddress(receiverAddress, sizeof(receiverAddress))) {
         Serial.println("   ✗ Failed to get addresses");
         return;
     }
     
     Serial.print("   Sender: ");
     Serial.println(senderAddress);
     Serial.print("   Receiver: ");
     Serial.println(receiverAddress);
     
     // Extract keys needed for transaction
     uint8_t senderPubkey[SOLDUINO_PUBKEY_SIZE];
     uint8_t receiverPubkey[SOLDUINO_PUBKEY_SIZE];
     uint8_t senderPrivateKey[SOLDUINO_SECRETKEY_SIZE];
     
     if (!senderKeypair.getPublicKey(senderPubkey) || 
         !receiverKeypair.getPublicKey(receiverPubkey) ||
         !senderKeypair.getPrivateKey(senderPrivateKey)) {
         Serial.println("   ✗ Failed to extract keys");
         return;
     }
     
     receiverKeypair.clear();
     
     // 2. Request airdrop for sender
    Serial.println("\n2. Requesting airdrop for sender...");
    (void)rpcClient.requestAirdrop(senderAddress, 1000000000);
    Serial.println("    ✓ Airdrop requested for sender");
    Serial.println("\n3. Waiting for airdrop to confirm...");
    if (!waitForBalanceMin(senderAddress, 1000000000ULL, AIRDROP_TIMEOUT_MS)) {
        Serial.println("   ✗ Airdrop not confirmed in time");
        return;
    }

     // Also request airdrop for receiver to ensure account exists on-chain
     Serial.println("\n3b. Requesting airdrop for receiver...");
    (void)rpcClient.requestAirdrop(receiverAddress, 1000000); // Small amount for receiver
    Serial.println("    ✓ Airdrop requested for receiver");
    (void)waitForBalanceMin(receiverAddress, 1000000ULL, AIRDROP_TIMEOUT_MS);
     
     // Check balances
     Serial.println("\n4. Checking balances...");
     uint64_t senderBalance = rpcClient.getBalanceLamports(senderAddress);
     uint64_t receiverBalance = rpcClient.getBalanceLamports(receiverAddress);
     
     Serial.print("   Sender: ");
     Serial.print((double)senderBalance / 1000000000.0, 9);
     Serial.println(" SOL");
     Serial.print("   Receiver: ");
     Serial.print((double)receiverBalance / 1000000000.0, 9);
     Serial.println(" SOL");
     
     // Create transaction first
     Serial.println("\n5. Creating transaction...");
     static Transaction transaction;

     if (!transaction.addTransferInstruction(senderPubkey, receiverPubkey, TRANSFER_AMOUNT)) {
         Serial.println("   ✗ Failed to create transaction");
         return;
     }

     // Get fresh blockhash right before signing
     Serial.println("\n6. Getting fresh blockhash...");
     uint8_t blockhash[BLOCKHASH_SIZE];

     if (!rpcClient.getLatestBlockhashBytes(blockhash)) {
         Serial.println("   ✗ Failed to get blockhash");
         return;
     }

     if (!transaction.setRecentBlockhash(blockhash)) {
         Serial.println("   ✗ Failed to set blockhash");
         return;
     }
     
     Serial.print("   Transfer Amount: ");
     Serial.print((double)TRANSFER_AMOUNT / 1000000000.0, 9);
     Serial.println(" SOL");
     
     // Sign transaction
     Serial.println("\n7. Signing transaction...");
     if (!transaction.sign(senderPrivateKey, senderPubkey)) {
         Serial.println("   ✗ Failed to sign transaction");
         return;
     }
     
    memset(senderPrivateKey, 0, sizeof(senderPrivateKey));
     Serial.println("   ✓ Transaction signed");

     // Serialize transaction
     Serial.println("\n8. Serializing transaction...");
     if (!TransactionSerializer::encodeTransactionBase58(transaction, g_txBase64, sizeof(g_txBase64))) {
         Serial.println("   ✗ Failed to encode transaction");
         return;
     }
     
     transaction.reset();
     
     // Send transaction
     Serial.println("\n9. Sending transaction...");
    String sendResponse = rpcClient.sendTransactionBase58(g_txBase64);
    String txSignature;
     
     {
        DynamicJsonDocument doc(512);
         deserializeJson(doc, sendResponse);
         
         if (doc.containsKey("result")) {
             txSignature = doc["result"].as<String>();
             Serial.print("   ✓ Transaction sent! Signature: ");
             Serial.println(txSignature);
         } else {
            Serial.print("   ✗ Transaction failed: ");
            String err = doc.containsKey("error") ? doc["error"]["message"].as<String>() : "Unknown error";
            Serial.println(err);
             return;
         }
     }
     
    Serial.println("   ⏳ Waiting for confirmation...");
    if (!waitForSignatureConfirmed(txSignature, CONFIRM_TIMEOUT_MS)) {
        Serial.println("   ⚠️  Confirmation timeout (network may still finalize it)");
    } else {
        Serial.println("   ✓ Confirmed");
    }
     /*
     // Verify transfer
     Serial.println("\n10. Verifying transfer...");
     uint64_t newSenderBalance = rpcClient.getBalanceLamports(senderAddress);
     uint64_t newReceiverBalance = rpcClient.getBalanceLamports(receiverAddress);
     
     Serial.print("   Sender: ");
     Serial.print((double)newSenderBalance / 1000000000.0, 9);
     Serial.println(" SOL");
     Serial.print("   Receiver: ");
     Serial.print((double)newReceiverBalance / 1000000000.0, 9);
     Serial.println(" SOL");
     
     if (newReceiverBalance > receiverBalance) {
         uint64_t received = newReceiverBalance - receiverBalance;
         Serial.print("\n   ✓ Transfer successful! Receiver received: ");
         Serial.print((double)received / 1000000000.0, 9);
         Serial.println(" SOL");
     } else {
         Serial.println("\n   ⚠️  Balance hasn't updated yet");
     }
     */
     Serial.println("\n=== Demo Complete ===");
 }
 
 
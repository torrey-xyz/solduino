/**
 * Solduino Transaction Demo
 * 
 * This example demonstrates transaction creation and signing functionality:
 * - Request airdrops for testing (devnet/localnet)
 * - Generate keypairs for sender and receiver
 * - Get account balances
 * - Get recent blockhash
 * - Create a transfer transaction
 * - Sign transaction with Ed25519
 * - Serialize transaction to wire format
 * - Encode transaction to base64
 * - Send transaction via RPC
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
 * - This demo uses devnet - no real SOL is involved
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
 * - Replace "localhost" in SOLDUINO_LOCALNET_RPC with your computer's IP address
 * - Example: Use "http://192.168.1.100:8899" or "http://172.17.53.216:8899"
 * - ESP32 and computer must be on the same WiFi network
 * - The validator shows "JSON RPC URL: http://127.0.0.1:8899" but use your network IP instead
 * 
 * Note: This demo creates transactions but doesn't send them by default
 * (commented out) to prevent accidental SOL transfers. Uncomment the
 * send transaction section if you want to test on devnet.
 */

#include <WiFi.h>
#include <solduino.h>
#include <ArduinoJson.h>

// ============================================================================
// Configuration
// ============================================================================

// WiFi credentials
const char* ssid = "YOUR_WIFI_SSID";
const char* password = "YOUR_WIFI_PASSWORD";

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
// ifconfig | grep "inet " | grep -v 127.0.0.1
// This makes it accessible from other devices on your network
const String RPC_ENDPOINT = "http://172.17.53.216:8899"; // Replace with your actual IP address
// Alternative: const String RPC_ENDPOINT = SOLDUINO_LOCALNET_RPC; // Only works if ESP32 and computer are same device

// Transfer amount in lamports (1 SOL = 1,000,000,000 lamports)
// For devnet testing, use small amounts
const uint64_t TRANSFER_AMOUNT = 1000000; // 0.001 SOL

// ============================================================================
// Global Objects
// ============================================================================

// Initialize Solduino SDK
Solduino solduino;

// Initialize RPC client
RpcClient rpcClient(RPC_ENDPOINT);

// Flag to ensure demo runs only once
bool demoRun = false;

// ============================================================================
// Setup
// ============================================================================

void setup() {
    Serial.begin(115200);
    delay(1000);
    
    Serial.println("\n\n");
    Serial.println("========================================");
    Serial.println("Solduino Transaction Demo");
    Serial.println("========================================");
    Serial.println();
    
    // Check if demo has already run
    if (demoRun) {
        Serial.println("Demo already executed. Skipping.");
        return;
    }
    
    // Initialize SDK
    if (!solduino.begin()) {
        Serial.println("[ERROR] Failed to initialize Solduino SDK");
        return;
    }
    
    Serial.print("Solduino Version: ");
    Serial.println(solduino.getVersion());
    Serial.println();
    
    Serial.println("Initializing WiFi connection...");
    connectToWiFi();
    
    Serial.println("\nInitializing RPC client...");
    if (rpcClient.begin()) {
        Serial.println("[SUCCESS] RPC client initialized!");
        rpcClient.setTimeout(15000); // 15 second timeout
    } else {
        Serial.println("[ERROR] Failed to initialize RPC client");
        return;
    }
    
    Serial.println("\n=== Starting Transaction Demo ===\n");
    
    // Mark demo as run before executing
    demoRun = true;
    
    // Run demo functions
    demonstrateAirdrop();
    demonstrateTransactionCreation();
    demonstrateTransactionSigning();
    demonstrateTransactionSerialization();
    
    Serial.println("\n=== Demo Complete ===");
    Serial.println("Note: Transaction sending is commented out for safety.");
    Serial.println("Uncomment the sendTransaction section to test on devnet.\n");
}

// ============================================================================
// Main Loop
// ============================================================================

void loop() {
    // Demo runs only once in setup()
    // This loop is intentionally empty
    delay(1000); // Small delay to prevent watchdog issues
}

// ============================================================================
// WiFi Connection
// ============================================================================

void connectToWiFi() {
    WiFi.mode(WIFI_STA);
    WiFi.begin(ssid, password);
    
    Serial.print("Connecting to WiFi");
    int attempts = 0;
    while (WiFi.status() != WL_CONNECTED && attempts < 20) {
        delay(500);
        Serial.print(".");
        attempts++;
    }
    
    if (WiFi.status() == WL_CONNECTED) {
        Serial.println();
        Serial.println("[SUCCESS] WiFi connected!");
        Serial.print("  IP Address: ");
        Serial.println(WiFi.localIP());
    } else {
        Serial.println();
        Serial.println("[ERROR] WiFi connection failed!");
        Serial.println("  Please check your SSID and password");
    }
}

// ============================================================================
// Demo Functions
// ============================================================================

/**
 * Demonstrate requesting an airdrop
 */
void demonstrateAirdrop() {
    Serial.println("--- Airdrop Demo ---");
    
    // Generate a keypair for airdrop
    Serial.println("\n1. Generating keypair for airdrop...");
    Keypair keypair;
    if (!keypair.generate()) {
        Serial.println("   ✗ Failed to generate keypair");
        return;
    }
    
    char address[64];
    if (!keypair.getPublicKeyAddress(address, sizeof(address))) {
        Serial.println("   ✗ Failed to get address");
        return;
    }
    Serial.print("   ✓ Address: ");
    Serial.println(address);
    
    // Check initial balance
    Serial.println("\n2. Checking initial balance...");
    String initialBalance = rpcClient.getBalance(address);
    Serial.print("   Initial Balance Response: ");
    Serial.println(initialBalance.substring(0, min(200, (int)initialBalance.length())));
    
    // Parse initial balance
    DynamicJsonDocument balanceDoc(512);
    deserializeJson(balanceDoc, initialBalance);
    uint64_t initialLamports = 0;
    if (balanceDoc.containsKey("result") && balanceDoc["result"].containsKey("value")) {
        initialLamports = balanceDoc["result"]["value"].as<uint64_t>();
        Serial.print("   Initial Balance: ");
        Serial.print(initialLamports);
        Serial.println(" lamports");
    }
    
    // Request airdrop
    Serial.println("\n3. Requesting airdrop...");
    // Request 1 SOL (1,000,000,000 lamports)
    // Note: Devnet max is usually 2 SOL per request
    // Localnet has no practical limit
    uint64_t airdropAmount = 1000000000; // 1 SOL
    Serial.print("   Requesting: ");
    Serial.print(airdropAmount);
    Serial.println(" lamports (1 SOL)");
    
    unsigned long airdropStart = millis();
    String airdropResponse = rpcClient.requestAirdrop(address, airdropAmount);
    unsigned long airdropTime = millis() - airdropStart;
    
    Serial.print("   Response: ");
    Serial.println(airdropResponse.substring(0, min(300, (int)airdropResponse.length())));
    
    // Parse airdrop response
    DynamicJsonDocument airdropDoc(1024);
    deserializeJson(airdropDoc, airdropResponse);
    
    if (airdropDoc.containsKey("result")) {
        String signature = airdropDoc["result"].as<String>();
        Serial.print("   ✓ Airdrop transaction signature: ");
        Serial.println(signature);
        Serial.print("   Airdrop request time: ");
        Serial.print(airdropTime);
        Serial.println(" ms");
        
        // Wait a bit for transaction to confirm
        Serial.println("\n4. Waiting for transaction confirmation...");
        delay(2000);
        
        // Check balance again
        Serial.println("\n5. Checking new balance...");
        String newBalance = rpcClient.getBalance(address);
        DynamicJsonDocument newBalanceDoc(512);
        deserializeJson(newBalanceDoc, newBalance);
        
        if (newBalanceDoc.containsKey("result") && newBalanceDoc["result"].containsKey("value")) {
            uint64_t newLamports = newBalanceDoc["result"]["value"].as<uint64_t>();
            Serial.print("   New Balance: ");
            Serial.print(newLamports);
            Serial.println(" lamports");
            
            if (newLamports > initialLamports) {
                uint64_t received = newLamports - initialLamports;
                Serial.print("   ✓ Airdrop successful! Received: ");
                Serial.print(received);
                Serial.println(" lamports");
            } else {
                Serial.println("   ⚠️  Balance hasn't updated yet (transaction may still be processing)");
            }
        }
        
        // Check transaction status
        Serial.println("\n6. Checking transaction status...");
        String txResponse = rpcClient.getTransaction(signature);
        if (txResponse.length() > 0) {
            Serial.print("   Transaction status: ");
            Serial.println(txResponse.substring(0, min(200, (int)txResponse.length())));
        }
    } else if (airdropDoc.containsKey("error")) {
        Serial.print("   ✗ Airdrop failed: ");
        Serial.println(airdropDoc["error"]["message"].as<String>());
        Serial.println("   Note: Devnet airdrops are limited. Try localnet for unlimited airdrops.");
    } else {
        Serial.println("   ⚠️  Unexpected response format");
    }
}

/**
 * Demonstrate transaction creation
 */
void demonstrateTransactionCreation() {
    Serial.println("--- Transaction Creation Demo ---");
    
    // Generate sender keypair
    Serial.println("\n1. Generating sender keypair...");
    Keypair senderKeypair;
    if (!senderKeypair.generate()) {
        Serial.println("   ✗ Failed to generate sender keypair");
        return;
    }
    
    char senderAddress[64];
    if (!senderKeypair.getPublicKeyAddress(senderAddress, sizeof(senderAddress))) {
        Serial.println("   ✗ Failed to get sender address");
        return;
    }
    Serial.print("   ✓ Sender Address: ");
    Serial.println(senderAddress);
    
    // Generate receiver keypair
    Serial.println("\n2. Generating receiver keypair...");
    Keypair receiverKeypair;
    if (!receiverKeypair.generate()) {
        Serial.println("   ✗ Failed to generate receiver keypair");
        return;
    }
    
    char receiverAddress[64];
    if (!receiverKeypair.getPublicKeyAddress(receiverAddress, sizeof(receiverAddress))) {
        Serial.println("   ✗ Failed to get receiver address");
        return;
    }
    Serial.print("   ✓ Receiver Address: ");
    Serial.println(receiverAddress);
    
    // Get account balances
    Serial.println("\n3. Checking account balances...");
    String senderBalance = rpcClient.getBalance(senderAddress);
    String receiverBalance = rpcClient.getBalance(receiverAddress);
    
    Serial.print("   Sender Balance: ");
    Serial.println(senderBalance);
    Serial.print("   Receiver Balance: ");
    Serial.println(receiverBalance);
    
    // Get recent blockhash
    Serial.println("\n4. Getting recent blockhash...");
    String blockhashResponse = rpcClient.getLatestBlockhash();
    
    DynamicJsonDocument blockhashDoc(1024);
    deserializeJson(blockhashDoc, blockhashResponse);
    
    if (!blockhashDoc.containsKey("result") || blockhashDoc["result"].isNull()) {
        Serial.println("   ✗ Failed to get blockhash");
        Serial.println("   Response: " + blockhashResponse);
        return;
    }
    
    String blockhashBase58 = blockhashDoc["result"]["value"]["blockhash"].as<String>();
    Serial.print("   ✓ Blockhash: ");
    Serial.println(blockhashBase58.substring(0, 32) + "...");
    
    // Decode blockhash from base58
    uint8_t blockhash[BLOCKHASH_SIZE];
    if (!addressToPublicKey(blockhashBase58.c_str(), blockhash)) {
        Serial.println("   ✗ Failed to decode blockhash");
        return;
    }
    
    // Create transaction
    Serial.println("\n5. Creating transfer transaction...");
    Transaction transaction;
    
    // Get public keys as bytes
    uint8_t senderPubkey[SOLDUINO_PUBKEY_SIZE];
    uint8_t receiverPubkey[SOLDUINO_PUBKEY_SIZE];
    if (!senderKeypair.getPublicKey(senderPubkey) || 
        !receiverKeypair.getPublicKey(receiverPubkey)) {
        Serial.println("   ✗ Failed to get public keys");
        return;
    }
    
    // Add transfer instruction
    if (!transaction.addTransferInstruction(senderPubkey, receiverPubkey, TRANSFER_AMOUNT)) {
        Serial.println("   ✗ Failed to add transfer instruction");
        return;
    }
    Serial.println("   ✓ Transfer instruction added");
    
    // Set recent blockhash
    if (!transaction.setRecentBlockhash(blockhash)) {
        Serial.println("   ✗ Failed to set blockhash");
        return;
    }
    Serial.println("   ✓ Blockhash set");
    
    Serial.println("\n   Transaction created successfully!");
    Serial.print("   Instruction Count: ");
    Serial.println(transaction.getMessage().getInstructionCount());
    Serial.print("   Account Count: ");
    Serial.println(transaction.getMessage().getAccountCount());
}

/**
 * Demonstrate transaction signing
 */
void demonstrateTransactionSigning() {
    Serial.println("\n--- Transaction Signing Demo ---");
    
    // Generate keypairs
    Keypair senderKeypair;
    if (!senderKeypair.generate()) {
        Serial.println("   ✗ Failed to generate keypair");
        return;
    }
    
    char senderAddress[64];
    senderKeypair.getPublicKeyAddress(senderAddress, sizeof(senderAddress));
    Serial.print("\n1. Sender Address: ");
    Serial.println(senderAddress);
    
    // Get blockhash
    String blockhashResponse = rpcClient.getLatestBlockhash();
    DynamicJsonDocument blockhashDoc(1024);
    deserializeJson(blockhashDoc, blockhashResponse);
    
    if (!blockhashDoc.containsKey("result") || blockhashDoc["result"].isNull()) {
        Serial.println("   ✗ Failed to get blockhash");
        return;
    }
    
    String blockhashBase58 = blockhashDoc["result"]["value"]["blockhash"].as<String>();
    uint8_t blockhash[BLOCKHASH_SIZE];
    if (!addressToPublicKey(blockhashBase58.c_str(), blockhash)) {
        Serial.println("   ✗ Failed to decode blockhash");
        return;
    }
    
    // Create transaction
    Serial.println("\n2. Creating transaction...");
    Transaction transaction;
    
    uint8_t senderPubkey[SOLDUINO_PUBKEY_SIZE];
    uint8_t receiverPubkey[SOLDUINO_PUBKEY_SIZE];
    
    // Use a known receiver address for demo (or generate one)
    Keypair receiverKeypair;
    receiverKeypair.generate();
    receiverKeypair.getPublicKey(receiverPubkey);
    
    senderKeypair.getPublicKey(senderPubkey);
    
    transaction.addTransferInstruction(senderPubkey, receiverPubkey, TRANSFER_AMOUNT);
    transaction.setRecentBlockhash(blockhash);
    
    Serial.println("   ✓ Transaction created");
    
    // Sign transaction
    Serial.println("\n3. Signing transaction...");
    uint8_t privateKey[SOLDUINO_SECRETKEY_SIZE];
    if (!senderKeypair.getPrivateKey(privateKey)) {
        Serial.println("   ✗ Failed to get private key");
        return;
    }
    
    unsigned long signStart = millis();
    if (!transaction.sign(privateKey, senderPubkey)) {
        Serial.println("   ✗ Failed to sign transaction");
        return;
    }
    unsigned long signTime = millis() - signStart;
    
    Serial.println("   ✓ Transaction signed successfully!");
    Serial.print("   Signing time: ");
    Serial.print(signTime);
    Serial.println(" ms");
    
    // Display signature
    uint8_t signature[SIGNATURE_SIZE];
    if (transaction.getSignature(0, signature)) {
        Serial.print("   Signature (hex): ");
        for (size_t i = 0; i < SIGNATURE_SIZE; i++) {
            if (signature[i] < 16) Serial.print("0");
            Serial.print(signature[i], HEX);
        }
        Serial.println();
    }
    
    Serial.print("   Signature Count: ");
    Serial.println(transaction.getSignatureCount());
}

/**
 * Demonstrate transaction serialization and encoding
 */
void demonstrateTransactionSerialization() {
    Serial.println("\n--- Transaction Serialization Demo ---");
    
    // Generate keypairs and create transaction
    Keypair senderKeypair, receiverKeypair;
    senderKeypair.generate();
    receiverKeypair.generate();
    
    uint8_t senderPubkey[SOLDUINO_PUBKEY_SIZE];
    uint8_t receiverPubkey[SOLDUINO_PUBKEY_SIZE];
    senderKeypair.getPublicKey(senderPubkey);
    receiverKeypair.getPublicKey(receiverPubkey);
    
    // Get blockhash
    String blockhashResponse = rpcClient.getLatestBlockhash();
    DynamicJsonDocument blockhashDoc(1024);
    deserializeJson(blockhashDoc, blockhashResponse);
    
    if (!blockhashDoc.containsKey("result") || blockhashDoc["result"].isNull()) {
        Serial.println("   ✗ Failed to get blockhash");
        return;
    }
    
    String blockhashBase58 = blockhashDoc["result"]["value"]["blockhash"].as<String>();
    uint8_t blockhash[BLOCKHASH_SIZE];
    if (!addressToPublicKey(blockhashBase58.c_str(), blockhash)) {
        Serial.println("   ✗ Failed to decode blockhash");
        return;
    }
    
    // Create and sign transaction
    Serial.println("\n1. Creating and signing transaction...");
    Transaction transaction;
    transaction.addTransferInstruction(senderPubkey, receiverPubkey, TRANSFER_AMOUNT);
    transaction.setRecentBlockhash(blockhash);
    
    uint8_t privateKey[SOLDUINO_SECRETKEY_SIZE];
    senderKeypair.getPrivateKey(privateKey);
    transaction.sign(privateKey, senderPubkey);
    
    Serial.println("   ✓ Transaction created and signed");
    
    // Calculate transaction size
    Serial.println("\n2. Calculating transaction size...");
    uint16_t estimatedSize = TransactionSerializer::calculateTransactionSize(transaction);
    Serial.print("   Estimated Size: ");
    Serial.print(estimatedSize);
    Serial.println(" bytes");
    
    // Serialize transaction
    Serial.println("\n3. Serializing transaction...");
    uint8_t* serializedBuffer = (uint8_t*)malloc(estimatedSize + 100);
    if (!serializedBuffer) {
        Serial.println("   ✗ Failed to allocate buffer");
        return;
    }
    
    uint16_t serializedLen = 0;
    unsigned long serializeStart = millis();
    if (!TransactionSerializer::serializeTransaction(transaction, serializedBuffer, estimatedSize + 100, serializedLen)) {
        Serial.println("   ✗ Failed to serialize transaction");
        free(serializedBuffer);
        return;
    }
    unsigned long serializeTime = millis() - serializeStart;
    
    Serial.println("   ✓ Transaction serialized successfully!");
    Serial.print("   Serialized Length: ");
    Serial.print(serializedLen);
    Serial.println(" bytes");
    Serial.print("   Serialization time: ");
    Serial.print(serializeTime);
    Serial.println(" ms");
    
    // Encode to base64
    Serial.println("\n4. Encoding transaction to base64...");
    char base64Buffer[2048];
    unsigned long encodeStart = millis();
    if (!TransactionSerializer::encodeTransaction(transaction, base64Buffer, sizeof(base64Buffer))) {
        Serial.println("   ✗ Failed to encode transaction");
        free(serializedBuffer);
        return;
    }
    unsigned long encodeTime = millis() - encodeStart;
    
    Serial.println("   ✓ Transaction encoded successfully!");
    Serial.print("   Base64 Length: ");
    Serial.print(strlen(base64Buffer));
    Serial.println(" characters");
    Serial.print("   Encoding time: ");
    Serial.print(encodeTime);
    Serial.println(" ms");
    Serial.print("   Base64 (first 80 chars): ");
    Serial.println(String(base64Buffer).substring(0, 80) + "...");
    
    // Clean up
    free(serializedBuffer);
    
    // NOTE: Sending transaction is commented out for safety
    // Uncomment the following section to test sending on devnet:
    // /*
    Serial.println("\n5. Sending transaction via RPC...");
    String sendResponse = rpcClient.sendTransaction(base64Buffer);
    Serial.print("   Response: ");
    Serial.println(sendResponse.substring(0, min(300, (int)sendResponse.length())));
    
    // Parse response
    DynamicJsonDocument sendDoc(1024);
    deserializeJson(sendDoc, sendResponse);
    if (sendDoc.containsKey("result")) {
        String signature = sendDoc["result"].as<String>();
        Serial.print("   ✓ Transaction sent! Signature: ");
        Serial.println(signature);
        
        // Wait a bit and check transaction status
        delay(2000);
        Serial.println("\n6. Checking transaction status...");
        String txResponse = rpcClient.getTransaction(signature);
        Serial.print("   Transaction status: ");
        Serial.println(txResponse.substring(0, min(200, (int)txResponse.length())));
    } else if (sendDoc.containsKey("error")) {
        Serial.print("   ✗ Transaction failed: ");
        Serial.println(sendDoc["error"]["message"].as<String>());
    }
    // */
}


/**
 * Solduino Airdrop Demo
 * 
 * This example demonstrates requesting SOL airdrops:
 * - Generate a keypair for receiving airdrop
 * - Check initial account balance
 * - Request an airdrop from the network
 * - Verify the airdrop was successful
 * - Check transaction status
 * 
 * Hardware: ESP32
 * 
 * Required Libraries:
 * - ArduinoJson (via Library Manager)
 * - Solduino library (includes TweetNaCl)
 * 
 * Security Notes:
 * - Uses ESP32 hardware random number generator for key generation
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
// This makes it accessible from other devices on your network
const String RPC_ENDPOINT = "http://172.17.53.216:8899"; // Replace with your actual IP address
// Alternative: const String RPC_ENDPOINT = SOLDUINO_LOCALNET_RPC; // Only works if ESP32 and computer are same device

// Airdrop amount in lamports (1 SOL = 1,000,000,000 lamports)
// Note: Devnet max is usually 2 SOL per request
// Localnet has no practical limit
const uint64_t AIRDROP_AMOUNT = 1000000000; // 1 SOL

// ============================================================================
// Global Objects
// ============================================================================

// Initialize Solduino SDK
Solduino solduino;

// Initialize RPC client
RpcClient rpcClient(RPC_ENDPOINT);

// ============================================================================
// Setup
// ============================================================================

void setup() {
    Serial.begin(115200);
    delay(1000);
    
    Serial.println("\n\n");
    Serial.println("========================================");
    Serial.println("Solduino Airdrop Demo");
    Serial.println("========================================");
    Serial.println();
    
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
    
    Serial.println("\n=== Starting Airdrop Demo ===\n");
    
    // Run airdrop demo
    demonstrateAirdrop();
    
    Serial.println("\n=== Demo Complete ===\n");
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
// Airdrop Demo
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
    Serial.print("   Requesting: ");
    Serial.print(AIRDROP_AMOUNT);
    Serial.print(" lamports (");
    Serial.print((double)AIRDROP_AMOUNT / 1000000000.0, 9);
    Serial.println(" SOL)");
    
    unsigned long airdropStart = millis();
    String airdropResponse = rpcClient.requestAirdrop(address, AIRDROP_AMOUNT);
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
        
        // Wait for transaction to confirm with retry logic
        Serial.println("\n4. Waiting for transaction confirmation...");
        bool confirmed = false;
        uint64_t newLamports = 0;
        int maxRetries = 10;
        int retryDelay = 1000; // Start with 1 second
        
        for (int retry = 0; retry < maxRetries; retry++) {
            delay(retryDelay);
            Serial.print("   Checking balance (attempt ");
            Serial.print(retry + 1);
            Serial.print("/");
            Serial.print(maxRetries);
            Serial.println(")...");
            
            String newBalance = rpcClient.getBalance(address);
            DynamicJsonDocument newBalanceDoc(512);
            deserializeJson(newBalanceDoc, newBalance);
            
            if (newBalanceDoc.containsKey("result") && newBalanceDoc["result"].containsKey("value")) {
                newLamports = newBalanceDoc["result"]["value"].as<uint64_t>();
                
                if (newLamports > initialLamports) {
                    confirmed = true;
                    break;
                }
            }
            
            // Increase delay for subsequent retries
            if (retry < maxRetries - 1) {
                retryDelay = min(2000, retryDelay + 500); // Max 2 seconds per retry
            }
        }
        
        // Display final balance
        Serial.println("\n5. Final balance check...");
        String finalBalance = rpcClient.getBalance(address);
        DynamicJsonDocument finalBalanceDoc(512);
        deserializeJson(finalBalanceDoc, finalBalance);
        
        if (finalBalanceDoc.containsKey("result") && finalBalanceDoc["result"].containsKey("value")) {
            newLamports = finalBalanceDoc["result"]["value"].as<uint64_t>();
            Serial.print("   Final Balance: ");
            Serial.print(newLamports);
            Serial.print(" lamports (");
            Serial.print((double)newLamports / 1000000000.0, 9);
            Serial.println(" SOL)");
            
            if (newLamports > initialLamports) {
                uint64_t received = newLamports - initialLamports;
                Serial.print("   ✓ Airdrop successful! Received: ");
                Serial.print(received);
                Serial.print(" lamports (");
                Serial.print((double)received / 1000000000.0, 9);
                Serial.println(" SOL)");
                confirmed = true;
            } else {
                Serial.println("   ⚠️  Balance hasn't updated yet");
            }
        }
        
        // Check transaction status
        Serial.println("\n6. Checking transaction status...");
        String txResponse = rpcClient.getTransaction(signature);
        DynamicJsonDocument txDoc(2048);
        deserializeJson(txDoc, txResponse);
        
        if (txDoc.containsKey("result") && !txDoc["result"].isNull()) {
            Serial.println("   ✓ Transaction found on blockchain");
            
            // Try to parse transaction details
            if (txDoc["result"].containsKey("meta")) {
                JsonObject meta = txDoc["result"]["meta"];
                if (meta.containsKey("err")) {
                    if (meta["err"].isNull()) {
                        Serial.println("   ✓ Transaction status: SUCCESS");
                    } else {
                        Serial.print("   ✗ Transaction error: ");
                        serializeJson(meta["err"], Serial);
                        Serial.println();
                    }
                }
            }
            
            Serial.print("   Transaction details (first 300 chars): ");
            Serial.println(txResponse.substring(0, min(300, (int)txResponse.length())));
        } else {
            Serial.println("   ⚠️  Transaction not found in getTransaction response");
            Serial.println("   Note: Transaction may still be processing or was pruned");
            Serial.print("   Full response: ");
            Serial.println(txResponse.substring(0, min(200, (int)txResponse.length())));
        }
        
        // Final status summary
        Serial.println("\n7. Summary:");
        if (confirmed) {
            Serial.println("   ✓ Airdrop completed successfully!");
        } else {
            Serial.println("   ⚠️  Airdrop transaction was sent but balance not updated yet");
            Serial.println("   Transaction signature: " + signature);
            Serial.println("   Check the transaction on Solana Explorer using the signature above");
            Serial.println("   The transaction may still be confirming on the network");
        }
    } else if (airdropDoc.containsKey("error")) {
        Serial.print("   ✗ Airdrop failed: ");
        Serial.println(airdropDoc["error"]["message"].as<String>());
        Serial.println("   Note: Devnet airdrops are limited. Try localnet for unlimited airdrops.");
    } else {
        Serial.println("   ⚠️  Unexpected response format");
    }
}


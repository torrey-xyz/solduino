/**
 * Solduino Airdrop Demo
 * 
 * Demonstrates requesting SOL airdrops on Solana network.
 * 
 * Hardware: ESP32
 * Required Libraries: Solduino
 */

#include <WiFi.h>
#include <solduino.h>

// Configuration
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
RpcClient rpcClient(RPC_ENDPOINT);

void setup() {
    Serial.begin(115200);
    delay(1000);
    
    Serial.println("Solduino Airdrop Demo\n");
    
    if (!solduino.begin()) {
        Serial.println("ERROR: Failed to initialize SDK");
        return;
    }
    
    connectToWiFi();
    
    if (!rpcClient.begin()) {
        Serial.println("ERROR: Failed to initialize RPC client");
        return;
    }
    
    demonstrateAirdrop();
}

void loop() {
    delay(1000);
}

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
        Serial.println("\nWiFi connected!");
        Serial.print("IP: ");
        Serial.println(WiFi.localIP());
    } else {
        Serial.println("\nWiFi connection failed!");
    }
    Serial.println();
}

void demonstrateAirdrop() {
    // Generate keypair
    Serial.println("Generating keypair...");
    Keypair keypair;
    if (!keypair.generate()) {
        Serial.println("ERROR: Failed to generate keypair");
        return;
    }
    
    char address[64];
    if (!keypair.getPublicKeyAddress(address, sizeof(address))) {
        Serial.println("ERROR: Failed to get address");
        return;
    }
    Serial.print("Address: ");
    Serial.println(address);
    
    // Check initial balance
    uint64_t initialLamports = rpcClient.getBalanceLamports(address);
    Serial.print("Initial balance: ");
    Serial.print(initialLamports);
    Serial.println(" lamports\n");
    
    // Request airdrop
    Serial.print("Requesting airdrop of ");
    Serial.print((double)AIRDROP_AMOUNT / 1000000000.0, 9);
    Serial.println(" SOL...");
    
    String signature = rpcClient.requestAirdrop(address, AIRDROP_AMOUNT);
    
    if (signature.length() > 0) {
        Serial.print("Transaction signature: ");
        Serial.println(signature);
        
        // Wait and check balance
        Serial.println("Waiting for confirmation...");
        delay(15000); // Wait 15 seconds for transaction to confirm
        uint64_t newLamports = rpcClient.getBalanceLamports(address);
        
        Serial.print("\nNew balance: ");
        Serial.print(newLamports);
        Serial.println(" lamports");
        
        if (newLamports > initialLamports) {
            Serial.println("SUCCESS: Airdrop received!");
        } else {
            Serial.println("Pending: Balance not updated yet");
        }
    } else {
        Serial.println("ERROR: Airdrop request failed");
    }
}
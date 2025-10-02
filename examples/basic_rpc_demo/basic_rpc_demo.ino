/**
 * Solduino Basic RPC Demo
 * 
 * This example demonstrates the core functionality of the Solduino library:
 * - Core SDK initialization
 * - RPC Communication Module
 * - Account operations
 * - Network monitoring
 * 
 * Hardware: ESP32
 * 
 * Required Libraries:
 * - ArduinoJson (via Library Manager)
 * 
 * Setup:
 * 1. Update WiFi credentials below
 * 2. Select ESP32 board in Arduino IDE
 * 3. Upload to ESP32
 */

#include <WiFi.h>
#include <solduino.h>

// ============================================================================
// Configuration
// ============================================================================

// WiFi credentials
const char* ssid = "YOUR_WIFI_SSID";
const char* password = "YOUR_WIFI_PASSWORD";

// Solana RPC endpoint (using devnet for testing)
const String RPC_ENDPOINT = SOLDUINO_DEVNET_RPC;

// Test account (Solana System Program - always exists)
const String TEST_ACCOUNT = "11111111111111111111111111111112";

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
    
    // Print SDK information
    Serial.println("\n\n");
    solduino.hello();
    Serial.println();
    
    // Initialize SDK
    if (!solduino.begin()) {
        Serial.println("[ERROR] Failed to initialize Solduino SDK");
        return;
    }
    
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
    
    Serial.println("\n=== Starting Demo ===\n");
    
    // Run demo functions
    demonstrateBasicRpcCalls();
    demonstrateAccountOperations();
    demonstrateNetworkMonitoring();
    
    Serial.println("\n=== Demo Complete ===");
    Serial.println("Demo will repeat every 30 seconds...\n");
}

// ============================================================================
// Main Loop
// ============================================================================

void loop() {
    delay(30000); // Wait 30 seconds
    
    // Periodic network health check
    Serial.println("\n[Periodic Check] Network Health:");
    String health = rpcClient.getHealth();
    if (health.indexOf("ok") >= 0 || health.indexOf("\"ok\"") >= 0) {
        Serial.println("  ✓ Network is healthy");
    } else {
        Serial.println("  ✗ Network health check failed");
        Serial.println("  Response: " + health);
    }
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
        Serial.print("  Signal Strength (RSSI): ");
        Serial.print(WiFi.RSSI());
        Serial.println(" dBm");
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
 * Demonstrate basic RPC calls
 */
void demonstrateBasicRpcCalls() {
    Serial.println("--- Basic RPC Calls ---");
    
    // Get network version
    Serial.println("\n1. Getting Solana version...");
    String version = rpcClient.getVersion();
    Serial.print("   Response: ");
    Serial.println(version.substring(0, min(200, (int)version.length())));
    
    // Get current slot
    Serial.println("\n2. Getting current slot...");
    String slot = rpcClient.getSlot();
    Serial.print("   Response: ");
    Serial.println(slot);
    
    // Get block height
    Serial.println("\n3. Getting block height...");
    String blockHeight = rpcClient.getBlockHeight();
    Serial.print("   Response: ");
    Serial.println(blockHeight);
    
    // Get network health
    Serial.println("\n4. Checking network health...");
    String health = rpcClient.getHealth();
    Serial.print("   Response: ");
    Serial.println(health);
    
    // Get latest blockhash (using the new method name)
    Serial.println("\n5. Getting latest blockhash...");
    String blockhash = rpcClient.getLatestBlockhash();
    Serial.print("   Response: ");
    Serial.println(blockhash.substring(0, min(200, (int)blockhash.length())) + "...");
}

/**
 * Demonstrate account operations
 */
void demonstrateAccountOperations() {
    Serial.println("\n--- Account Operations ---");
    
    Serial.print("\nUsing test account: ");
    Serial.println(TEST_ACCOUNT);
    
    // Get account info
    Serial.println("\n1. Getting account info...");
    String accountInfo = rpcClient.getAccountInfo(TEST_ACCOUNT);
    Serial.print("   Response: ");
    Serial.println(accountInfo.substring(0, min(300, (int)accountInfo.length())) + "...");
    
    // Parse account info
    AccountInfo info;
    if (parseAccountInfo(accountInfo, info)) {
        Serial.println("\n   Parsed Account Info:");
        Serial.print("     Owner: ");
        Serial.println(info.owner);
        Serial.print("     Lamports: ");
        Serial.println(info.lamports);
        Serial.print("     Executable: ");
        Serial.println(info.executable ? "true" : "false");
        Serial.print("     Rent Epoch: ");
        Serial.println(info.rentEpoch);
    } else {
        Serial.println("   Failed to parse account info");
    }
    
    // Get balance
    Serial.println("\n2. Getting account balance...");
    String balance = rpcClient.getBalance(TEST_ACCOUNT);
    Serial.print("   Response: ");
    Serial.println(balance);
    
    // Parse balance
    Balance bal;
    if (parseBalance(balance, bal)) {
        Serial.println("\n   Parsed Balance:");
        Serial.print("     Value: ");
        Serial.print(bal.value);
        Serial.println(" lamports");
        Serial.print("     Context Slot: ");
        Serial.println(bal.context);
    } else {
        Serial.println("   Failed to parse balance");
    }
}

/**
 * Demonstrate network monitoring
 */
void demonstrateNetworkMonitoring() {
    Serial.println("\n--- Network Monitoring ---");
    
    // Get current slot
    Serial.println("\n1. Current network status:");
    String slotResponse = rpcClient.getSlot();
    DynamicJsonDocument slotDoc(512);
    deserializeJson(slotDoc, slotResponse);
    
    if (slotDoc.containsKey("result")) {
        uint64_t currentSlot = slotDoc["result"].as<uint64_t>();
        Serial.print("   Current Slot: ");
        Serial.println(currentSlot);
        
        // Try to get block info for a recent slot
        if (currentSlot > 10) {
            uint64_t recentSlot = currentSlot - 10;
            Serial.print("\n2. Getting block info for slot ");
            Serial.print(recentSlot);
            Serial.println("...");
            
            String blockInfo = rpcClient.getBlock(recentSlot);
            if (blockInfo.length() > 0) {
                Serial.print("   Block info received (first 200 chars): ");
                Serial.println(blockInfo.substring(0, min(200, (int)blockInfo.length())) + "...");
            } else {
                Serial.println("   Failed to get block info");
            }
        }
    }
    
    // Network version
    Serial.println("\n3. Network information:");
    String versionResponse = rpcClient.getVersion();
    DynamicJsonDocument versionDoc(512);
    deserializeJson(versionDoc, versionResponse);
    
    if (versionDoc.containsKey("result")) {
        if (versionDoc["result"].containsKey("solana-core")) {
            Serial.print("   Solana Core Version: ");
            Serial.println(versionDoc["result"]["solana-core"].as<String>());
        }
    }
}

// ============================================================================
// Utility Functions
// ============================================================================

/**
 * Helper function to print formatted JSON (for debugging)
 */
void printFormattedJson(const String& json) {
    DynamicJsonDocument doc(4096);
    DeserializationError error = deserializeJson(doc, json);
    
    if (error) {
        Serial.print("JSON Parse Error: ");
        Serial.println(error.c_str());
        return;
    }
    
    serializeJsonPretty(doc, Serial);
    Serial.println();
}


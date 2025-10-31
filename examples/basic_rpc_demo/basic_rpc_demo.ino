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
const char* ssid = "Parvat's Device";
const char* password = "Parvat@94255";

// Solana RPC endpoint (using devnet for testing)
const String RPC_ENDPOINT = SOLDUINO_MAINNET_RPC;

// Test account (Solana System Program - always exists)
const String TEST_ACCOUNT = "11111111111111111111111111111111";

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
    testNetworkInfo();
    testAccountInfo();
    showNetworkStatus();
    
    Serial.println("\n=== Demo Complete ===");
}

// ============================================================================
// Main Loop
// ============================================================================

void loop() {
    delay(30000); // Wait 30 seconds
    
    // Periodic network health check
    Serial.println("\n[Periodic Check] Network Health:");
    if (rpcClient.getHealth()) {
        Serial.println("  ✓ Network is healthy");
    } else {
        Serial.println("  ✗ Network health check failed");
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
 * Test network information methods
 */
void testNetworkInfo() {
    Serial.println("--- Basic RPC Calls ---");
    
    // Get network version
    Serial.println("\n1. Getting Solana version...");
    String version = rpcClient.getVersion();
    if (version.length() > 0) {
        Serial.print("   Solana Core Version: ");
        Serial.println(version);
    } else {
        Serial.println("   Failed to get version");
    }
    
    // Get current slot
    Serial.println("\n2. Getting current slot...");
    uint64_t slot = rpcClient.getSlot();
    if (slot > 0) {
        Serial.print("   Current Slot: ");
        Serial.println(slot);
    } else {
        Serial.println("   Failed to get slot");
    }
    
    // Get block height
    Serial.println("\n3. Getting block height...");
    uint64_t blockHeight = rpcClient.getBlockHeight();
    if (blockHeight > 0) {
        Serial.print("   Block Height: ");
        Serial.println(blockHeight);
    } else {
        Serial.println("   Failed to get block height");
    }
    
    // Get network health
    Serial.println("\n4. Checking network health...");
    bool health = rpcClient.getHealth();
    if (health) {
        Serial.println("   ✓ Network is healthy");
    } else {
        Serial.println("   ✗ Network health check failed");
    }
    
    // Get latest blockhash
    Serial.println("\n5. Getting latest blockhash...");
    String blockhash = rpcClient.getLatestBlockhash();
    if (blockhash.length() > 0) {
        Serial.print("   Blockhash: ");
        Serial.println(blockhash);
    } else {
        Serial.println("   Failed to get blockhash");
    }
}

/**
 * Test account information methods
 */
void testAccountInfo() {
    Serial.println("\n--- Account Operations ---");
    
    Serial.print("\nUsing test account: ");
    Serial.println(TEST_ACCOUNT);
    
    // Get account info
    Serial.println("\n1. Getting account info...");
    AccountInfo info;
    if (rpcClient.getAccountInfo(TEST_ACCOUNT, info)) {
        Serial.println("   Account Info:");
        Serial.print("     Owner: ");
        Serial.println(info.owner);
        Serial.print("     Lamports: ");
        Serial.println(info.lamports);
        Serial.print("     Executable: ");
        Serial.println(info.executable ? "true" : "false");
        Serial.print("     Rent Epoch: ");
        Serial.println(info.rentEpoch);
    } else {
        Serial.println("   Failed to get account info");
    }
    
    // Get balance
    Serial.println("\n2. Getting account balance...");
    uint64_t balanceLamports = rpcClient.getBalanceLamports(TEST_ACCOUNT);
    Serial.print("   Balance: ");
    Serial.print(balanceLamports);
    Serial.println(" lamports");
}

/**
 * Show network status
 */
void showNetworkStatus() {
    Serial.println("\n--- Network Monitoring ---");
    
    // Get current slot
    Serial.println("\n1. Current network status:");
    uint64_t currentSlot = rpcClient.getSlot();
    if (currentSlot > 0) {
        Serial.print("   Current Slot: ");
        Serial.println(currentSlot);
    } else {
        Serial.println("   Failed to get current slot");
    }
    
    // Network version
    Serial.println("\n2. Network information:");
    String version = rpcClient.getVersion();
    if (version.length() > 0) {
        Serial.print("   Solana Core Version: ");
        Serial.println(version);
    } else {
        Serial.println("   Failed to get version");
    }
}
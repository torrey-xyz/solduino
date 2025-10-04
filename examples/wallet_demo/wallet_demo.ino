/**
 * Solduino Wallet Demo
 * 
 * This example demonstrates wallet generation and import functionality:
 * - Generate new Solana keypairs
 * - Import wallets from private keys (Base58)
 * - Import wallets from seeds
 * - Display wallet addresses
 * - Sign and verify messages
 * - Use wallets with RPC client
 * 
 * Hardware: ESP32
 * 
 * Required Libraries:
 * - ArduinoJson (via Library Manager)
 * - Solduino library (includes TweetNaCl)
 * 
 * Security Notes:
 * - Uses ESP32 hardware random number generator for key generation
 * - Uses mbedTLS SHA-512 for cryptographic operations
 * - Private keys are stored securely in memory
 * - Never log or expose private keys in production code
 * 
 * Setup:
 * 1. Select ESP32 board in Arduino IDE
 * 2. Upload to ESP32
 * 3. Open Serial Monitor at 115200 baud
 */

#include <WiFi.h>
#include <solduino.h>

// ============================================================================
// Configuration
// ============================================================================

// WiFi credentials (optional - only needed for RPC operations)
const char* ssid = "YOUR_WIFI_SSID";
const char* password = "YOUR_WIFI_PASSWORD";

// Example private key for import demo (Base58 format)
// WARNING: This is a test key - never use real private keys in code!
const char* EXAMPLE_PRIVATE_KEY_BASE58 = "YourPrivateKeyBase58Here";

// Example seed for import demo (32 bytes as hex string)
const char* EXAMPLE_SEED_HEX = "0000000000000000000000000000000000000000000000000000000000000000";

// ============================================================================
// Global Objects
// ============================================================================

Solduino solduino;

// ============================================================================
// Setup
// ============================================================================

void setup() {
    Serial.begin(115200);
    delay(1000);
    
    Serial.println("\n\n");
    Serial.println("========================================");
    Serial.println("Solduino Wallet Generation Demo");
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
    
    Serial.println("=== Wallet Generation Demo ===\n");
    
    // Run demos
    demonstrateWalletGeneration();
    demonstrateWalletImport();
    demonstrateSeedImport();
    demonstrateMessageSigning();
    demonstrateRawBytesOperations();
    demonstrateMultipleKeypairs();
    
    Serial.println("\n=== Demo Complete ===");
}

// ============================================================================
// Main Loop
// ============================================================================

void loop() {
    // Optionally add periodic operations here
    // For example, you could:
    // - Check wallet balance via RPC
    // - Sign periodic messages
    // - Rotate keys
    
    delay(10000);
}

// ============================================================================
// Demo Functions
// ============================================================================

/**
 * Demonstrate generating a new wallet
 * Uses ESP32 hardware random number generator for cryptographically secure randomness
 */
void demonstrateWalletGeneration() {
    Serial.println("--- Generating New Wallet ---");
    Serial.println("Using ESP32 hardware RNG for secure randomness");
    
    Keypair keypair;
    
    Serial.println("\n1. Generating new random keypair...");
    unsigned long startTime = millis();
    if (keypair.generate()) {
        unsigned long generationTime = millis() - startTime;
        Serial.println("   ✓ Keypair generated successfully!");
        Serial.print("   Generation time: ");
        Serial.print(generationTime);
        Serial.println(" ms");
        
        // Display public address
        char address[64];
        if (keypair.getPublicKeyAddress(address, sizeof(address))) {
            Serial.print("   Public Address: ");
            Serial.println(address);
            Serial.print("   Address length: ");
            Serial.print(strlen(address));
            Serial.println(" characters");
        }
        
        // Display public key as raw bytes (hex)
        uint8_t pubKeyBytes[SOLDUINO_PUBKEY_SIZE];
        if (keypair.getPublicKey(pubKeyBytes)) {
            Serial.print("   Public Key (hex): ");
            for (size_t i = 0; i < SOLDUINO_PUBKEY_SIZE; i++) {
                if (pubKeyBytes[i] < 16) Serial.print("0");
                Serial.print(pubKeyBytes[i], HEX);
            }
            Serial.println();
        }
        
        // Display private key (Base58) - WARNING: Only for demo!
        char privateKeyBase58[128];
        if (keypair.getPrivateKeyBase58(privateKeyBase58, sizeof(privateKeyBase58))) {
            Serial.print("   Private Key (Base58): ");
            Serial.println(privateKeyBase58);
            Serial.println("   ⚠️  Keep this private key secure and never share it!");
        }
        
        // Verify keypair is initialized
        if (keypair.isInitialized()) {
            Serial.println("   ✓ Keypair is initialized and ready to use");
        }
    } else {
        Serial.println("   ✗ Failed to generate keypair");
        Serial.println("   Check ESP32 hardware RNG availability");
    }
    
    Serial.println();
}

/**
 * Demonstrate importing a wallet from private key
 */
void demonstrateWalletImport() {
    Serial.println("--- Importing Wallet from Private Key ---");
    
    // Generate a test keypair first
    Keypair testKeypair;
    if (!testKeypair.generate()) {
        Serial.println("   ✗ Failed to generate test keypair");
        return;
    }
    
    // Get the private key as Base58
    char privateKeyBase58[128];
    if (!testKeypair.getPrivateKeyBase58(privateKeyBase58, sizeof(privateKeyBase58))) {
        Serial.println("   ✗ Failed to get private key");
        return;
    }
    
    Serial.print("\n1. Test private key: ");
    Serial.println(privateKeyBase58);
    
    // Import the keypair
    Serial.println("\n2. Importing keypair from private key...");
    Keypair importedKeypair;
    if (importedKeypair.importFromPrivateKeyBase58(privateKeyBase58)) {
        Serial.println("   ✓ Keypair imported successfully!");
        
        // Verify the addresses match
        char originalAddress[64];
        char importedAddress[64];
        
        if (testKeypair.getPublicKeyAddress(originalAddress, sizeof(originalAddress)) &&
            importedKeypair.getPublicKeyAddress(importedAddress, sizeof(importedAddress))) {
            
            Serial.print("   Original Address: ");
            Serial.println(originalAddress);
            Serial.print("   Imported Address: ");
            Serial.println(importedAddress);
            
            if (strcmp(originalAddress, importedAddress) == 0) {
                Serial.println("   ✓ Addresses match! Import successful.");
            } else {
                Serial.println("   ✗ Addresses don't match!");
            }
        }
    } else {
        Serial.println("   ✗ Failed to import keypair");
    }
    
    Serial.println();
}

/**
 * Demonstrate importing a wallet from seed
 */
void demonstrateSeedImport() {
    Serial.println("--- Importing Wallet from Seed ---");
    
    // Generate a random seed
    uint8_t seed[SOLDUINO_SEED_SIZE];
    Serial.println("\n1. Generating random seed...");
    if (!generateRandomSeed(seed)) {
        Serial.println("   ✗ Failed to generate seed");
        return;
    }
    
    Serial.print("   Seed (hex): ");
    for (size_t i = 0; i < SOLDUINO_SEED_SIZE; i++) {
        if (seed[i] < 16) Serial.print("0");
        Serial.print(seed[i], HEX);
    }
    Serial.println();
    
    // Create keypair from seed
    Serial.println("\n2. Creating keypair from seed...");
    Keypair seedKeypair;
    if (seedKeypair.importFromSeed(seed)) {
        Serial.println("   ✓ Keypair created from seed!");
        
        char address[64];
        if (seedKeypair.getPublicKeyAddress(address, sizeof(address))) {
            Serial.print("   Public Address: ");
            Serial.println(address);
        }
        
        // Verify deterministic generation
        Serial.println("\n3. Verifying deterministic generation...");
        Keypair seedKeypair2;
        if (seedKeypair2.importFromSeed(seed)) {
            char address2[64];
            if (seedKeypair2.getPublicKeyAddress(address2, sizeof(address2))) {
                if (strcmp(address, address2) == 0) {
                    Serial.println("   ✓ Same seed produces same address (deterministic)");
                } else {
                    Serial.println("   ✗ Different addresses from same seed!");
                }
            }
        }
    } else {
        Serial.println("   ✗ Failed to create keypair from seed");
    }
    
    Serial.println();
}

/**
 * Demonstrate message signing
 */
void demonstrateMessageSigning() {
    Serial.println("--- Message Signing Demo ---");
    
    Keypair keypair;
    if (!keypair.generate()) {
        Serial.println("   ✗ Failed to generate keypair");
        return;
    }
    
    char address[64];
    keypair.getPublicKeyAddress(address, sizeof(address));
    Serial.print("\n1. Keypair Address: ");
    Serial.println(address);
    
    // Sign a message
    String message = "Hello, Solana!";
    Serial.print("\n2. Signing message: \"");
    Serial.print(message);
    Serial.println("\"");
    
    uint8_t signature[SOLDUINO_SIGNATURE_SIZE];
    unsigned long signStart = millis();
    if (keypair.signString(message, signature)) {
        unsigned long signTime = millis() - signStart;
        Serial.println("   ✓ Message signed successfully!");
        Serial.print("   Signing time: ");
        Serial.print(signTime);
        Serial.println(" ms");
        
        Serial.print("   Signature (hex): ");
        for (size_t i = 0; i < SOLDUINO_SIGNATURE_SIZE; i++) {
            if (signature[i] < 16) Serial.print("0");
            Serial.print(signature[i], HEX);
        }
        Serial.println();
        
        // Verify signature
        Serial.println("\n3. Verifying signature...");
        unsigned long verifyStart = millis();
        if (keypair.verify((const uint8_t*)message.c_str(), message.length(), signature)) {
            unsigned long verifyTime = millis() - verifyStart;
            Serial.println("   ✓ Signature verified successfully!");
            Serial.print("   Verification time: ");
            Serial.print(verifyTime);
            Serial.println(" ms");
        } else {
            Serial.println("   ✗ Signature verification failed");
            Serial.println("   Note: Full Ed25519 verification requires complete implementation");
        }
        
        // Try verifying with wrong message
        String wrongMessage = "Wrong message";
        Serial.println("\n4. Testing with wrong message...");
        if (!keypair.verify((const uint8_t*)wrongMessage.c_str(), wrongMessage.length(), signature)) {
            Serial.println("   ✓ Correctly rejected wrong message");
        } else {
            Serial.println("   ✗ Incorrectly accepted wrong message!");
        }
    } else {
        Serial.println("   ✗ Failed to sign message");
    }
    
    Serial.println();
}

/**
 * Demonstrate raw bytes operations
 */
void demonstrateRawBytesOperations() {
    Serial.println("--- Raw Bytes Operations ---");
    
    Keypair keypair;
    if (!keypair.generate()) {
        Serial.println("   ✗ Failed to generate keypair");
        return;
    }
    
    Serial.println("\n1. Getting raw key bytes...");
    
    // Get public key as bytes
    uint8_t pubKey[SOLDUINO_PUBKEY_SIZE];
    uint8_t privKey[SOLDUINO_SECRETKEY_SIZE];
    
    if (keypair.getPublicKey(pubKey) && keypair.getPrivateKey(privKey)) {
        Serial.println("   ✓ Retrieved raw key bytes");
        
        Serial.print("   Public Key (32 bytes): ");
        for (size_t i = 0; i < SOLDUINO_PUBKEY_SIZE; i++) {
            if (pubKey[i] < 16) Serial.print("0");
            Serial.print(pubKey[i], HEX);
            if (i < SOLDUINO_PUBKEY_SIZE - 1) Serial.print(":");
        }
        Serial.println();
        
        Serial.print("   Private Key (64 bytes): ");
        Serial.print("[SECRET - Not displayed]");
        Serial.println();
        Serial.println("   ✓ Private key bytes retrieved (not displayed for security)");
        
        // Create new keypair from raw bytes
        Serial.println("\n2. Importing keypair from raw bytes...");
        Keypair imported;
        if (imported.importFromPrivateKey(privKey)) {
            Serial.println("   ✓ Keypair imported from raw bytes");
            
            char originalAddr[64], importedAddr[64];
            if (keypair.getPublicKeyAddress(originalAddr, sizeof(originalAddr)) &&
                imported.getPublicKeyAddress(importedAddr, sizeof(importedAddr))) {
                
                if (strcmp(originalAddr, importedAddr) == 0) {
                    Serial.println("   ✓ Addresses match - import successful!");
                } else {
                    Serial.println("   ✗ Addresses don't match!");
                }
            }
        }
    }
    
    Serial.println();
}

/**
 * Demonstrate multiple keypairs
 */
void demonstrateMultipleKeypairs() {
    Serial.println("--- Multiple Keypairs Demo ---");
    
    Serial.println("\n1. Generating multiple keypairs...");
    
    const int numKeypairs = 3;
    Keypair keypairs[numKeypairs];
    char addresses[numKeypairs][64];
    
    for (int i = 0; i < numKeypairs; i++) {
        Serial.print("   Generating keypair ");
        Serial.print(i + 1);
        Serial.print("/");
        Serial.print(numKeypairs);
        Serial.print("...");
        
        if (keypairs[i].generate()) {
            if (keypairs[i].getPublicKeyAddress(addresses[i], sizeof(addresses[i]))) {
                Serial.println(" ✓");
                Serial.print("     Address: ");
                Serial.println(addresses[i]);
            } else {
                Serial.println(" ✗ (address retrieval failed)");
            }
        } else {
            Serial.println(" ✗ (generation failed)");
        }
    }
    
    // Verify all addresses are unique
    Serial.println("\n2. Verifying uniqueness...");
    bool allUnique = true;
    for (int i = 0; i < numKeypairs; i++) {
        for (int j = i + 1; j < numKeypairs; j++) {
            if (strcmp(addresses[i], addresses[j]) == 0) {
                Serial.print("   ✗ Duplicate address found: ");
                Serial.println(addresses[i]);
                allUnique = false;
            }
        }
    }
    
    if (allUnique) {
        Serial.println("   ✓ All addresses are unique");
    }
    
    Serial.println();
}

// ============================================================================
// Utility Functions
// ============================================================================

/**
 * Convert hex string to bytes
 */
void hexStringToBytes(const char* hex, uint8_t* bytes, size_t len) {
    for (size_t i = 0; i < len; i++) {
        char hexByte[3] = {hex[i * 2], hex[i * 2 + 1], '\0'};
        bytes[i] = strtoul(hexByte, NULL, 16);
    }
}


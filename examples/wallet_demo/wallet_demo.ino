/**
 * Solduino Wallet Demo
 * 
 * Minimal demo for wallet generation and keypair operations
 * 
 * Hardware: ESP32
 * 
 * Required Libraries:
 * - Solduino library (includes TweetNaCl)
 */

#include <WiFi.h>
#include <solduino.h>

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
    
    Serial.println("\n=== Solduino Wallet Demo ===\n");
    
    // Initialize SDK
    if (!solduino.begin()) {
        Serial.println("[ERROR] Failed to initialize SDK");
        return;
    }
    
    // Run demo
    testWalletGeneration();
    testWalletImport();
    
    Serial.println("\n=== Demo Complete ===\n");
}

void loop() {
    delay(10000);
}

// ============================================================================
// Demo Functions
// ============================================================================

/**
 * Test wallet generation
 */
void testWalletGeneration() {
    Serial.println("--- Generating Wallet ---");
    
    Keypair keypair;
    if (keypair.generate()) {
        Serial.println("✓ Keypair generated");
        
        char address[64];
        if (keypair.getPublicKeyAddress(address, sizeof(address))) {
            Serial.print("Address: ");
            Serial.println(address);
        }
    } else {
        Serial.println("✗ Failed to generate keypair");
    }
    
    Serial.println();
}

/**
 * Test wallet import
 */
void testWalletImport() {
    Serial.println("--- Importing Wallet ---");
    
    // Hardcoded private key for demo
    const char* TEST_PRIVATE_KEY = "3Dn6BSEa11MgKqv5nj2ZAdykmACQep9HwAQ51ghRYHJx";
    
    // Import keypair from hardcoded private key
    Keypair imported;
    if (imported.importFromPrivateKeyBase58(TEST_PRIVATE_KEY)) {
        Serial.println("✓ Keypair imported");
        
        char importedAddr[64];
        if (imported.getPublicKeyAddress(importedAddr, sizeof(importedAddr))) {
            Serial.print("Address: ");
            Serial.println(importedAddr);
        }
    } else {
        Serial.println("✗ Failed to import keypair");
    }
    
    Serial.println();
}

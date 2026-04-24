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
    testWalletImportFromJSON();
    
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

/**
 * Test importing a wallet from solana-keygen JSON byte array
 */
void testWalletImportFromJSON() {
    Serial.println("--- Importing Wallet from solana-keygen JSON ---");

    // Byte array from: solana-keygen new -o dev-solduino.json
    // Full 64-byte secret key (first 32 = seed, last 32 = pubkey)
    const uint8_t keypairBytes[64] = {
        62,90,61,63,130,195,30,214,206,239,94,189,187,215,28,185,
        231,51,243,89,138,191,56,253,31,157,192,123,246,47,86,228,
        207,24,159,177,20,217,203,121,160,29,141,184,43,142,214,197,
        46,27,220,209,107,51,155,196,240,29,192,215,62,248,80,152
    };

    // Option A: import using all 64 bytes
    Keypair walletA;
    if (walletA.importFromPrivateKey(keypairBytes)) {
        Serial.println("✓ Imported via importFromPrivateKey (64 bytes)");
        char addr[64];
        if (walletA.getPublicKeyAddress(addr, sizeof(addr))) {
            Serial.print("  Address: ");
            Serial.println(addr);
        }
    } else {
        Serial.println("✗ Failed to import via importFromPrivateKey");
    }

    // Option B: import using only the first 32 bytes (seed)
    Keypair walletB;
    if (walletB.importFromSeed(keypairBytes)) {
        Serial.println("✓ Imported via importFromSeed (first 32 bytes)");
        char addr[64];
        if (walletB.getPublicKeyAddress(addr, sizeof(addr))) {
            Serial.print("  Address: ");
            Serial.println(addr);
        }
    } else {
        Serial.println("✗ Failed to import via importFromSeed");
    }

    Serial.println();
}

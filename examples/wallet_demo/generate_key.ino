/**
 * Temporary script to generate a private key for wallet_demo.ino
 * Run this once, copy the private key, then use it in wallet_demo.ino
 */

#include <WiFi.h>
#include <solduino.h>

Solduino solduino;

void setup() {
    Serial.begin(115200);
    delay(1000);
    
    Serial.println("\n=== Generate Private Key ===\n");
    
    if (!solduino.begin()) {
        Serial.println("[ERROR] Failed to initialize SDK");
        return;
    }
    
    // Generate a keypair
    Keypair keypair;
    if (keypair.generate()) {
        Serial.println("✓ Keypair generated");
        
        // Get and print the address
        char address[64];
        if (keypair.getPublicKeyAddress(address, sizeof(address))) {
            Serial.print("Address: ");
            Serial.println(address);
        }
        
        // Get and print the private key (Base58)
        char privateKey[128];
        if (keypair.getPrivateKeyBase58(privateKey, sizeof(privateKey))) {
            Serial.println("\n--- Copy this private key ---");
            Serial.println("const char* TEST_PRIVATE_KEY = \"");
            Serial.print(privateKey);
            Serial.println("\";");
            Serial.println("--- End of private key ---\n");
        }
    } else {
        Serial.println("✗ Failed to generate keypair");
    }
}

void loop() {
    delay(10000);
}


#include <solduino.h>

// Example message to sign
const char* message = "Hello Solana from Arduino!";

// Buffer for keys and signatures
unsigned char public_key[32];
unsigned char private_key[64];
unsigned char signature[64];

void setup() {
    Serial.begin(115200);
    while (!Serial) {
        ; // Wait for serial port to connect
    }
    Serial.println("Solduino Ed25519 Signing Example");

    // Generate a new keypair
    if (crypto_sign_keypair(public_key, private_key) != 0) {
        Serial.println("Failed to generate keypair!");
        return;
    }

    // Print the public key
    Serial.println("\nPublic Key:");
    for (int i = 0; i < 32; i++) {
        if (public_key[i] < 16) Serial.print("0");
        Serial.print(public_key[i], HEX);
    }

    // Sign the message
    unsigned long long siglen;
    if (crypto_sign_detached(
        signature, 
        &siglen, 
        (unsigned char*)message, 
        strlen(message), 
        private_key
    ) != 0) {
        Serial.println("\nFailed to sign message!");
        return;
    }

    // Print the signature
    Serial.println("\n\nSignature:");
    for (int i = 0; i < 64; i++) {
        if (signature[i] < 16) Serial.print("0");
        Serial.print(signature[i], HEX);
    }

    // Verify the signature
    if (crypto_sign_verify_detached(
        signature,
        (unsigned char*)message,
        strlen(message),
        public_key
    ) != 0) {
        Serial.println("\n\nSignature verification failed!");
    } else {
        Serial.println("\n\nSignature verified successfully!");
    }
}

void loop() {
    // Nothing to do here
    delay(1000);
} 
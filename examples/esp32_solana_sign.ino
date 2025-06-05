#include <solduino.h>
#include <WiFi.h>
#include <SPIFFS.h>

// WiFi credentials
const char* ssid = "YOUR_WIFI_SSID";
const char* password = "YOUR_WIFI_PASSWORD";

// Keypair storage file
const char* KEYPAIR_FILE = "/solana_keypair.bin";

// Buffers for keys and signatures
unsigned char public_key[32];
unsigned char private_key[64];
unsigned char signature[64];

// Function to save keypair to SPIFFS
void saveKeypair() {
    File file = SPIFFS.open(KEYPAIR_FILE, FILE_WRITE);
    if (!file) {
        Serial.println("Failed to open file for writing");
        return;
    }
    file.write(private_key, 64);
    file.close();
}

// Function to load keypair from SPIFFS
bool loadKeypair() {
    if (!SPIFFS.exists(KEYPAIR_FILE)) {
        Serial.println("No existing keypair found");
        return false;
    }
    
    File file = SPIFFS.open(KEYPAIR_FILE, FILE_READ);
    if (!file) {
        Serial.println("Failed to open file for reading");
        return false;
    }
    
    file.read(private_key, 64);
    file.close();
    
    // Extract public key from private key
    memcpy(public_key, private_key + 32, 32);
    return true;
}

void setup() {
    Serial.begin(115200);
    
    // Initialize SPIFFS
    if (!SPIFFS.begin(true)) {
        Serial.println("SPIFFS initialization failed!");
        return;
    }

    // Connect to WiFi
    WiFi.begin(ssid, password);
    while (WiFi.status() != WL_CONNECTED) {
        delay(500);
        Serial.print(".");
    }
    Serial.println("\nWiFi connected");

    // Load or generate keypair
    if (!loadKeypair()) {
        Serial.println("Generating new keypair...");
        if (crypto_sign_keypair(public_key, private_key) != 0) {
            Serial.println("Failed to generate keypair!");
            return;
        }
        saveKeypair();
    }

    // Print the public key
    Serial.println("\nSolana Public Key:");
    for (int i = 0; i < 32; i++) {
        if (public_key[i] < 16) Serial.print("0");
        Serial.print(public_key[i], HEX);
    }

    // Example: Sign a message
    const char* message = "Hello Solana from ESP32!";
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
    // Add your Solana transaction logic here
    delay(1000);
} 
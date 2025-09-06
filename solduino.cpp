#include "solduino.h"

// Constructor
Solduino::Solduino() {
    // Initialize if needed
}

// Get library version string
const char* Solduino::getVersion() {
    return SOLDUINO_VERSION_STRING;
}

// Get major version number
int Solduino::getVersionMajor() {
    return SOLDUINO_VERSION_MAJOR;
}

// Get minor version number
int Solduino::getVersionMinor() {
    return SOLDUINO_VERSION_MINOR;
}

// Get patch version number
int Solduino::getVersionPatch() {
    return SOLDUINO_VERSION_PATCH;
}

// Initialize library
bool Solduino::begin() {
    // Placeholder for future initialization logic
    // e.g., setting up default configurations, checking prerequisites
    return true;
}

// Cleanup library resources
void Solduino::end() {
    // Placeholder for cleanup logic
}

// Basic hello function implementation
void Solduino::hello(void) {
    Serial.println("========================================");
    Serial.println("Solduino - Solana Library for Arduino");
    Serial.print("Version: ");
    Serial.println(getVersion());
    Serial.println("========================================");
    Serial.println("Modules:");
    Serial.println("  ✓ RPC Communication");
    Serial.println("  ✓ Connection Management");
    Serial.println("  ⚠ Wallet Generation (pending)");
    Serial.println("  ⚠ Transaction Signing (pending)");
    Serial.println("========================================");
}

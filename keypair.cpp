#include "keypair.h"
#include <string.h>

Keypair::Keypair() : initialized(false) {
    clearKeys();
}

Keypair::~Keypair() {
    clearKeys();
}

void Keypair::clearKeys() {
    memset(publicKey, 0, sizeof(publicKey));
    memset(privateKey, 0, sizeof(privateKey));
    initialized = false;
}

bool Keypair::generate() {
    clearKeys();
    
    uint8_t seed[SOLDUINO_SEED_SIZE];
    if (!generateRandomSeed(seed)) {
        return false;
    }
    
    if (!generateKeypairFromSeed(seed, publicKey, privateKey)) {
        return false;
    }
    
    initialized = true;
    return true;
}

bool Keypair::importFromPrivateKey(const uint8_t* privateKeyBytes) {
    if (!privateKeyBytes) {
        return false;
    }
    
    clearKeys();
    memcpy(privateKey, privateKeyBytes, SOLDUINO_SECRETKEY_SIZE);
    
    // Extract public key from private key
    if (!getPublicKeyFromPrivate(privateKey, publicKey)) {
        clearKeys();
        return false;
    }
    
    initialized = true;
    return true;
}

bool Keypair::importFromPrivateKeyBase58(const char* privateKeyBase58) {
    if (!privateKeyBase58) {
        return false;
    }
    
    clearKeys();
    
    uint8_t importedPrivateKey[SOLDUINO_SECRETKEY_SIZE];
    if (!base58ToPrivateKey(privateKeyBase58, importedPrivateKey)) {
        return false;
    }
    
    return importFromPrivateKey(importedPrivateKey);
}

bool Keypair::importFromSeed(const uint8_t* seed) {
    if (!seed) {
        return false;
    }
    
    clearKeys();
    
    if (!generateKeypairFromSeed(seed, publicKey, privateKey)) {
        return false;
    }
    
    initialized = true;
    return true;
}

bool Keypair::getPublicKey(uint8_t* output) const {
    if (!output || !initialized) {
        return false;
    }
    
    memcpy(output, publicKey, SOLDUINO_PUBKEY_SIZE);
    return true;
}

bool Keypair::getPrivateKey(uint8_t* output) const {
    if (!output || !initialized) {
        return false;
    }
    
    memcpy(output, privateKey, SOLDUINO_SECRETKEY_SIZE);
    return true;
}

bool Keypair::getPublicKeyAddress(char* address, size_t addressLen) const {
    if (!address || addressLen == 0 || !initialized) {
        return false;
    }
    
    return publicKeyToAddress(publicKey, address, addressLen);
}

bool Keypair::getPrivateKeyBase58(char* output, size_t outputLen) const {
    if (!output || outputLen == 0 || !initialized) {
        return false;
    }
    
    return privateKeyToBase58(privateKey, output, outputLen);
}

bool Keypair::sign(const uint8_t* message, size_t messageLen, uint8_t* signature) const {
    if (!message || !signature || messageLen == 0 || !initialized) {
        return false;
    }
    
    return signMessage(message, messageLen, privateKey, signature);
}

bool Keypair::signString(const String& message, uint8_t* signature) const {
    if (!initialized) {
        return false;
    }
    
    return sign((const uint8_t*)message.c_str(), message.length(), signature);
}

bool Keypair::verify(const uint8_t* message, size_t messageLen, const uint8_t* signature) const {
    if (!message || !signature || messageLen == 0 || !initialized) {
        return false;
    }
    
    return verifySignature(message, messageLen, signature, publicKey);
}

void Keypair::clear() {
    clearKeys();
}

void Keypair::printAddress() const {
    if (!initialized) {
        Serial.println("Keypair not initialized");
        return;
    }
    
    char address[64];
    if (getPublicKeyAddress(address, sizeof(address))) {
        Serial.print("Public Address: ");
        Serial.println(address);
    } else {
        Serial.println("Failed to get address");
    }
}


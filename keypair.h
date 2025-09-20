#ifndef SOLDUINO_KEYPAIR_H
#define SOLDUINO_KEYPAIR_H

#include <Arduino.h>
#include <stdint.h>
#include "crypto.h"

// ============================================================================
// Solduino Keypair Module
// ============================================================================
// Provides wallet generation and management functionality:
// - Generate new Solana keypairs
// - Import existing wallets from private keys
// - Sign messages and transactions
// - Manage public/private key pairs
// ============================================================================

/**
 * Solana Keypair Class
 * 
 * Represents a Solana wallet with Ed25519 keypair.
 * Provides functionality to generate, import, and use wallets.
 */
class Keypair {
private:
    uint8_t publicKey[SOLDUINO_PUBKEY_SIZE];
    uint8_t privateKey[SOLDUINO_SECRETKEY_SIZE];
    bool initialized;
    
    void clearKeys();

public:
    /**
     * Constructor - creates uninitialized keypair
     */
    Keypair();
    
    /**
     * Destructor - clears sensitive data
     */
    ~Keypair();
    
    /**
     * Generate a new random keypair
     * @return true if successful, false otherwise
     */
    bool generate();
    
    /**
     * Import keypair from private key bytes
     * @param privateKeyBytes Private key (64 bytes)
     * @return true if successful, false otherwise
     */
    bool importFromPrivateKey(const uint8_t* privateKeyBytes);
    
    /**
     * Import keypair from Base58 encoded private key string
     * @param privateKeyBase58 Base58 encoded private key
     * @return true if successful, false otherwise
     */
    bool importFromPrivateKeyBase58(const char* privateKeyBase58);
    
    /**
     * Import keypair from seed (32 bytes)
     * @param seed Seed bytes (32 bytes)
     * @return true if successful, false otherwise
     */
    bool importFromSeed(const uint8_t* seed);
    
    /**
     * Get public key as bytes
     * @param output Output buffer (32 bytes)
     * @return true if keypair is initialized, false otherwise
     */
    bool getPublicKey(uint8_t* output) const;
    
    /**
     * Get private key as bytes
     * @param output Output buffer (64 bytes)
     * @return true if keypair is initialized, false otherwise
     */
    bool getPrivateKey(uint8_t* output) const;
    
    /**
     * Get public key as Solana address (Base58)
     * @param address Output string buffer
     * @param addressLen Maximum address length
     * @return true if successful, false otherwise
     */
    bool getPublicKeyAddress(char* address, size_t addressLen) const;
    
    /**
     * Get private key as Base58 string
     * @param output Output string buffer
     * @param outputLen Maximum output length
     * @return true if successful, false otherwise
     */
    bool getPrivateKeyBase58(char* output, size_t outputLen) const;
    
    /**
     * Sign a message with this keypair
     * @param message Message to sign
     * @param messageLen Length of message
     * @param signature Output signature (64 bytes)
     * @return true if successful, false otherwise
     */
    bool sign(const uint8_t* message, size_t messageLen, uint8_t* signature) const;
    
    /**
     * Sign a string message
     * @param message String message to sign
     * @param signature Output signature (64 bytes)
     * @return true if successful, false otherwise
     */
    bool signString(const String& message, uint8_t* signature) const;
    
    /**
     * Verify a signature with this keypair's public key
     * @param message Original message
     * @param messageLen Length of message
     * @param signature Signature to verify (64 bytes)
     * @return true if signature is valid, false otherwise
     */
    bool verify(const uint8_t* message, size_t messageLen, const uint8_t* signature) const;
    
    /**
     * Check if keypair is initialized
     * @return true if initialized, false otherwise
     */
    bool isInitialized() const { return initialized; }
    
    /**
     * Clear the keypair (zero out keys)
     */
    void clear();
    
    /**
     * Print public key address to Serial (for debugging)
     */
    void printAddress() const;
};

#endif // SOLDUINO_KEYPAIR_H


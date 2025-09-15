#ifndef SOLDUINO_CRYPTO_H
#define SOLDUINO_CRYPTO_H

#include <Arduino.h>
#include <stdint.h>

// ============================================================================
// Solduino Crypto Module
// ============================================================================
// Provides cryptographic functions for Solana wallet operations:
// - Base58 encoding/decoding (for Solana addresses)
// - Ed25519 keypair operations
// - Message signing and verification
// ============================================================================

// Key sizes
#define SOLDUINO_PUBKEY_SIZE 32
#define SOLDUINO_SECRETKEY_SIZE 64
#define SOLDUINO_SIGNATURE_SIZE 64
#define SOLDUINO_SEED_SIZE 32

// Base58 alphabet for Solana
#define BASE58_ALPHABET "123456789ABCDEFGHJKLMNPQRSTUVWXYZabcdefghijkmnopqrstuvwxyz"

/**
 * Generate a random seed for keypair generation
 * @param seed Output buffer (32 bytes)
 * @return true if successful, false otherwise
 */
bool generateRandomSeed(uint8_t* seed);

/**
 * Generate Ed25519 keypair from seed
 * @param seed Input seed (32 bytes)
 * @param publicKey Output public key (32 bytes)
 * @param privateKey Output private key (64 bytes)
 * @return true if successful, false otherwise
 */
bool generateKeypairFromSeed(const uint8_t* seed, uint8_t* publicKey, uint8_t* privateKey);

/**
 * Generate Ed25519 keypair with random seed
 * @param publicKey Output public key (32 bytes)
 * @param privateKey Output private key (64 bytes)
 * @return true if successful, false otherwise
 */
bool generateKeypair(uint8_t* publicKey, uint8_t* privateKey);

/**
 * Sign a message with Ed25519 private key
 * @param message Message to sign
 * @param messageLen Length of message
 * @param privateKey Private key (64 bytes)
 * @param signature Output signature (64 bytes)
 * @return true if successful, false otherwise
 */
bool signMessage(const uint8_t* message, size_t messageLen, const uint8_t* privateKey, uint8_t* signature);

/**
 * Verify Ed25519 signature
 * @param message Original message
 * @param messageLen Length of message
 * @param signature Signature to verify (64 bytes)
 * @param publicKey Public key (32 bytes)
 * @return true if signature is valid, false otherwise
 */
bool verifySignature(const uint8_t* message, size_t messageLen, const uint8_t* signature, const uint8_t* publicKey);

/**
 * Encode bytes to Base58 string (for Solana addresses)
 * @param data Input data
 * @param len Length of data
 * @param output Output string buffer
 * @param outputLen Maximum output length
 * @return Length of encoded string, or 0 on error
 */
size_t base58Encode(const uint8_t* data, size_t len, char* output, size_t outputLen);

/**
 * Decode Base58 string to bytes
 * @param input Base58 encoded string
 * @param output Output buffer
 * @param outputLen Maximum output length
 * @return Length of decoded data, or 0 on error
 */
size_t base58Decode(const char* input, uint8_t* output, size_t outputLen);

/**
 * Convert public key bytes to Solana address (Base58)
 * @param publicKey Public key bytes (32 bytes)
 * @param address Output string buffer
 * @param addressLen Maximum address length
 * @return true if successful, false otherwise
 */
bool publicKeyToAddress(const uint8_t* publicKey, char* address, size_t addressLen);

/**
 * Convert Solana address (Base58) to public key bytes
 * @param address Base58 encoded address
 * @param publicKey Output buffer (32 bytes)
 * @return true if successful, false otherwise
 */
bool addressToPublicKey(const char* address, uint8_t* publicKey);

/**
 * Convert private key bytes to Base58 string
 * @param privateKey Private key bytes (64 bytes)
 * @param output Output string buffer
 * @param outputLen Maximum output length
 * @return true if successful, false otherwise
 */
bool privateKeyToBase58(const uint8_t* privateKey, char* output, size_t outputLen);

/**
 * Convert Base58 string to private key bytes
 * @param input Base58 encoded private key
 * @param privateKey Output buffer (64 bytes)
 * @return true if successful, false otherwise
 */
bool base58ToPrivateKey(const char* input, uint8_t* privateKey);

/**
 * Extract public key from private key (Ed25519)
 * @param privateKey Private key (64 bytes)
 * @param publicKey Output public key (32 bytes)
 * @return true if successful, false otherwise
 */
bool getPublicKeyFromPrivate(const uint8_t* privateKey, uint8_t* publicKey);

// Test function for Ed25519
bool testEd25519();

#endif // SOLDUINO_CRYPTO_H


#include "crypto.h"
#include <sodium.h>
#include <string.h>
#include <stdlib.h>

#ifdef ESP32
#include "esp_random.h"
#else
#include <time.h>
#endif

// Helper macro for min
#ifndef min
#define min(a, b) ((a) < (b) ? (a) : (b))
#endif

// Base58 implementation
static const char base58_chars[] = BASE58_ALPHABET;

size_t base58Encode(const uint8_t* data, size_t len, char* output, size_t outputLen) {
    if (!data || !output || len == 0 || outputLen == 0) {
        return 0;
    }
    
    // Count leading zeros
    size_t zeros = 0;
    while (zeros < len && data[zeros] == 0) {
        zeros++;
    }
    
    // Convert to base58
    uint8_t* buffer = (uint8_t*)malloc(len * 2);
    if (!buffer) return 0;
    
    memcpy(buffer, data, len);
    
    size_t output_idx = 0;
    for (size_t i = 0; i < zeros && output_idx < outputLen - 1; i++) {
        output[output_idx++] = '1';
    }
    
    size_t start = zeros;
    while (start < len) {
        uint32_t remainder = 0;
        bool leading_zero = true;
        
        for (size_t i = start; i < len; i++) {
            remainder = remainder * 256 + buffer[i];
            buffer[i] = remainder / 58;
            remainder %= 58;
            if (buffer[i] != 0) leading_zero = false;
        }
        
        if (output_idx >= outputLen - 1) break;
        output[output_idx++] = base58_chars[remainder];
        
        while (start < len && buffer[start] == 0) {
            start++;
        }
    }
    
    // Reverse the string
    for (size_t i = 0; i < output_idx / 2; i++) {
        char temp = output[i];
        output[i] = output[output_idx - 1 - i];
        output[output_idx - 1 - i] = temp;
    }
    
    output[output_idx] = '\0';
    free(buffer);
    return output_idx;
}

size_t base58Decode(const char* input, uint8_t* output, size_t outputLen) {
    if (!input || !output || outputLen == 0) {
        return 0;
    }
    
    size_t inputLen = strlen(input);
    if (inputLen == 0) return 0;
    
    // Count leading ones
    size_t zeros = 0;
    while (zeros < inputLen && input[zeros] == '1') {
        zeros++;
    }
    
    uint8_t* buffer = (uint8_t*)malloc(inputLen * 2);
    if (!buffer) return 0;
    
    memset(buffer, 0, inputLen * 2);
    
    // Convert from base58
    for (size_t i = zeros; i < inputLen; i++) {
        const char* pos = strchr(base58_chars, input[i]);
        if (!pos) {
            free(buffer);
            return 0;
        }
        
        uint8_t value = pos - base58_chars;
        uint32_t carry = value;
        
        for (size_t j = 0; j < inputLen * 2; j++) {
            carry += buffer[j] * 58;
            buffer[j] = carry & 0xFF;
            carry >>= 8;
        }
    }
    
    // Find the actual length
    size_t resultLen = 0;
    for (size_t i = inputLen * 2; i > 0; i--) {
        if (buffer[i - 1] != 0) {
            resultLen = i;
            break;
        }
    }
    
    resultLen += zeros;
    if (resultLen > outputLen) {
        free(buffer);
        return 0;
    }
    
    // Copy result
    memset(output, 0, outputLen);
    for (size_t i = 0; i < zeros; i++) {
        output[i] = 0;
    }
    
    for (size_t i = 0; i < resultLen - zeros; i++) {
        output[zeros + i] = buffer[resultLen - zeros - 1 - i];
    }
    
    free(buffer);
    return resultLen;
}

bool generateRandomSeed(uint8_t* seed) {
    if (!seed) return false;
    
#ifdef ESP32
    // Use ESP32 hardware random number generator
    for (size_t i = 0; i < SOLDUINO_SEED_SIZE; i += 4) {
        uint32_t rand = esp_random();
        memcpy(seed + i, &rand, min(4, (int)(SOLDUINO_SEED_SIZE - i)));
    }
    return true;
#else
    // Fallback for other platforms
    srand(time(NULL));
    for (size_t i = 0; i < SOLDUINO_SEED_SIZE; i++) {
        seed[i] = rand() % 256;
    }
    return true;
#endif
}

bool generateKeypairFromSeed(const uint8_t* seed, uint8_t* publicKey, uint8_t* privateKey) {
    if (!seed || !publicKey || !privateKey) return false;
    if (sodium_init() < 0) return false;
    // libsodium: generate Ed25519 keypair from 32-byte seed
    int result = crypto_sign_ed25519_seed_keypair(publicKey, privateKey, seed);
    return result == 0;
}

bool generateKeypair(uint8_t* publicKey, uint8_t* privateKey) {
    if (!publicKey || !privateKey) return false;
    if (sodium_init() < 0) return false;
    int result = crypto_sign_ed25519_keypair(publicKey, privateKey);
    return result == 0;
}

bool signMessage(const uint8_t* message, size_t messageLen, const uint8_t* privateKey, uint8_t* signature) {
    if (!message || !privateKey || !signature) {
        return false;
    }
    if (sodium_init() < 0) return false;
    // Detached Ed25519 signature
    unsigned long long siglen = 0;
    int result = crypto_sign_ed25519_detached(signature, &siglen, message, (unsigned long long)messageLen, privateKey);
    return result == 0 && siglen == SOLDUINO_SIGNATURE_SIZE;
}

bool verifySignature(const uint8_t* message, size_t messageLen, const uint8_t* signature, const uint8_t* publicKey) {
    if (!message || !signature || !publicKey) {
        return false;
    }
    if (sodium_init() < 0) return false;
    int result = crypto_sign_ed25519_verify_detached(signature, message, (unsigned long long)messageLen, publicKey);
    return result == 0;
}

bool publicKeyToAddress(const uint8_t* publicKey, char* address, size_t addressLen) {
    if (!publicKey || !address || addressLen == 0) {
        return false;
    }
    
    return base58Encode(publicKey, SOLDUINO_PUBKEY_SIZE, address, addressLen) > 0;
}

bool addressToPublicKey(const char* address, uint8_t* publicKey) {
    if (!address || !publicKey) {
        return false;
    }
    
    size_t decodedLen = base58Decode(address, publicKey, SOLDUINO_PUBKEY_SIZE);
    return decodedLen == SOLDUINO_PUBKEY_SIZE;
}

bool privateKeyToBase58(const uint8_t* privateKey, char* output, size_t outputLen) {
    if (!privateKey || !output || outputLen == 0) {
        return false;
    }
    
    // Encode only the first 32 bytes (the seed) for Solana private keys
    return base58Encode(privateKey, 32, output, outputLen) > 0;
}

bool base58ToPrivateKey(const char* input, uint8_t* privateKey) {
    if (!input || !privateKey) {
        return false;
    }
    
    uint8_t decoded[64];
    size_t decodedLen = base58Decode(input, decoded, 64);
    
    if (decodedLen != 32 && decodedLen != 64) {
        return false;
    }
    
    // If decoded is 32 bytes, we need to derive the public key
    if (decodedLen == 32) {
        uint8_t publicKey[32];
        if (!generateKeypairFromSeed(decoded, publicKey, privateKey)) {
            return false;
        }
    } else {
        memcpy(privateKey, decoded, 64);
    }
    
    return true;
}

bool getPublicKeyFromPrivate(const uint8_t* privateKey, uint8_t* publicKey) {
    if (!privateKey || !publicKey) {
        return false;
    }
    if (sodium_init() < 0) return false;
    int result = crypto_sign_ed25519_sk_to_pk(publicKey, privateKey);
    return result == 0;
}

// Test function to verify Ed25519 signing
bool testEd25519() {
    // Test vector from RFC8032
    uint8_t seed[32] = {
        0x9d, 0x61, 0xb1, 0x9d, 0xef, 0xfd, 0x5a, 0x60, 0xba, 0x84, 0x4a, 0xf4, 0x92, 0xec, 0x2c, 0xc4,
        0x44, 0x49, 0xc5, 0x69, 0x7b, 0x32, 0x69, 0x19, 0x70, 0x3b, 0xac, 0x03, 0x1c, 0xae, 0x7f, 0x60
    };
    
    uint8_t expected_pubkey[32] = {
        0xd7, 0x5a, 0x98, 0x01, 0x82, 0xb1, 0x0a, 0xb7, 0xd5, 0x4b, 0xfe, 0xd3, 0xc9, 0x64, 0x07, 0x3a,
        0x0e, 0xe1, 0x72, 0xf3, 0xda, 0xa6, 0x23, 0x25, 0xaf, 0x02, 0x1a, 0x68, 0xf7, 0x07, 0x51, 0x1a
    };
    
    uint8_t expected_signature[64] = {
        0xe5, 0x56, 0x43, 0x00, 0xc3, 0x60, 0xac, 0x72, 0x90, 0x86, 0xe2, 0xcc, 0x80, 0x6e, 0x82, 0x8a,
        0x84, 0x87, 0x7f, 0x1e, 0xb8, 0xe5, 0xd9, 0x74, 0xd8, 0x73, 0xe0, 0x65, 0x22, 0x49, 0x01, 0x55,
        0x5f, 0xb8, 0x82, 0x15, 0x90, 0xa3, 0x3b, 0xac, 0xc6, 0x1e, 0x39, 0x70, 0x1c, 0xf9, 0xb4, 0x6b,
        0xd2, 0x5b, 0xf5, 0xf0, 0x59, 0x5b, 0xbe, 0x24, 0x65, 0x51, 0x41, 0x43, 0x8e, 0x7a, 0x10, 0x0b
    };
    
    uint8_t publicKey[32];
    uint8_t privateKey[64];
    if (!generateKeypairFromSeed(seed, publicKey, privateKey)) {
        Serial.println("Keypair generation failed");
        return false;
    }
    
    // Check public key
    Serial.print("Generated pubkey: ");
    for (int i = 0; i < 32; i++) {
        if (publicKey[i] < 16) Serial.print("0");
        Serial.print(publicKey[i], HEX);
    }
    Serial.println();
    
    Serial.print("Expected pubkey: ");
    for (int i = 0; i < 32; i++) {
        if (expected_pubkey[i] < 16) Serial.print("0");
        Serial.print(expected_pubkey[i], HEX);
    }
    Serial.println();
    
    if (memcmp(publicKey, expected_pubkey, 32) != 0) {
        Serial.println("Public key mismatch");
        return false;
    }
    
    // Sign empty message
    uint8_t message[0];
    uint8_t signature[64];
    if (!signMessage(message, 0, privateKey, signature)) {
        Serial.println("Signing failed");
        return false;
    }
    
    Serial.print("Generated signature: ");
    for (int i = 0; i < 64; i++) {
        if (signature[i] < 16) Serial.print("0");
        Serial.print(signature[i], HEX);
    }
    Serial.println();
    
    Serial.print("Expected signature: ");
    for (int i = 0; i < 64; i++) {
        if (expected_signature[i] < 16) Serial.print("0");
        Serial.print(expected_signature[i], HEX);
    }
    Serial.println();
    
    if (memcmp(signature, expected_signature, 64) != 0) {
        Serial.println("Signature mismatch");
        return false;
    }
    
    // Verify signature
    if (!verifySignature(message, 0, signature, publicKey)) {
        Serial.println("Verification failed");
        return false;
    }
    
    Serial.println("All tests passed");
    return true;
} 


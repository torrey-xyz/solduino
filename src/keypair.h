#ifndef SOLDUINO_KEYPAIR_H
#define SOLDUINO_KEYPAIR_H

#include <stdint.h>

// Constants
#define SOLDUINO_PUBLIC_KEY_LENGTH 32
#define SOLDUINO_SECRET_KEY_LENGTH 64
#define SOLDUINO_SEED_LENGTH 32
#define SOLDUINO_BASE58_PUBKEY_LENGTH 45  // Max length for base58 encoded public key

// Keypair structure - holds both public and secret key
typedef struct {
    uint8_t publicKey[SOLDUINO_PUBLIC_KEY_LENGTH];
    uint8_t secretKey[SOLDUINO_SECRET_KEY_LENGTH];
} Keypair;

#ifdef __cplusplus
extern "C" {
#endif

// Function declarations
bool keypair_from_seed(Keypair* keypair, const uint8_t* seed);
bool keypair_get_public_key_base58(const Keypair* keypair, char* out_b58, size_t out_size);
bool keypair_verify(const Keypair* keypair);

#ifdef __cplusplus
}
#endif

#endif // SOLDUINO_KEYPAIR_H 
#ifndef SOLDUINO_CRYPTO_H
#define SOLDUINO_CRYPTO_H

#include <stdint.h>
#include <stddef.h>

// Constants
#define SOLDUINO_ED25519_SIGNATURE_LENGTH 64

#ifdef __cplusplus
extern "C" {
#endif

// Function declarations
bool crypto_sign_message(
    const uint8_t* message,
    size_t message_len,
    const uint8_t secret_key[64],
    uint8_t signature[64]
);

bool crypto_verify_signature(
    const uint8_t* message,
    size_t message_len,
    const uint8_t public_key[32],
    const uint8_t signature[64]
);

#ifdef __cplusplus
}
#endif

#endif // SOLDUINO_CRYPTO_H 
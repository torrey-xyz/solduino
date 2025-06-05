#include "crypto.h"
#include "tweetnacl.h"

// TweetNaCl context
static unsigned char _context[crypto_sign_BYTES];

extern "C" {

bool crypto_sign_message(
    const uint8_t* message,
    size_t message_len,
    const uint8_t secret_key[64],
    uint8_t signature[64]
) {
    unsigned long long signed_msg_len;
    
    // Create signature using TweetNaCl
    int result = crypto_sign_detached(
        signature,                    // Output signature
        &signed_msg_len,             // Length of signature
        message,                     // Message to sign
        message_len,                 // Message length
        secret_key                   // Secret key (includes public key in last 32 bytes)
    );
    
    return result == 0;
}

bool crypto_verify_signature(
    const uint8_t* message,
    size_t message_len,
    const uint8_t public_key[32],
    const uint8_t signature[64]
) {
    // Verify signature using TweetNaCl
    int result = crypto_sign_verify_detached(
        signature,                    // Signature to verify
        message,                     // Original message
        message_len,                 // Message length
        public_key                   // Public key
    );
    
    return result == 0;
}

} // extern "C" 
#ifndef SOLDUINO_TRANSACTION_H
#define SOLDUINO_TRANSACTION_H

#include <stdint.h>
#include <stddef.h>
#include "keypair.h"

// Constants
#define SOLDUINO_SIGNATURE_LENGTH 64
#define SOLDUINO_MAX_SIGNATURES 1
#define SOLDUINO_BLOCKHASH_LENGTH 32

// Transaction structure
typedef struct {
    uint8_t signatures[SOLDUINO_MAX_SIGNATURES][SOLDUINO_SIGNATURE_LENGTH];
    uint8_t num_signatures;
    uint8_t recent_blockhash[SOLDUINO_BLOCKHASH_LENGTH];
    uint8_t* message;
    size_t message_len;
} Transaction;

#ifdef __cplusplus
extern "C" {
#endif

// Function declarations
bool transaction_create_transfer(
    Transaction* tx,
    const uint8_t* from_pubkey,
    const uint8_t* to_pubkey,
    uint64_t lamports,
    const uint8_t* recent_blockhash
);

bool transaction_serialize_message(
    const Transaction* tx,
    uint8_t* buffer,
    size_t buffer_size,
    size_t* written
);

bool transaction_add_signature(
    Transaction* tx,
    const uint8_t signature[SOLDUINO_SIGNATURE_LENGTH]
);

bool transaction_serialize(
    const Transaction* tx,
    uint8_t* buffer,
    size_t buffer_size,
    size_t* written
);

#ifdef __cplusplus
}
#endif

#endif // SOLDUINO_TRANSACTION_H 
#ifndef SOLDUINO_SPL_TOKEN_H
#define SOLDUINO_SPL_TOKEN_H

#include <stdint.h>
#include "account.h"
#include "program.h"

// Token Account Data
typedef struct {
    uint8_t mint[32];
    uint8_t owner[32];
    uint64_t amount;
    uint8_t delegate[32];
    uint8_t state;
    uint64_t is_native;
    uint64_t delegated_amount;
    uint8_t close_authority[32];
} TokenAccount;

// Token Mint Data
typedef struct {
    uint8_t mint_authority[32];
    uint64_t supply;
    uint8_t decimals;
    bool is_initialized;
    uint8_t freeze_authority[32];
} TokenMint;

#ifdef __cplusplus
extern "C" {
#endif

// Token Operations
bool token_create_mint(
    const Keypair* payer,
    const Keypair* mint_authority,
    uint8_t decimals,
    const uint8_t* freeze_authority,
    RpcClient* rpc,
    char* signature_out,
    size_t signature_out_size
);

bool token_create_account(
    const Keypair* payer,
    const uint8_t* mint,
    const uint8_t* owner,
    RpcClient* rpc,
    char* signature_out,
    size_t signature_out_size
);

bool token_create_associated_account(
    const Keypair* payer,
    const uint8_t* mint,
    const uint8_t* owner,
    RpcClient* rpc,
    char* signature_out,
    size_t signature_out_size
);

bool token_mint_to(
    const uint8_t* mint,
    const uint8_t* destination,
    const Keypair* authority,
    uint64_t amount,
    RpcClient* rpc,
    char* signature_out,
    size_t signature_out_size
);

bool token_transfer(
    const uint8_t* source,
    const uint8_t* destination,
    const Keypair* authority,
    uint64_t amount,
    RpcClient* rpc,
    char* signature_out,
    size_t signature_out_size
);

bool token_get_account_info(
    const uint8_t* account,
    RpcClient* rpc,
    TokenAccount* info
);

bool token_get_mint_info(
    const uint8_t* mint,
    RpcClient* rpc,
    TokenMint* info
);

#ifdef __cplusplus
}
#endif

#endif // SOLDUINO_SPL_TOKEN_H 
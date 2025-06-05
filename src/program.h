#ifndef SOLDUINO_PROGRAM_H
#define SOLDUINO_PROGRAM_H

#include <stdint.h>
#include "transaction.h"
#include "account.h"

// Constants
#define SOLDUINO_MAX_ACCOUNTS 4
#define SOLDUINO_MAX_INSTRUCTION_DATA 1024

typedef struct {
    uint8_t program_id[32];
    Account* accounts[SOLDUINO_MAX_ACCOUNTS];
    uint8_t num_accounts;
    uint8_t instruction_data[SOLDUINO_MAX_INSTRUCTION_DATA];
    size_t instruction_data_len;
} ProgramInstruction;

#ifdef __cplusplus
extern "C" {
#endif

// Program Interaction
bool program_create_instruction(
    ProgramInstruction* ix,
    const uint8_t* program_id,
    const uint8_t* instruction_data,
    size_t instruction_data_len
);

bool program_add_account(
    ProgramInstruction* ix,
    Account* account,
    bool is_signer,
    bool is_writable
);

bool program_execute(
    const ProgramInstruction* ix,
    RpcClient* rpc,
    const Keypair* payer,
    char* signature_out,
    size_t signature_out_size
);

// Built-in Program IDs
extern const uint8_t SYSTEM_PROGRAM_ID[32];
extern const uint8_t TOKEN_PROGRAM_ID[32];
extern const uint8_t ASSOCIATED_TOKEN_PROGRAM_ID[32];

#ifdef __cplusplus
}
#endif

#endif // SOLDUINO_PROGRAM_H 
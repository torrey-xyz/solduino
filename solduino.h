#ifndef SOLDUINO_H
#define SOLDUINO_H

// Version information
#define SOLDUINO_VERSION_MAJOR 0
#define SOLDUINO_VERSION_MINOR 1
#define SOLDUINO_VERSION_PATCH 0

// Core components
#include "src/keypair.h"
#include "src/serializer.h"
#include "src/transaction.h"
#include "src/rpc_client.h"
#include "src/crypto.h"

// Web3-like components
#include "src/account.h"
#include "src/program.h"
#include "src/connection.h"
#include "src/spl_token.h"

// Utility macros
#define SOLDUINO_LAMPORTS_TO_SOL(lamports) ((double)(lamports) / 1000000000.0)
#define SOLDUINO_SOL_TO_LAMPORTS(sol) ((uint64_t)((sol) * 1000000000.0))

#ifdef __cplusplus
extern "C" {
#endif

// Library initialization
bool solduino_init(void);

// Error handling
const char* solduino_get_last_error(void);
void solduino_clear_error(void);

#ifdef __cplusplus
}
#endif

#endif // SOLDUINO_H 
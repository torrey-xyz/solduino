#ifndef SOLDUINO_CONNECTION_H
#define SOLDUINO_CONNECTION_H

#include <stdint.h>
#include "rpc_client.h"

// Connection Configuration
typedef struct {
    char endpoint[SOLDUINO_MAX_URL_LENGTH];
    uint32_t commitment;
    uint32_t timeout_ms;
    bool use_websocket;
} ConnectionConfig;

// Connection Status
typedef struct {
    bool is_connected;
    uint32_t slot;
    char version[32];
    uint32_t ping_ms;
} ConnectionStatus;

typedef enum {
    COMMITMENT_PROCESSED = 0,
    COMMITMENT_CONFIRMED = 1,
    COMMITMENT_FINALIZED = 2
} Commitment;

#ifdef __cplusplus
extern "C" {
#endif

// Connection Management
bool connection_init(
    RpcClient* rpc,
    const char* endpoint,
    Commitment commitment
);

bool connection_get_status(
    RpcClient* rpc,
    ConnectionStatus* status
);

bool connection_get_slot(
    RpcClient* rpc,
    uint64_t* slot
);

bool connection_get_minimum_ledger_slot(
    RpcClient* rpc,
    uint64_t* slot
);

bool connection_get_genesis_hash(
    RpcClient* rpc,
    char* hash_out,
    size_t hash_out_size
);

bool connection_get_health(
    RpcClient* rpc,
    bool* is_healthy
);

#ifdef __cplusplus
}
#endif

#endif // SOLDUINO_CONNECTION_H 
#ifndef SOLDUINO_RPC_CLIENT_H
#define SOLDUINO_RPC_CLIENT_H

#include <stdint.h>
#include <stddef.h>

// Constants
#define SOLDUINO_MAX_URL_LENGTH 128
#define SOLDUINO_MAX_RESPONSE_SIZE 2048

typedef struct {
    char endpoint[SOLDUINO_MAX_URL_LENGTH];
} RpcClient;

#ifdef __cplusplus
extern "C" {
#endif

// Function declarations
bool rpc_client_init(RpcClient* client, const char* endpoint);
bool rpc_client_get_recent_blockhash(RpcClient* client, char* out_blockhash, size_t out_size);
bool rpc_client_send_transaction(
    RpcClient* client,
    const uint8_t* transaction_data,
    size_t transaction_size,
    char* out_signature,
    size_t out_size
);

#ifdef __cplusplus
}
#endif

#endif // SOLDUINO_RPC_CLIENT_H 
#ifndef RPC_CLIENT_H
#define RPC_CLIENT_H

#include <Arduino.h>
#include <WiFi.h>
#include <HTTPClient.h>
#include <WiFiClientSecure.h>
#include <WiFiClient.h>
#include <ArduinoJson.h>

struct AccountInfo {
    String owner;
    uint64_t lamports;
    String data;
    bool executable;
    uint64_t rentEpoch;
};

struct Balance {
    uint64_t value;
    String context;
};

struct BlockInfo {
    uint64_t slot;
    String blockhash;
    String previousBlockhash;
    uint64_t blockTime;
    int transactionCount;
};

struct TransactionResponse {
    String signature;
    int slot;
    String status;
    String error;
};

struct TokenAmount {
    uint64_t amount;
    uint8_t  decimals;
    String   uiAmountString;
};

struct TokenAccount {
    String   pubkey;
    String   mint;
    String   owner;
    uint64_t amount;
    uint8_t  decimals;
};

struct ProgramAccount {
    String      pubkey;
    AccountInfo account;
};

struct BlockCommitment {
    uint64_t commitment[32];
    uint64_t totalStake;
};

/**
 * Solana RPC Client for Arduino
 */
class RpcClient {
private:
    String rpcEndpoint;
    HTTPClient http;
    WiFiClientSecure* secureClient;
    WiFiClient* httpClient;
    bool useSecure;
    int requestId;
    int timeoutMs;

    String makeRpcRequest(const String& method, const String& params = "[]");
    bool extractResult(const String& response, DynamicJsonDocument& doc);
    void logError(const String& message);

public:
    explicit RpcClient(const String& endpoint);
    ~RpcClient();

    bool begin();
    void end();
    void setTimeout(int timeout);

    bool     getAccountInfo(const String& publicKey, AccountInfo& info);
    float    getBalance(const String& publicKey);
    uint64_t getBalanceLamports(const String& publicKey);
    uint64_t getBlockHeight();
    uint64_t getSlot();
    String   getVersion();
    bool     getHealth();

    String sendTransaction(const String& transaction);
    String sendTransactionBase58(const String& transaction);
    bool   getTransaction(const String& signature, TransactionResponse& tx);
    bool   getConfirmedTransaction(const String& signature, TransactionResponse& tx);

    bool   getBlock(uint64_t slot, BlockInfo& info);
    bool   getBlockCommitment(uint64_t slot, BlockCommitment& commitment);
    size_t getBlocks(uint64_t startSlot, uint64_t endSlot, uint64_t* buffer, size_t maxCount);

    size_t getProgramAccounts(const String& programId, ProgramAccount* buffer, size_t maxCount);
    size_t getTokenAccountsByOwner(const String& owner, const String& mint, TokenAccount* buffer, size_t maxCount);
    bool   getTokenSupply(const String& mint, TokenAmount& supply);

    String   getLatestBlockhash();
    bool     getLatestBlockhashBytes(uint8_t* blockhash);
    uint64_t getMinimumBalanceForRentExemption(size_t dataSize);
    uint64_t getFeeForMessage(const String& message);

    /**
     * Request an airdrop of SOL to an account
     * @param publicKey Public key address (Base58)
     * @param lamports Amount of lamports to airdrop (1 SOL = 1,000,000,000 lamports)
     * @return Transaction signature string (Base58), or empty string on error
     */
    String requestAirdrop(const String& publicKey, uint64_t lamports);

    String callRpc(const String& method, const String& params = "[]");

    String getEndpoint() const { return rpcEndpoint; }
    int    getTimeout() const { return timeoutMs; }
};

bool   parseAccountInfo(const String& jsonResponse, AccountInfo& info);
bool   parseBalance(const String& jsonResponse, Balance& balance);
bool   parseBlockInfo(const String& jsonResponse, BlockInfo& info);
bool   parseTransaction(const String& jsonResponse, TransactionResponse& tx);
bool   parseTokenAmount(const String& jsonResponse, TokenAmount& supply);
size_t parseTokenAccounts(const String& jsonResponse, TokenAccount* buffer, size_t maxCount);
size_t parseProgramAccounts(const String& jsonResponse, ProgramAccount* buffer, size_t maxCount);
bool   parseBlockCommitment(const String& jsonResponse, BlockCommitment& commitment);
size_t parseBlocks(const String& jsonResponse, uint64_t* buffer, size_t maxCount);

enum RpcClientStatus {
    RPC_DISCONNECTED,
    RPC_CONNECTING,
    RPC_CONNECTED,
    RPC_ERROR_STATE
};

#endif // RPC_CLIENT_H

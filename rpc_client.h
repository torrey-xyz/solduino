#ifndef RPC_CLIENT_H
#define RPC_CLIENT_H

#include <Arduino.h>
#include <WiFi.h>
#include <HTTPClient.h>
#include <WiFiClientSecure.h>
#include <WiFiClient.h>
#include <ArduinoJson.h>

struct AccountInfo;
struct Balance;
struct BlockInfo;
struct TransactionResponse;

/**
 * Solana RPC Client for Arduino
 * Provides basic RPC functionality similar to web3.js Connection class
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
    bool parseJsonResponse(const String& response, DynamicJsonDocument& doc);
    void logError(const String& message);

public:
    explicit RpcClient(const String& endpoint);
    ~RpcClient();
    
    bool begin();
    void end();
    void setTimeout(int timeout);
    
    String getAccountInfo(const String& publicKey);
    String getBalance(const String& publicKey);
    uint64_t getBalanceLamports(const String& publicKey); // Returns balance in lamports directly
    String getBlockHeight();
    String getSlot();
    String getVersion();
    String getHealth();
    
    String sendTransaction(const String& transaction);
    String sendTransactionBase58(const String& transaction); // Send transaction with base58 encoding
    String getTransaction(const String& signature);
    String getConfirmedTransaction(const String& signature);
    
    String getBlock(uint64_t slot);
    String getBlockCommitment(uint64_t slot);
    String getBlocks(uint64_t startSlot, uint64_t endSlot = 0);
    
    String getProgramAccounts(const String& programId);
    
    String getTokenAccountsByOwner(const String& owner, const String& mint = "");
    String getTokenSupply(const String& mint);
    
    String getRecentBlockhash(); // Deprecated - use getLatestBlockhash()
    String getLatestBlockhash();
    bool getLatestBlockhashBytes(uint8_t* blockhash); // Returns blockhash as bytes (32 bytes), returns false on error
    String getMinimumBalanceForRentExemption(size_t dataSize);
    String getFeeForMessage(const String& message);
    
    /**
     * Request an airdrop of SOL to an account
     * @param publicKey Public key address (Base58)
     * @param lamports Amount of lamports to airdrop (1 SOL = 1,000,000,000 lamports)
     * @return JSON RPC response with transaction signature
     */
    String requestAirdrop(const String& publicKey, uint64_t lamports);
    
    String callRpc(const String& method, const String& params = "[]");
    
    String getEndpoint() const { return rpcEndpoint; }
    int getTimeout() const { return timeoutMs; }
};

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

bool parseAccountInfo(const String& jsonResponse, AccountInfo& info);
bool parseBalance(const String& jsonResponse, Balance& balance);
bool parseBlockInfo(const String& jsonResponse, BlockInfo& info);
bool parseTransaction(const String& jsonResponse, TransactionResponse& tx);

enum RpcClientStatus {
    RPC_DISCONNECTED,
    RPC_CONNECTING,
    RPC_CONNECTED,
    RPC_ERROR_STATE
};

#endif // RPC_CLIENT_H
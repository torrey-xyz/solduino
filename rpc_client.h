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
    
    bool getAccountInfo(const String& publicKey, AccountInfo& info); // Returns AccountInfo struct, returns false on error
    String getBalance(const String& publicKey);
    uint64_t getBalanceLamports(const String& publicKey); // Returns balance in lamports directly
    uint64_t getBlockHeight(); // Returns block height directly, or 0 on error
    uint64_t getSlot(); // Returns current slot directly, or 0 on error
    String getVersion(); // Returns Solana version string (e.g., "1.18.0"), or empty string on error
    bool getHealth(); // Returns true if network is healthy, false otherwise
    
    String sendTransaction(const String& transaction);
    String sendTransactionBase58(const String& transaction); // Send transaction with base58 encoding
    bool getTransaction(const String& signature, TransactionResponse& tx); // Returns TransactionResponse struct, returns false on error
    String getConfirmedTransaction(const String& signature);
    
    bool getBlock(uint64_t slot, BlockInfo& info); // Returns BlockInfo struct, returns false on error
    String getBlockCommitment(uint64_t slot);
    String getBlocks(uint64_t startSlot, uint64_t endSlot = 0);
    
    String getProgramAccounts(const String& programId);
    
    String getTokenAccountsByOwner(const String& owner, const String& mint = "");
    String getTokenSupply(const String& mint);
    
    String getRecentBlockhash(); // Deprecated - use getLatestBlockhash()
    String getLatestBlockhash(); // Returns latest blockhash string (Base58), or empty string on error
    bool getLatestBlockhashBytes(uint8_t* blockhash); // Returns blockhash as bytes (32 bytes), returns false on error
    String getMinimumBalanceForRentExemption(size_t dataSize);
    String getFeeForMessage(const String& message);
    
    /**
     * Request an airdrop of SOL to an account
     * @param publicKey Public key address (Base58)
     * @param lamports Amount of lamports to airdrop (1 SOL = 1,000,000,000 lamports)
     * @return Transaction signature string (Base58), or empty string on error
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
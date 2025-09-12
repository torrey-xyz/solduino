#include "rpc_client.h"
#include "crypto.h"
#include "transaction.h"

RpcClient::RpcClient(const String& endpoint) 
    : rpcEndpoint(endpoint), requestId(1), timeoutMs(10000), secureClient(nullptr), httpClient(nullptr) {
    // Detect if endpoint uses HTTP or HTTPS
    useSecure = endpoint.startsWith("https://");
    
    if (useSecure) {
        secureClient = new WiFiClientSecure();
        // Skip certificate validation for HTTPS (insecure but works for testing)
        // In production, you should provide root CA certificates
        secureClient->setInsecure();
    } else {
        httpClient = new WiFiClient();
    }
}

RpcClient::~RpcClient() {
    end();
    if (secureClient) {
        delete secureClient;
        secureClient = nullptr;
    }
    if (httpClient) {
        delete httpClient;
        httpClient = nullptr;
    }
}

bool RpcClient::begin() {
    if (WiFi.status() != WL_CONNECTED) {
        logError("WiFi not connected");
        return false;
    }
    
    // Don't initialize http here - initialize it in makeRpcRequest for each request
    // This ensures we get a fresh connection each time
    String health = getHealth();
    return health.indexOf("ok") >= 0 || health.indexOf("\"ok\"") >= 0;
}

void RpcClient::end() {
    // HTTPClient will be cleaned up automatically when it goes out of scope
    // But we can call end() to ensure cleanup
    if (http.connected()) {
        http.end();
    }
}

void RpcClient::setTimeout(int timeout) {
    timeoutMs = timeout;
    // Timeout is applied when http.begin() is called in makeRpcRequest
}

String RpcClient::makeRpcRequest(const String& method, const String& params) {
    if (WiFi.status() != WL_CONNECTED) {
        logError("WiFi not connected");
        return "";
    }
    
    // Initialize HTTP client for this request
    bool beginSuccess = false;
    if (useSecure && secureClient) {
        beginSuccess = http.begin(*secureClient, rpcEndpoint);
    } else if (!useSecure && httpClient) {
        beginSuccess = http.begin(*httpClient, rpcEndpoint);
    }
    
    if (!beginSuccess) {
        logError("Failed to begin HTTP connection");
        return "";
    }
    
    http.addHeader("Content-Type", "application/json");
    http.setTimeout(timeoutMs);
    
    DynamicJsonDocument requestDoc(2048);
    requestDoc["jsonrpc"] = "2.0";
    requestDoc["id"] = requestId++;
    requestDoc["method"] = method;
    

    if (params != "[]" && params.length() > 0) {
        DynamicJsonDocument paramsDoc(1024);
        deserializeJson(paramsDoc, params);
        requestDoc["params"] = paramsDoc.as<JsonArray>();
    } else {
        requestDoc["params"] = JsonArray();
    }
    
    String requestBody;
    serializeJson(requestDoc, requestBody);
    
    int httpResponseCode = http.POST(requestBody);
    String response = "";
    
    if (httpResponseCode == HTTP_CODE_OK) {
        response = http.getString();
    } else {
        logError("HTTP Error: " + String(httpResponseCode));
        if (httpResponseCode < 0) {
            // Negative codes are connection errors
            String errorMsg = "Connection failed";
            if (httpResponseCode == -1) {
                errorMsg = "Connection failed (timeout or server unreachable)";
            } else if (httpResponseCode == -5) {
                errorMsg = "Connection lost";
            }
            logError(errorMsg);
        }
    }
    
    // Clean up the connection
    http.end();
    
    return response;
}

bool RpcClient::parseJsonResponse(const String& response, DynamicJsonDocument& doc) {
    DeserializationError error = deserializeJson(doc, response);
    
    if (error) {
        logError("JSON parse error: " + String(error.c_str()));
        return false;
    }
    
    if (doc.containsKey("error")) {
        logError("RPC Error: " + doc["error"]["message"].as<String>());
        return false;
    }
    
    return true;
}

void RpcClient::logError(const String& message) {
    Serial.println("[RPC_ERROR] " + message);
}

String RpcClient::getAccountInfo(const String& publicKey) {
    String params = "[\"" + publicKey + "\", {\"encoding\": \"base64\"}]";
    return makeRpcRequest("getAccountInfo", params);
}

String RpcClient::getBalance(const String& publicKey) {
    String params = "[\"" + publicKey + "\"]";
    String response = makeRpcRequest("getBalance", params);
    return response;
}

uint64_t RpcClient::getBalanceLamports(const String& publicKey) {
    String response = getBalance(publicKey);
    
    if (response.length() == 0) {
        return 0;
    }
    
    DynamicJsonDocument doc(512);
    DeserializationError error = deserializeJson(doc, response);
    
    if (error) {
        return 0;
    }
    
    if (!doc.containsKey("result")) {
        return 0;
    }
    
    if (!doc["result"].containsKey("value")) {
        return 0;
    }
    
    uint64_t balance = doc["result"]["value"].as<uint64_t>();
    
    return balance;
}

String RpcClient::getBlockHeight() {
    return makeRpcRequest("getBlockHeight");
}

String RpcClient::getSlot() {
    return makeRpcRequest("getSlot");
}

String RpcClient::getVersion() {
    return makeRpcRequest("getVersion");
}

String RpcClient::getHealth() {
    return makeRpcRequest("getHealth");
}

String RpcClient::sendTransaction(const String& transaction) {
    String params = "[\"" + transaction + "\", {\"encoding\": \"base64\"}]";
    return makeRpcRequest("sendTransaction", params);
}

String RpcClient::sendTransactionBase58(const String& transaction) {
    String params = "[\"" + transaction + "\", {\"encoding\": \"base58\"}]";
    return makeRpcRequest("sendTransaction", params);
}

String RpcClient::getTransaction(const String& signature) {
    String params = "[\"" + signature + "\", {\"encoding\": \"base64\"}]";
    return makeRpcRequest("getTransaction", params);
}

String RpcClient::getConfirmedTransaction(const String& signature) {
    String params = "[\"" + signature + "\", {\"encoding\": \"base64\"}]";
    return makeRpcRequest("getConfirmedTransaction", params);
}

String RpcClient::getBlock(uint64_t slot) {
    // Include maxSupportedTransactionVersion: 0 to support versioned transactions
    String params = "[" + String(slot) + ", {\"encoding\": \"base64\", \"maxSupportedTransactionVersion\": 0}]";
    return makeRpcRequest("getBlock", params);
}

String RpcClient::getBlockCommitment(uint64_t slot) {
    String params = "[" + String(slot) + "]";
    return makeRpcRequest("getBlockCommitment", params);
}

String RpcClient::getBlocks(uint64_t startSlot, uint64_t endSlot) {
    String params;
    if (endSlot > 0) {
        params = "[" + String(startSlot) + ", " + String(endSlot) + "]";
    } else {
        params = "[" + String(startSlot) + "]";
    }
    return makeRpcRequest("getBlocks", params);
}

String RpcClient::getProgramAccounts(const String& programId) {
    String params = "[\"" + programId + "\", {\"encoding\": \"base64\"}]";
    return makeRpcRequest("getProgramAccounts", params);
}

String RpcClient::getTokenAccountsByOwner(const String& owner, const String& mint) {
    String params;
    if (mint.length() > 0) {
        params = "[\"" + owner + "\", {\"mint\": \"" + mint + "\"}, {\"encoding\": \"base64\"}]";
    } else {
        params = "[\"" + owner + "\", {\"programId\": \"TokenkegQfeZyiNwAJbNbGKPFXCWuBvf9Ss623VQ5DA\"}, {\"encoding\": \"base64\"}]";
    }
    return makeRpcRequest("getTokenAccountsByOwner", params);
}

String RpcClient::getTokenSupply(const String& mint) {
    String params = "[\"" + mint + "\"]";
    return makeRpcRequest("getTokenSupply", params);
}

String RpcClient::getRecentBlockhash() {
    // Deprecated method - redirect to getLatestBlockhash for backward compatibility
    return getLatestBlockhash();
}

String RpcClient::getLatestBlockhash() {
    return makeRpcRequest("getLatestBlockhash");
}

bool RpcClient::getLatestBlockhashBytes(uint8_t* blockhash) {
    if (!blockhash) {
        return false;
    }
    
    String response = getLatestBlockhash();
    if (response.length() == 0) {
        return false;
    }
    
    DynamicJsonDocument doc(1024);
    DeserializationError error = deserializeJson(doc, response);
    
    if (error || !doc.containsKey("result") || doc["result"].isNull()) {
        return false;
    }
    
    String blockhashBase58 = doc["result"]["value"]["blockhash"].as<String>();
    if (blockhashBase58.length() == 0) {
        return false;
    }
    
    return addressToPublicKey(blockhashBase58.c_str(), blockhash);
}

String RpcClient::getMinimumBalanceForRentExemption(size_t dataSize) {
    String params = "[" + String(dataSize) + "]";
    return makeRpcRequest("getMinimumBalanceForRentExemption", params);
}

String RpcClient::getFeeForMessage(const String& message) {
    String params = "[\"" + message + "\"]";
    return makeRpcRequest("getFeeForMessage", params);
}

String RpcClient::requestAirdrop(const String& publicKey, uint64_t lamports) {
    // Maximum airdrop amount is typically 2 SOL (2,000,000,000 lamports) on devnet
    // On localnet, there's usually no limit
    String params = "[\"" + publicKey + "\", " + String(lamports) + "]";
    String response = makeRpcRequest("requestAirdrop", params);
    return response;
}

String RpcClient::callRpc(const String& method, const String& params) {
    return makeRpcRequest(method, params);
}


bool parseAccountInfo(const String& jsonResponse, AccountInfo& info) {
    DynamicJsonDocument doc(2048);
    DeserializationError error = deserializeJson(doc, jsonResponse);
    
    if (error || !doc.containsKey("result") || doc["result"].isNull()) {
        return false;
    }
    
    JsonObject result = doc["result"]["value"];
    if (result.isNull()) {
        return false;
    }
    
    info.owner = result["owner"].as<String>();
    info.lamports = result["lamports"].as<uint64_t>();
    info.data = result["data"][0].as<String>();
    info.executable = result["executable"].as<bool>();
    info.rentEpoch = result["rentEpoch"].as<uint64_t>();
    
    return true;
}

bool parseBalance(const String& jsonResponse, Balance& balance) {
    DynamicJsonDocument doc(1024);
    DeserializationError error = deserializeJson(doc, jsonResponse);
    
    if (error || !doc.containsKey("result")) {
        return false;
    }
    
    balance.value = doc["result"]["value"].as<uint64_t>();
    balance.context = doc["result"]["context"]["slot"].as<String>();
    
    return true;
}

bool parseBlockInfo(const String& jsonResponse, BlockInfo& info) {
    DynamicJsonDocument doc(4096);
    DeserializationError error = deserializeJson(doc, jsonResponse);
    
    if (error || !doc.containsKey("result") || doc["result"].isNull()) {
        return false;
    }
    
    JsonObject result = doc["result"];
    
    info.slot = result["parentSlot"].as<uint64_t>() + 1;
    info.blockhash = result["blockhash"].as<String>();
    info.previousBlockhash = result["previousBlockhash"].as<String>();
    info.blockTime = result["blockTime"].as<uint64_t>();
    info.transactionCount = result["transactions"].size();
    
    return true;
}

bool parseTransaction(const String& jsonResponse, TransactionResponse& tx) {
    DynamicJsonDocument doc(4096);
    DeserializationError error = deserializeJson(doc, jsonResponse);
    
    if (error || !doc.containsKey("result") || doc["result"].isNull()) {
        return false;
    }
    
    JsonObject result = doc["result"];
    
    tx.slot = result["slot"].as<int>();
    tx.status = result["meta"]["status"].as<String>();
    
    if (result["meta"].containsKey("err") && !result["meta"]["err"].isNull()) {
        tx.error = result["meta"]["err"].as<String>();
    } else {
        tx.error = "";
    }
    
    return true;
}
#include "rpc_client.h"
#include "crypto.h"
#include "transaction.h"

RpcClient::RpcClient(const String& endpoint)
    : rpcEndpoint(endpoint), requestId(1), timeoutMs(10000), secureClient(nullptr), httpClient(nullptr) {
    useSecure = endpoint.startsWith("https://");

    if (useSecure) {
        secureClient = new WiFiClientSecure();
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
    return getHealth();
}

void RpcClient::end() {
    if (http.connected()) {
        http.end();
    }
}

void RpcClient::setTimeout(int timeout) {
    timeoutMs = timeout;
}

String RpcClient::makeRpcRequest(const String& method, const String& params) {
    if (WiFi.status() != WL_CONNECTED) {
        logError("WiFi not connected");
        return "";
    }

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
            String errorMsg = "Connection failed";
            if (httpResponseCode == -1) {
                errorMsg = "Connection failed (timeout or server unreachable)";
            } else if (httpResponseCode == -5) {
                errorMsg = "Connection lost";
            }
            logError(errorMsg);
        }
    }

    http.end();
    return response;
}

bool RpcClient::extractResult(const String& response, DynamicJsonDocument& doc) {
    if (response.length() == 0) {
        return false;
    }
    DeserializationError error = deserializeJson(doc, response);
    if (error) {
        logError("JSON parse error: " + String(error.c_str()));
        return false;
    }
    if (doc.containsKey("error")) {
        logError("RPC Error: " + doc["error"]["message"].as<String>());
        return false;
    }
    if (!doc.containsKey("result") || doc["result"].isNull()) {
        return false;
    }
    return true;
}

void RpcClient::logError(const String& message) {
    Serial.println("[RPC_ERROR] " + message);
}

// ============================================================================
// Balances / chain state
// ============================================================================

bool RpcClient::getAccountInfo(const String& publicKey, AccountInfo& info) {
    String params = "[\"" + publicKey + "\", {\"encoding\": \"base64\"}]";
    String response = makeRpcRequest("getAccountInfo", params);
    return parseAccountInfo(response, info);
}

uint64_t RpcClient::getBalanceLamports(const String& publicKey) {
    String params = "[\"" + publicKey + "\"]";
    String response = makeRpcRequest("getBalance", params);

    DynamicJsonDocument doc(512);
    if (!extractResult(response, doc)) return 0;

    if (!doc["result"].containsKey("value")) return 0;
    return doc["result"]["value"].as<uint64_t>();
}

float RpcClient::getBalance(const String& publicKey) {
    String params = "[\"" + publicKey + "\"]";
    String response = makeRpcRequest("getBalance", params);

    DynamicJsonDocument doc(512);
    if (!extractResult(response, doc)) return -1.0f;
    if (!doc["result"].containsKey("value")) return -1.0f;

    uint64_t lamports = doc["result"]["value"].as<uint64_t>();
    return (float)((double)lamports / 1000000000.0);
}

uint64_t RpcClient::getBlockHeight() {
    String response = makeRpcRequest("getBlockHeight");
    DynamicJsonDocument doc(512);
    if (!extractResult(response, doc)) return 0;
    return doc["result"].as<uint64_t>();
}

uint64_t RpcClient::getSlot() {
    String response = makeRpcRequest("getSlot");
    DynamicJsonDocument doc(512);
    if (!extractResult(response, doc)) return 0;
    return doc["result"].as<uint64_t>();
}

String RpcClient::getVersion() {
    String response = makeRpcRequest("getVersion");
    DynamicJsonDocument doc(512);
    if (!extractResult(response, doc)) return "";

    if (doc["result"].containsKey("solana-core")) {
        return doc["result"]["solana-core"].as<String>();
    }
    return "";
}

bool RpcClient::getHealth() {
    String response = makeRpcRequest("getHealth");
    if (response.length() == 0) return false;

    if (response.indexOf("ok") >= 0) return true;

    DynamicJsonDocument doc(512);
    if (!extractResult(response, doc)) return false;

    return doc["result"].as<String>() == "ok";
}

// ============================================================================
// Transactions
// ============================================================================

String RpcClient::sendTransaction(const String& transaction) {
    String params = "[\"" + transaction + "\", {\"encoding\": \"base64\"}]";
    String response = makeRpcRequest("sendTransaction", params);

    DynamicJsonDocument doc(1024);
    if (!extractResult(response, doc)) return "";
    return doc["result"].as<String>();
}

String RpcClient::sendTransactionBase58(const String& transaction) {
    String params = "[\"" + transaction + "\", {\"encoding\": \"base58\"}]";
    String response = makeRpcRequest("sendTransaction", params);

    DynamicJsonDocument doc(1024);
    if (!extractResult(response, doc)) return "";
    return doc["result"].as<String>();
}

bool RpcClient::getTransaction(const String& signature, TransactionResponse& tx) {
    String params = "[\"" + signature + "\", {\"encoding\": \"base64\"}]";
    String response = makeRpcRequest("getTransaction", params);

    tx.signature = signature;
    return parseTransaction(response, tx);
}

bool RpcClient::getConfirmedTransaction(const String& signature, TransactionResponse& tx) {
    String params = "[\"" + signature + "\", {\"encoding\": \"base64\"}]";
    String response = makeRpcRequest("getConfirmedTransaction", params);

    tx.signature = signature;
    return parseTransaction(response, tx);
}

// ============================================================================
// Blocks
// ============================================================================

bool RpcClient::getBlock(uint64_t slot, BlockInfo& info) {
    String params = "[" + String(slot) + ", {\"encoding\": \"base64\", \"maxSupportedTransactionVersion\": 0}]";
    String response = makeRpcRequest("getBlock", params);
    return parseBlockInfo(response, info);
}

bool RpcClient::getBlockCommitment(uint64_t slot, BlockCommitment& commitment) {
    String params = "[" + String(slot) + "]";
    String response = makeRpcRequest("getBlockCommitment", params);
    return parseBlockCommitment(response, commitment);
}

size_t RpcClient::getBlocks(uint64_t startSlot, uint64_t endSlot, uint64_t* buffer, size_t maxCount) {
    if (!buffer || maxCount == 0) return 0;

    String params;
    if (endSlot > 0) {
        params = "[" + String(startSlot) + ", " + String(endSlot) + "]";
    } else {
        params = "[" + String(startSlot) + "]";
    }
    String response = makeRpcRequest("getBlocks", params);
    return parseBlocks(response, buffer, maxCount);
}

// ============================================================================
// Accounts / tokens
// ============================================================================

size_t RpcClient::getProgramAccounts(const String& programId, ProgramAccount* buffer, size_t maxCount) {
    if (!buffer || maxCount == 0) return 0;

    String params = "[\"" + programId + "\", {\"encoding\": \"base64\"}]";
    String response = makeRpcRequest("getProgramAccounts", params);
    return parseProgramAccounts(response, buffer, maxCount);
}

size_t RpcClient::getTokenAccountsByOwner(const String& owner, const String& mint, TokenAccount* buffer, size_t maxCount) {
    if (!buffer || maxCount == 0) return 0;

    String params;
    if (mint.length() > 0) {
        params = "[\"" + owner + "\", {\"mint\": \"" + mint + "\"}, {\"encoding\": \"jsonParsed\"}]";
    } else {
        params = "[\"" + owner + "\", {\"programId\": \"TokenkegQfeZyiNwAJbNbGKPFXCWuBvf9Ss623VQ5DA\"}, {\"encoding\": \"jsonParsed\"}]";
    }
    String response = makeRpcRequest("getTokenAccountsByOwner", params);
    return parseTokenAccounts(response, buffer, maxCount);
}

bool RpcClient::getTokenSupply(const String& mint, TokenAmount& supply) {
    String params = "[\"" + mint + "\"]";
    String response = makeRpcRequest("getTokenSupply", params);
    return parseTokenAmount(response, supply);
}

// ============================================================================
// Blockhash / fees / rent
// ============================================================================

String RpcClient::getLatestBlockhash() {
    String response = makeRpcRequest("getLatestBlockhash");
    DynamicJsonDocument doc(1024);
    if (!extractResult(response, doc)) return "";

    if (doc["result"].containsKey("value") && doc["result"]["value"].containsKey("blockhash")) {
        return doc["result"]["value"]["blockhash"].as<String>();
    }
    return "";
}

bool RpcClient::getLatestBlockhashBytes(uint8_t* blockhash) {
    if (!blockhash) return false;

    String response = makeRpcRequest("getLatestBlockhash");
    DynamicJsonDocument doc(1024);
    if (!extractResult(response, doc)) return false;

    String blockhashBase58 = doc["result"]["value"]["blockhash"].as<String>();
    if (blockhashBase58.length() == 0) return false;

    return addressToPublicKey(blockhashBase58.c_str(), blockhash);
}

uint64_t RpcClient::getMinimumBalanceForRentExemption(size_t dataSize) {
    String params = "[" + String(dataSize) + "]";
    String response = makeRpcRequest("getMinimumBalanceForRentExemption", params);

    DynamicJsonDocument doc(512);
    if (!extractResult(response, doc)) return 0;
    return doc["result"].as<uint64_t>();
}

uint64_t RpcClient::getFeeForMessage(const String& message) {
    String params = "[\"" + message + "\"]";
    String response = makeRpcRequest("getFeeForMessage", params);

    DynamicJsonDocument doc(512);
    if (!extractResult(response, doc)) return 0;

    if (doc["result"].containsKey("value") && !doc["result"]["value"].isNull()) {
        return doc["result"]["value"].as<uint64_t>();
    }
    return 0;
}

String RpcClient::requestAirdrop(const String& publicKey, uint64_t lamports) {
    String params = "[\"" + publicKey + "\", " + String(lamports) + "]";
    String response = makeRpcRequest("requestAirdrop", params);

    DynamicJsonDocument doc(1024);
    if (!extractResult(response, doc)) return "";
    return doc["result"].as<String>();
}

String RpcClient::callRpc(const String& method, const String& params) {
    return makeRpcRequest(method, params);
}

// ============================================================================
// Free-standing parsers
// ============================================================================

bool parseAccountInfo(const String& jsonResponse, AccountInfo& info) {
    DynamicJsonDocument doc(2048);
    DeserializationError error = deserializeJson(doc, jsonResponse);
    if (error || !doc.containsKey("result") || doc["result"].isNull()) return false;

    JsonObject result = doc["result"]["value"];
    if (result.isNull()) return false;

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
    if (error || !doc.containsKey("result")) return false;

    balance.value = doc["result"]["value"].as<uint64_t>();
    balance.context = doc["result"]["context"]["slot"].as<String>();
    return true;
}

bool parseBlockInfo(const String& jsonResponse, BlockInfo& info) {
    DynamicJsonDocument doc(4096);
    DeserializationError error = deserializeJson(doc, jsonResponse);
    if (error || !doc.containsKey("result") || doc["result"].isNull()) return false;

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
    if (error || !doc.containsKey("result") || doc["result"].isNull()) return false;

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

bool parseTokenAmount(const String& jsonResponse, TokenAmount& supply) {
    DynamicJsonDocument doc(1024);
    DeserializationError error = deserializeJson(doc, jsonResponse);
    if (error || !doc.containsKey("result") || doc["result"].isNull()) return false;

    JsonObject value = doc["result"]["value"];
    if (value.isNull()) return false;

    supply.amount = strtoull(value["amount"].as<const char*>(), nullptr, 10);
    supply.decimals = value["decimals"].as<uint8_t>();
    supply.uiAmountString = value["uiAmountString"].as<String>();
    return true;
}

size_t parseTokenAccounts(const String& jsonResponse, TokenAccount* buffer, size_t maxCount) {
    if (!buffer || maxCount == 0) return 0;

    DynamicJsonDocument doc(8192);
    DeserializationError error = deserializeJson(doc, jsonResponse);
    if (error || !doc.containsKey("result") || doc["result"].isNull()) return 0;

    JsonArray arr = doc["result"]["value"];
    if (arr.isNull()) return 0;

    size_t n = 0;
    for (JsonObject entry : arr) {
        if (n >= maxCount) break;
        TokenAccount& t = buffer[n];
        t.pubkey = entry["pubkey"].as<String>();
        JsonObject parsed = entry["account"]["data"]["parsed"]["info"];
        t.mint = parsed["mint"].as<String>();
        t.owner = parsed["owner"].as<String>();
        JsonObject amt = parsed["tokenAmount"];
        t.amount = strtoull(amt["amount"].as<const char*>(), nullptr, 10);
        t.decimals = amt["decimals"].as<uint8_t>();
        n++;
    }
    return n;
}

size_t parseProgramAccounts(const String& jsonResponse, ProgramAccount* buffer, size_t maxCount) {
    if (!buffer || maxCount == 0) return 0;

    DynamicJsonDocument doc(8192);
    DeserializationError error = deserializeJson(doc, jsonResponse);
    if (error || !doc.containsKey("result") || doc["result"].isNull()) return 0;

    JsonArray arr = doc["result"];
    if (arr.isNull()) return 0;

    size_t n = 0;
    for (JsonObject entry : arr) {
        if (n >= maxCount) break;
        ProgramAccount& p = buffer[n];
        p.pubkey = entry["pubkey"].as<String>();
        JsonObject acc = entry["account"];
        p.account.owner = acc["owner"].as<String>();
        p.account.lamports = acc["lamports"].as<uint64_t>();
        p.account.data = acc["data"][0].as<String>();
        p.account.executable = acc["executable"].as<bool>();
        p.account.rentEpoch = acc["rentEpoch"].as<uint64_t>();
        n++;
    }
    return n;
}

bool parseBlockCommitment(const String& jsonResponse, BlockCommitment& commitment) {
    DynamicJsonDocument doc(2048);
    DeserializationError error = deserializeJson(doc, jsonResponse);
    if (error || !doc.containsKey("result") || doc["result"].isNull()) return false;

    JsonObject result = doc["result"];
    commitment.totalStake = result["totalStake"].as<uint64_t>();

    for (size_t i = 0; i < 32; i++) commitment.commitment[i] = 0;

    if (!result["commitment"].isNull()) {
        JsonArray arr = result["commitment"];
        size_t i = 0;
        for (JsonVariant v : arr) {
            if (i >= 32) break;
            commitment.commitment[i++] = v.as<uint64_t>();
        }
    }
    return true;
}

size_t parseBlocks(const String& jsonResponse, uint64_t* buffer, size_t maxCount) {
    if (!buffer || maxCount == 0) return 0;

    DynamicJsonDocument doc(4096);
    DeserializationError error = deserializeJson(doc, jsonResponse);
    if (error || !doc.containsKey("result") || doc["result"].isNull()) return 0;

    JsonArray arr = doc["result"];
    if (arr.isNull()) return 0;

    size_t n = 0;
    for (JsonVariant v : arr) {
        if (n >= maxCount) break;
        buffer[n++] = v.as<uint64_t>();
    }
    return n;
}

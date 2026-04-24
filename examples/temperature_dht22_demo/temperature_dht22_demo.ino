/**
 * Solduino DHT22 Temperature+Humidity Sensor-to-Chain Demo
 *
 * Reads DHT22 temperature/humidity and periodically pushes both values
 * to a Solana program.
 *
 * On-chain instruction assumption:
 *   record_data(value1: i64, value2: i64, timestamp: i64)
 *
 * Encoding:
 *   value1 = tempC * 100
 *   value2 = humidityPct * 100
 *
 * Required extra libraries:
 *   - DHT sensor library
 *   - Adafruit Unified Sensor (dependency of DHT library on many setups)
 */

#include <WiFi.h>
#include <solduino.h>
#include <ArduinoJson.h>
#include <DHT.h>

// ============================================================================
// Configuration -- EDIT THESE
// ============================================================================

const char* ssid     = "YOUR_WIFI_SSID";
const char* password = "YOUR_WIFI_PASSWORD";

const String RPC_ENDPOINT = SOLDUINO_DEVNET_RPC;
const char* PROGRAM_ID_BASE58 = "YourProgramId1111111111111111111111111111111";
const char* AUTHORITY_PRIVATE_KEY_BASE58 = "YourBase58PrivateKeyHere";

const int DHT_PIN = 4;
#define DHTTYPE DHT22

const uint32_t READING_INTERVAL_MS = 10000;
const uint32_t POLL_INTERVAL_MS   = 1000;
const uint32_t CONFIRM_TIMEOUT_MS = 30000;

const uint8_t RECORD_DATA_DISCRIMINATOR[8] = {
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00
};

// ============================================================================
// Global State
// ============================================================================

Solduino solduino;
RpcClient rpcClient(RPC_ENDPOINT);

Keypair authorityKeypair;
uint8_t authorityPub[SOLDUINO_PUBKEY_SIZE];
uint8_t programId[SOLDUINO_PUBKEY_SIZE];
uint8_t dataAccountPda[SOLDUINO_PUBKEY_SIZE];
uint8_t pdaBump;

DHT dht(DHT_PIN, DHTTYPE);

static char g_txBuf[2048];

// ============================================================================
// Sensor Read
// ============================================================================

bool readDhtValues(int64_t& tempX100, int64_t& humidityX100) {
    float h = dht.readHumidity();
    float t = dht.readTemperature();

    if (isnan(h) || isnan(t)) {
        Serial.println("  [ERROR] DHT read failed");
        return false;
    }

    tempX100 = (int64_t)(t * 100.0f);
    humidityX100 = (int64_t)(h * 100.0f);

    Serial.print("  Temp(C): ");
    Serial.print(t, 2);
    Serial.print("  Humidity(%): ");
    Serial.print(h, 2);
    Serial.print("  Encoded: ");
    Serial.print(tempX100);
    Serial.print(", ");
    Serial.println(humidityX100);

    return true;
}

// ============================================================================
// Transaction Push
// ============================================================================

bool pushSensorData(int64_t tempX100, int64_t humidityX100) {
    int64_t timestamp = (int64_t)(millis() / 1000);

    Instruction ix;
    ix.setProgram(programId);
    ix.addKey(authorityPub, true, true);
    ix.addKey(dataAccountPda, false, true);
    ix.addKey(SystemProgram::PROGRAM_ID, false, false);
    ix.writeBytes(RECORD_DATA_DISCRIMINATOR, 8);
    ix.writeI64LE(tempX100);
    ix.writeI64LE(humidityX100);
    ix.writeI64LE(timestamp);

    Transaction tx;
    tx.add(ix);

    uint8_t blockhash[BLOCKHASH_SIZE];
    if (!rpcClient.getLatestBlockhashBytes(blockhash)) return false;
    if (!tx.setRecentBlockhash(blockhash)) return false;
    if (!tx.sign(authorityKeypair)) return false;
    if (!TransactionSerializer::encodeTransactionBase58(tx, g_txBuf, sizeof(g_txBuf))) return false;

    String resp = rpcClient.sendTransactionBase58(g_txBuf);
    DynamicJsonDocument doc(512);
    deserializeJson(doc, resp);
    if (!doc.containsKey("result")) {
        Serial.println("  [ERROR] sendTransaction failed");
        return false;
    }

    String sig = doc["result"].as<String>();
    Serial.print("  Sent signature: ");
    Serial.println(sig);

    uint32_t start = millis();
    while (millis() - start < CONFIRM_TIMEOUT_MS) {
        TransactionResponse txResp;
        if (rpcClient.getTransaction(sig, txResp)) return true;
        delay(POLL_INTERVAL_MS);
    }
    Serial.println("  [WARN] Confirmation timeout");
    return true;
}

// ============================================================================
// Boot Helpers
// ============================================================================

void connectToWiFi() {
    WiFi.mode(WIFI_STA);
    WiFi.begin(ssid, password);
    int attempts = 0;
    while (WiFi.status() != WL_CONNECTED && attempts < 20) {
        delay(500);
        Serial.print(".");
        attempts++;
    }
    Serial.println(WiFi.status() == WL_CONNECTED ? " Connected!" : " Failed!");
}

void setup() {
    Serial.begin(115200);
    delay(1000);

    Serial.println("\n=== DHT22 Sensor-to-Chain Demo ===\n");

    solduino.begin();

    Serial.println("Connecting to WiFi...");
    connectToWiFi();
    if (WiFi.status() != WL_CONNECTED) {
        Serial.println("[FATAL] No WiFi -- halting.");
        while (true) delay(1000);
    }

    rpcClient.begin();
    rpcClient.setTimeout(15000);

    if (!authorityKeypair.importFromPrivateKeyBase58(AUTHORITY_PRIVATE_KEY_BASE58)) {
        Serial.println("[FATAL] Invalid private key -- halting.");
        while (true) delay(1000);
    }
    authorityKeypair.getPublicKey(authorityPub);

    if (!addressToPublicKey(PROGRAM_ID_BASE58, programId)) {
        Serial.println("[FATAL] Invalid program ID -- halting.");
        while (true) delay(1000);
    }

    const uint8_t seed1[] = "sensor";
    const uint8_t* seeds[] = { seed1, authorityPub };
    const size_t seedLens[] = { 6, SOLDUINO_PUBKEY_SIZE };
    if (!findProgramAddress(seeds, seedLens, 2, programId, dataAccountPda, &pdaBump)) {
        Serial.println("[FATAL] PDA derivation failed -- halting.");
        while (true) delay(1000);
    }

    dht.begin();

    Serial.println("Setup complete.\n");
}

static uint32_t lastReading = 0;
static uint32_t readingCount = 0;

void loop() {
    if (millis() - lastReading < READING_INTERVAL_MS) {
        delay(100);
        return;
    }
    lastReading = millis();
    readingCount++;

    Serial.print("=== Reading #");
    Serial.print(readingCount);
    Serial.println(" ===");

    int64_t tempX100 = 0;
    int64_t humidityX100 = 0;
    if (!readDhtValues(tempX100, humidityX100)) {
        Serial.println("  Skipping push due to sensor read error.\n");
        return;
    }

    if (pushSensorData(tempX100, humidityX100)) {
        Serial.println("  Data stored on-chain successfully!");
    } else {
        Serial.println("  Failed to store data on-chain.");
    }
    Serial.println();
}

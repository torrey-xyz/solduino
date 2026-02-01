/**
 * Solduino Thermocouple (MAX6675) Sensor-to-Chain Demo
 *
 * Reads temperature from a K-type thermocouple via MAX6675 and periodically
 * pushes temperature to a Solana program.
 *
 * On-chain instruction assumption:
 *   record_data(value: i64, timestamp: i64)
 *
 * Encoding:
 *   value = tempC * 100 (fixed-point, 2 decimals)
 *
 * Required extra library:
 *   - max6675
 */

#include <WiFi.h>
#include <solduino.h>
#include <ArduinoJson.h>
#include <max6675.h>

// ============================================================================
// Configuration -- EDIT THESE
// ============================================================================

const char* ssid     = "YOUR_WIFI_SSID";
const char* password = "YOUR_WIFI_PASSWORD";

const String RPC_ENDPOINT = SOLDUINO_DEVNET_RPC;
const char* PROGRAM_ID_BASE58 = "YourProgramId1111111111111111111111111111111";
const char* AUTHORITY_PRIVATE_KEY_BASE58 = "YourBase58PrivateKeyHere";

// MAX6675 pins (ESP32)
const int THERMO_SCK_PIN = 18;
const int THERMO_CS_PIN  = 5;
const int THERMO_SO_PIN  = 19;

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
uint8_t authorityPriv[SOLDUINO_SECRETKEY_SIZE];
uint8_t programId[SOLDUINO_PUBKEY_SIZE];
uint8_t dataAccountPda[SOLDUINO_PUBKEY_SIZE];
uint8_t pdaBump;

MAX6675 thermocouple(THERMO_SCK_PIN, THERMO_CS_PIN, THERMO_SO_PIN);

static char g_txBuf[2048];

// ============================================================================
// Sensor Read
// ============================================================================

int64_t readThermocoupleTempX100() {
    // MAX6675 library returns Celsius as float
    float tempC = thermocouple.readCelsius();
    int64_t value = (int64_t)(tempC * 100.0f);

    Serial.print("  Temp(C): ");
    Serial.print(tempC, 2);
    Serial.print("  Encoded: ");
    Serial.println(value);

    return value;
}

// ============================================================================
// Transaction Push
// ============================================================================

bool pushSensorData(int64_t sensorValue) {
    int64_t timestamp = (int64_t)(millis() / 1000);

    Instruction ix;
    ix.setProgram(programId);
    ix.addKey(authorityPub, true, true);
    ix.addKey(dataAccountPda, false, true);
    ix.addKey(SystemProgram::PROGRAM_ID, false, false);
    ix.writeBytes(RECORD_DATA_DISCRIMINATOR, 8);
    ix.writeI64LE(sensorValue);
    ix.writeI64LE(timestamp);

    Transaction tx;
    tx.add(ix);

    uint8_t blockhash[BLOCKHASH_SIZE];
    if (!rpcClient.getLatestBlockhashBytes(blockhash)) return false;
    if (!tx.setRecentBlockhash(blockhash)) return false;
    if (!tx.sign(authorityPriv, authorityPub)) return false;
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

    Serial.println("\n=== Thermocouple Sensor-to-Chain Demo ===\n");

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
    authorityKeypair.getPrivateKey(authorityPriv);

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

    // MAX6675 needs a short warm-up after power-up.
    delay(500);

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

    int64_t value = readThermocoupleTempX100();
    if (pushSensorData(value)) {
        Serial.println("  Data stored on-chain successfully!");
    } else {
        Serial.println("  Failed to store data on-chain.");
    }
    Serial.println();
}

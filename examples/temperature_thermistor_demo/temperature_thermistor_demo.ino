/**
 * Solduino Thermistor Temperature Sensor-to-Chain Demo
 *
 * Reads a thermistor (analog voltage divider) on ESP32 and periodically
 * pushes temperature readings to a Solana program.
 *
 * On-chain instruction assumption:
 *   record_data(value: i64, timestamp: i64)
 *
 * Encoding:
 *   value = tempC * 100 (fixed-point, 2 decimals)
 *
 * Hardware:
 *   - ESP32
 *   - NTC thermistor + fixed resistor in voltage divider
 *   - Divider output connected to SENSOR_PIN
 *
 * Notes:
 *   - Tune THERMISTOR_NOMINAL, SERIES_RESISTOR, and BETA_COEFFICIENT
 *     for your thermistor model.
 */

#include <WiFi.h>
#include <solduino.h>
#include <ArduinoJson.h>
#include <math.h>

// ============================================================================
// Configuration -- EDIT THESE
// ============================================================================

const char* ssid     = "YOUR_WIFI_SSID";
const char* password = "YOUR_WIFI_PASSWORD";

const String RPC_ENDPOINT = SOLDUINO_DEVNET_RPC;
const char* PROGRAM_ID_BASE58 = "YourProgramId1111111111111111111111111111111";
const char* AUTHORITY_PRIVATE_KEY_BASE58 = "YourBase58PrivateKeyHere";

// Sensor + schedule
const int SENSOR_PIN = 34;
const uint32_t READING_INTERVAL_MS = 10000;

// Thermistor model parameters (typical 10k NTC defaults)
const float THERMISTOR_NOMINAL = 10000.0f;   // ohms at 25C
const float TEMPERATURE_NOMINAL = 25.0f;     // 25C
const float BETA_COEFFICIENT = 3950.0f;      // beta value
const float SERIES_RESISTOR = 10000.0f;      // voltage divider fixed resistor

// Tx settings
const uint32_t POLL_INTERVAL_MS   = 1000;
const uint32_t CONFIRM_TIMEOUT_MS = 30000;

// Anchor discriminator for "record_data"
// first 8 bytes of SHA-256("global:record_data")
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

static char g_txBuf[2048];

// ============================================================================
// Sensor Read
// ============================================================================

int64_t readThermistorTempX100() {
    int raw = analogRead(SENSOR_PIN);
    if (raw <= 0) raw = 1;
    if (raw >= 4095) raw = 4094;

    float resistance = SERIES_RESISTOR * ((4095.0f / (float)raw) - 1.0f);

    float steinhart = resistance / THERMISTOR_NOMINAL;
    steinhart = logf(steinhart);
    steinhart /= BETA_COEFFICIENT;
    steinhart += 1.0f / (TEMPERATURE_NOMINAL + 273.15f);
    steinhart = 1.0f / steinhart;
    float tempC = steinhart - 273.15f;

    int64_t value = (int64_t)(tempC * 100.0f);

    Serial.print("  ADC: ");
    Serial.print(raw);
    Serial.print("  Resistance(ohm): ");
    Serial.print(resistance, 1);
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

    Serial.println("\n=== Thermistor Sensor-to-Chain Demo ===\n");

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

    analogReadResolution(12);
    pinMode(SENSOR_PIN, INPUT);

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

    int64_t value = readThermistorTempX100();
    if (pushSensorData(value)) {
        Serial.println("  Data stored on-chain successfully!");
    } else {
        Serial.println("  Failed to store data on-chain.");
    }
    Serial.println();
}

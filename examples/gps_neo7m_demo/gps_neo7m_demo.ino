/**
 * Solduino GPS (NEO-7M) Sensor-to-Chain Demo
 *
 * Reads GPS coordinates from a NEO-7M module over UART and periodically pushes
 * position data to a Solana program.
 *
 * On-chain instruction assumption:
 *   record_data(latE7: i64, lngE7: i64, sats: i64, timestamp: i64)
 *
 * Encoding:
 *   latE7 = latitude * 1e7
 *   lngE7 = longitude * 1e7
 *   sats  = satellites count
 *
 * Required extra library:
 *   - TinyGPSPlus
 */

#include <WiFi.h>
#include <solduino.h>
#include <ArduinoJson.h>
#include <TinyGPS++.h>

// ============================================================================
// Configuration -- EDIT THESE
// ============================================================================

const char* ssid     = "YOUR_WIFI_SSID";
const char* password = "YOUR_WIFI_PASSWORD";

const String RPC_ENDPOINT = SOLDUINO_DEVNET_RPC;
const char* PROGRAM_ID_BASE58 = "YourProgramId1111111111111111111111111111111";
const char* AUTHORITY_PRIVATE_KEY_BASE58 = "YourBase58PrivateKeyHere";

// NEO-7M UART pins (ESP32)
const int GPS_RX_PIN = 16;   // ESP32 RX <- GPS TX
const int GPS_TX_PIN = 17;   // ESP32 TX -> GPS RX (optional)
const uint32_t GPS_BAUD = 9600;

const uint32_t READING_INTERVAL_MS = 15000;
const uint32_t GPS_FIX_WAIT_MS = 5000;

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

TinyGPSPlus gps;
HardwareSerial gpsSerial(1);

static char g_txBuf[2048];

// ============================================================================
// Sensor Read
// ============================================================================

bool readGpsFix(int64_t& latE7, int64_t& lngE7, int64_t& sats) {
    uint32_t start = millis();
    while (millis() - start < GPS_FIX_WAIT_MS) {
        while (gpsSerial.available() > 0) {
            gps.encode(gpsSerial.read());
        }
        if (gps.location.isValid()) {
            latE7 = (int64_t)(gps.location.lat() * 10000000.0);
            lngE7 = (int64_t)(gps.location.lng() * 10000000.0);
            sats = gps.satellites.isValid() ? (int64_t)gps.satellites.value() : 0;

            Serial.print("  latE7: ");
            Serial.print(latE7);
            Serial.print("  lngE7: ");
            Serial.print(lngE7);
            Serial.print("  sats: ");
            Serial.println(sats);
            return true;
        }
        delay(20);
    }

    Serial.println("  [WARN] No GPS fix in current window");
    return false;
}

// ============================================================================
// Transaction Push
// ============================================================================

bool pushSensorData(int64_t latE7, int64_t lngE7, int64_t sats) {
    int64_t timestamp = (int64_t)(millis() / 1000);

    Instruction ix;
    ix.setProgram(programId);
    ix.addKey(authorityPub, true, true);
    ix.addKey(dataAccountPda, false, true);
    ix.addKey(SystemProgram::PROGRAM_ID, false, false);
    ix.writeBytes(RECORD_DATA_DISCRIMINATOR, 8);
    ix.writeI64LE(latE7);
    ix.writeI64LE(lngE7);
    ix.writeI64LE(sats);
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

    Serial.println("\n=== GPS NEO-7M Sensor-to-Chain Demo ===\n");

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

    gpsSerial.begin(GPS_BAUD, SERIAL_8N1, GPS_RX_PIN, GPS_TX_PIN);

    Serial.println("Setup complete.\n");
}

static uint32_t lastReading = 0;
static uint32_t readingCount = 0;

void loop() {
    if (millis() - lastReading < READING_INTERVAL_MS) {
        while (gpsSerial.available() > 0) {
            gps.encode(gpsSerial.read());
        }
        delay(50);
        return;
    }
    lastReading = millis();
    readingCount++;

    Serial.print("=== Reading #");
    Serial.print(readingCount);
    Serial.println(" ===");

    int64_t latE7 = 0;
    int64_t lngE7 = 0;
    int64_t sats = 0;
    if (!readGpsFix(latE7, lngE7, sats)) {
        Serial.println("  Skipping push due to missing fix.\n");
        return;
    }

    if (pushSensorData(latE7, lngE7, sats)) {
        Serial.println("  Data stored on-chain successfully!");
    } else {
        Serial.println("  Failed to store data on-chain.");
    }
    Serial.println();
}

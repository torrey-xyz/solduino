/**
 * Solduino Sensor-to-Chain Demo
 *
 * Reads sensor data on an ESP32 and pushes it on-chain to a Solana
 * smart contract using the Instruction builder API.
 *
 * This example demonstrates a real IoT-to-blockchain workflow:
 *   1. Read a sensor value (temperature, humidity, or any analog input)
 *   2. Build an Instruction with Anchor-style discriminator + data payload
 *   3. Derive the PDA where the data will be stored
 *   4. Sign, serialize, send, and confirm the transaction
 *
 * The on-chain program is assumed to have an instruction like:
 *
 *   #[derive(Accounts)]
 *   pub struct RecordData<'info> {
 *       #[account(mut)]
 *       pub authority: Signer<'info>,
 *       #[account(mut, seeds = [b"sensor", authority.key().as_ref()], bump)]
 *       pub data_account: Account<'info, SensorData>,
 *       pub system_program: Program<'info, System>,
 *   }
 *
 *   pub fn record_data(ctx: Context<RecordData>, value: i64, timestamp: i64) -> Result<()>
 *
 * Hardware:
 *   - ESP32 (any variant)
 *   - Analog sensor connected to GPIO 34 (or change SENSOR_PIN below)
 *     Examples: TMP36 temperature sensor, photoresistor, soil moisture sensor
 *
 * Required Libraries:
 *   - ArduinoJson (via Library Manager)
 *   - Solduino
 *
 * Setup:
 *   1. Deploy your Anchor program and note its Program ID
 *   2. Update WiFi credentials, RPC_ENDPOINT, and PROGRAM_ID_BASE58 below
 *   3. Import the authority private key (or generate one and fund it)
 *   4. Upload to ESP32, open Serial Monitor at 115200 baud
 */

#include <WiFi.h>
#include <solduino.h>
#include <ArduinoJson.h>

// ============================================================================
// Configuration -- EDIT THESE
// ============================================================================

const char* ssid     = "YOUR_WIFI_SSID";
const char* password = "YOUR_WIFI_PASSWORD";

// RPC endpoint (localnet or devnet)
const String RPC_ENDPOINT = SOLDUINO_DEVNET_RPC;

// Your deployed Anchor program's Base58 address
const char* PROGRAM_ID_BASE58 = "YourProgramId1111111111111111111111111111111";

// Authority private key (Base58). Fund this wallet before running.
// WARNING: Never commit real private keys to source control!
const char* AUTHORITY_PRIVATE_KEY_BASE58 = "YourBase58PrivateKeyHere";

// Sensor configuration
const int SENSOR_PIN = 34;               // Analog input pin
const uint32_t READING_INTERVAL_MS = 10000; // Push every 10 seconds

// Transaction settings
const uint32_t POLL_INTERVAL_MS   = 1000;
const uint32_t CONFIRM_TIMEOUT_MS = 30000;

// Anchor discriminator for "record_data" instruction
// = first 8 bytes of SHA-256("global:record_data")
// Replace with your actual discriminator from the IDL.
const uint8_t RECORD_DATA_DISCRIMINATOR[8] = {
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00  // placeholder
};

// ============================================================================
// Global Objects
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
// Sensor Reading
// ============================================================================

/**
 * Read a sensor value and convert to a meaningful integer.
 *
 * For a TMP36 temperature sensor on 3.3 V:
 *   voltage = analogRead * (3.3 / 4095)
 *   tempC   = (voltage - 0.5) * 100
 *
 * We store as fixed-point: value = tempC * 100  (e.g., 2350 = 23.50 C)
 *
 * Replace this with your own sensor logic.
 */
int64_t readSensorValue() {
    int raw = analogRead(SENSOR_PIN);

    // TMP36 conversion (3.3V reference, 12-bit ADC)
    float voltage = raw * (3.3f / 4095.0f);
    float tempC = (voltage - 0.5f) * 100.0f;

    // Fixed-point: multiply by 100 for two decimal places
    int64_t value = (int64_t)(tempC * 100.0f);

    Serial.print("  Raw ADC: ");
    Serial.print(raw);
    Serial.print("  Voltage: ");
    Serial.print(voltage, 3);
    Serial.print("V  Temp: ");
    Serial.print(tempC, 2);
    Serial.print("C  Encoded: ");
    Serial.println(value);

    return value;
}

// ============================================================================
// Build & Send Transaction
// ============================================================================

bool pushSensorData(int64_t sensorValue) {
    Serial.println("\n  Building transaction...");

    // Current timestamp (seconds since boot -- replace with NTP for real time)
    int64_t timestamp = (int64_t)(millis() / 1000);

    // Build the instruction
    Instruction ix;
    ix.setProgram(programId);
    ix.addKey(authorityPub, true, true);                 // authority (signer, writable)
    ix.addKey(dataAccountPda, false, true);               // data PDA (writable)
    ix.addKey(SystemProgram::PROGRAM_ID, false, false);   // system program

    // Instruction data: discriminator + i64 value + i64 timestamp
    ix.writeBytes(RECORD_DATA_DISCRIMINATOR, 8);
    ix.writeI64LE(sensorValue);
    ix.writeI64LE(timestamp);

    Serial.print("  Instruction data: ");
    Serial.print(ix.getDataLength());
    Serial.println(" bytes");

    // Build transaction
    Transaction tx;
    tx.add(ix);

    // Get blockhash
    uint8_t blockhash[BLOCKHASH_SIZE];
    if (!rpcClient.getLatestBlockhashBytes(blockhash)) {
        Serial.println("  [ERROR] Failed to get blockhash");
        return false;
    }
    tx.setRecentBlockhash(blockhash);

    // Sign
    if (!tx.sign(authorityKeypair)) {
        Serial.println("  [ERROR] Failed to sign");
        return false;
    }

    // Encode
    if (!TransactionSerializer::encodeTransactionBase58(tx, g_txBuf, sizeof(g_txBuf))) {
        Serial.println("  [ERROR] Failed to encode");
        return false;
    }

    // Send
    Serial.println("  Sending transaction...");
    String sig = rpcClient.sendTransactionBase58(g_txBuf);
    if (sig.length() == 0) {
        Serial.println("  [ERROR] Send failed");
        return false;
    }
    Serial.print("  Sent! Sig: ");
    Serial.println(sig);

    // Wait for confirmation
    uint32_t start = millis();
    while (millis() - start < CONFIRM_TIMEOUT_MS) {
        TransactionResponse txResp;
        if (rpcClient.getTransaction(sig, txResp)) {
            Serial.println("  Confirmed on-chain!");
            return true;
        }
        delay(POLL_INTERVAL_MS);
    }
    Serial.println("  Confirmation timed out");
    return true;
}

// ============================================================================
// WiFi
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

// ============================================================================
// Setup
// ============================================================================

void setup() {
    Serial.begin(115200);
    delay(1000);

    Serial.println("\n=== Solduino Sensor-to-Chain Demo ===\n");

    // Initialize SDK
    solduino.begin();

    // Connect WiFi
    Serial.println("Connecting to WiFi...");
    connectToWiFi();
    if (WiFi.status() != WL_CONNECTED) {
        Serial.println("[FATAL] No WiFi -- halting.");
        while (true) delay(1000);
    }

    // Start RPC client
    rpcClient.begin();
    rpcClient.setTimeout(15000);

    // Import authority keypair
    Serial.println("Importing authority keypair...");
    if (!authorityKeypair.importFromPrivateKeyBase58(AUTHORITY_PRIVATE_KEY_BASE58)) {
        Serial.println("[FATAL] Invalid private key -- halting.");
        while (true) delay(1000);
    }
    authorityKeypair.getPublicKey(authorityPub);

    char addr[64];
    authorityKeypair.getPublicKeyAddress(addr, sizeof(addr));
    Serial.print("Authority: ");
    Serial.println(addr);

    // Decode program ID
    if (!addressToPublicKey(PROGRAM_ID_BASE58, programId)) {
        Serial.println("[FATAL] Invalid program ID -- halting.");
        while (true) delay(1000);
    }

    // Derive the PDA for this authority's sensor data account
    const uint8_t seed1[] = "sensor";
    const uint8_t* seeds[] = { seed1, authorityPub };
    const size_t seedLens[] = { 6, SOLDUINO_PUBKEY_SIZE };

    if (!findProgramAddress(seeds, seedLens, 2, programId, dataAccountPda, &pdaBump)) {
        Serial.println("[FATAL] PDA derivation failed -- halting.");
        while (true) delay(1000);
    }

    char pdaAddr[64];
    publicKeyToAddress(dataAccountPda, pdaAddr, sizeof(pdaAddr));
    Serial.print("Data PDA:  ");
    Serial.println(pdaAddr);
    Serial.print("Bump:      ");
    Serial.println(pdaBump);

    // Configure ADC
    analogReadResolution(12);
    pinMode(SENSOR_PIN, INPUT);

    Serial.println("\nSetup complete. Starting sensor loop...\n");
}

// ============================================================================
// Main Loop -- read sensor & push on-chain at intervals
// ============================================================================

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

    int64_t value = readSensorValue();

    if (pushSensorData(value)) {
        Serial.println("  Data stored on-chain successfully!");
    } else {
        Serial.println("  Failed to store data on-chain.");
    }

    Serial.println();
}

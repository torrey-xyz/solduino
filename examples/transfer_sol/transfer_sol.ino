#include <solduino.h>
#include <WiFi.h>
#include <SPIFFS.h>

// WiFi Configuration
const char* WIFI_SSID = "YOUR_WIFI_SSID";
const char* WIFI_PASSWORD = "YOUR_WIFI_PASSWORD";

// Solana Configuration
const char* SOLANA_ENDPOINT = "https://api.devnet.solana.com";
const char* RECIPIENT_ADDRESS = "RECIPIENT_PUBKEY_HERE";  // Base58 encoded public key

// Global variables
RpcClient rpc;
Account sender_account;
ConnectionStatus connection_status;

void setup() {
    Serial.begin(115200);
    while (!Serial) { delay(100); }
    
    // Initialize SPIFFS for secure storage
    if (!SPIFFS.begin(true)) {
        Serial.println("Failed to mount SPIFFS");
        return;
    }

    // Initialize WiFi
    WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
    Serial.print("Connecting to WiFi");
    while (WiFi.status() != WL_CONNECTED) {
        delay(500);
        Serial.print(".");
    }
    Serial.println("\nWiFi Connected!");

    // Initialize Solduino
    if (!solduino_init()) {
        Serial.println("Failed to initialize Solduino");
        return;
    }

    // Initialize connection with commitment level
    if (!connection_init(&rpc, SOLANA_ENDPOINT, COMMITMENT_CONFIRMED)) {
        Serial.println("Failed to initialize Solana connection");
        return;
    }

    // Check connection health
    if (!connection_get_status(&rpc, &connection_status)) {
        Serial.println("Failed to get connection status");
        return;
    }

    if (!connection_status.is_connected) {
        Serial.println("Not connected to Solana network");
        return;
    }

    Serial.printf("Connected to Solana %s (slot %d)\n", 
                 connection_status.version,
                 connection_status.slot);

    // Load or generate sender account
    if (!load_or_create_account()) {
        Serial.println("Failed to setup sender account");
        return;
    }

    // Get and display account balance
    uint64_t balance;
    if (account_get_balance(&sender_account, &rpc, &balance)) {
        Serial.printf("Account balance: %f SOL\n", 
                     SOLDUINO_LAMPORTS_TO_SOL(balance));
    }
}

void loop() {
    // Send 0.001 SOL every 30 seconds
    static unsigned long lastTransfer = 0;
    const unsigned long TRANSFER_INTERVAL = 30000; // 30 seconds

    if (millis() - lastTransfer >= TRANSFER_INTERVAL) {
        lastTransfer = millis();
        send_sol(RECIPIENT_ADDRESS, SOLDUINO_SOL_TO_LAMPORTS(0.001));
    }
}

bool load_or_create_account() {
    // Try to load keypair from SPIFFS
    File keyFile = SPIFFS.open("/keypair.dat", "r");
    uint8_t seed[SOLDUINO_SEED_LENGTH];
    
    if (!keyFile || keyFile.size() != SOLDUINO_SEED_LENGTH) {
        // Generate new keypair if none exists
        for (int i = 0; i < SOLDUINO_SEED_LENGTH; i++) {
            seed[i] = random(256);
        }
        
        // Save to SPIFFS
        keyFile.close();
        keyFile = SPIFFS.open("/keypair.dat", "w");
        if (!keyFile) {
            Serial.println("Failed to create key file");
            return false;
        }
        keyFile.write(seed, SOLDUINO_SEED_LENGTH);
    } else {
        // Load existing keypair
        keyFile.read(seed, SOLDUINO_SEED_LENGTH);
    }
    keyFile.close();

    // Create account from seed
    if (!account_create(&sender_account, seed, 0, 0, SYSTEM_PROGRAM_ID)) {
        Serial.println("Failed to create account from seed");
        return false;
    }

    // Get and display public key
    char pubkey_b58[SOLDUINO_BASE58_PUBKEY_LENGTH];
    keypair_get_public_key_base58(&sender_account.keypair, pubkey_b58, sizeof(pubkey_b58));
    Serial.printf("Sender public key: %s\n", pubkey_b58);

    return true;
}

void send_sol(const char* recipient, uint64_t lamports) {
    char signature[89];  // Base58 signature length
    Transaction tx;
    
    Serial.printf("Sending %f SOL to %s\n", 
                 SOLDUINO_LAMPORTS_TO_SOL(lamports),
                 recipient);

    // Create and send transaction
    if (!transaction_create_transfer(
            &tx,
            sender_account.keypair.publicKey,
            (uint8_t*)recipient,  // Recipient address should be decoded from base58
            lamports,
            NULL  // Recent blockhash will be fetched automatically
        )) {
        Serial.printf("Failed to create transaction: %s\n", 
                     solduino_get_last_error());
        return;
    }

    // Sign transaction
    uint8_t sig[SOLDUINO_SIGNATURE_LENGTH];
    if (!crypto_sign_message(
            tx.message,
            tx.message_len,
            sender_account.keypair.secretKey,
            sig
        )) {
        Serial.printf("Failed to sign transaction: %s\n", 
                     solduino_get_last_error());
        return;
    }

    // Add signature
    if (!transaction_add_signature(&tx, sig)) {
        Serial.printf("Failed to add signature: %s\n", 
                     solduino_get_last_error());
        return;
    }

    // Send transaction
    if (!rpc_client_send_transaction(
            &rpc,
            (uint8_t*)&tx,
            sizeof(tx),
            signature,
            sizeof(signature)
        )) {
        Serial.printf("Failed to send transaction: %s\n", 
                     solduino_get_last_error());
        return;
    }

    Serial.printf("Transaction sent! Signature: %s\n", signature);
} 
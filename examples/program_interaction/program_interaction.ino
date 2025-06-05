#include <solduino.h>
#include <WiFi.h>
#include <SPIFFS.h>

// WiFi Configuration
const char* WIFI_SSID = "YOUR_WIFI_SSID";
const char* WIFI_PASSWORD = "YOUR_WIFI_PASSWORD";

// Solana Configuration
const char* SOLANA_ENDPOINT = "https://api.devnet.solana.com";
const char* PROGRAM_ID = "YOUR_PROGRAM_ID";  // Base58 encoded

// Global variables
RpcClient rpc;
Account payer_account;
Account data_account;
ConnectionStatus connection_status;

// Custom instruction data structure
typedef struct {
    uint8_t instruction_type;
    uint32_t value;
} CustomInstruction;

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

    // Initialize connection
    if (!connection_init(&rpc, SOLANA_ENDPOINT, COMMITMENT_CONFIRMED)) {
        Serial.println("Failed to initialize Solana connection");
        return;
    }

    // Check connection health
    if (!connection_get_status(&rpc, &connection_status)) {
        Serial.println("Failed to get connection status");
        return;
    }

    // Load or create payer account
    if (!load_or_create_account(&payer_account, "/payer_keypair.dat")) {
        Serial.println("Failed to setup payer account");
        return;
    }

    // Create program data account
    if (!create_program_account()) {
        Serial.println("Failed to create program account");
        return;
    }
}

void loop() {
    // Interact with program every 10 seconds
    static unsigned long lastInteraction = 0;
    const unsigned long INTERACTION_INTERVAL = 10000; // 10 seconds

    if (millis() - lastInteraction >= INTERACTION_INTERVAL) {
        lastInteraction = millis();
        
        // Create a custom instruction
        CustomInstruction instruction = {
            .instruction_type = 1,  // Example instruction type
            .value = random(100)    // Random value to store
        };
        
        // Send instruction to program
        interact_with_program(&instruction);
    }
}

bool load_or_create_account(Account* account, const char* keypair_path) {
    File keyFile = SPIFFS.open(keypair_path, "r");
    uint8_t seed[SOLDUINO_SEED_LENGTH];
    
    if (!keyFile || keyFile.size() != SOLDUINO_SEED_LENGTH) {
        for (int i = 0; i < SOLDUINO_SEED_LENGTH; i++) {
            seed[i] = random(256);
        }
        
        keyFile.close();
        keyFile = SPIFFS.open(keypair_path, "w");
        if (!keyFile) {
            Serial.println("Failed to create key file");
            return false;
        }
        keyFile.write(seed, SOLDUINO_SEED_LENGTH);
    } else {
        keyFile.read(seed, SOLDUINO_SEED_LENGTH);
    }
    keyFile.close();

    if (!account_create(account, seed, 0, 0, SYSTEM_PROGRAM_ID)) {
        Serial.println("Failed to create account from seed");
        return false;
    }

    return true;
}

bool create_program_account() {
    // Generate new keypair for program data account
    if (!load_or_create_account(&data_account, "/data_keypair.dat")) {
        return false;
    }

    // Calculate rent-exempt balance for 1024 bytes
    uint64_t lamports = account_get_minimum_balance_for_rent_exemption(&rpc, 1024);
    
    // Create system program instruction to create account
    ProgramInstruction create_ix;
    uint8_t create_data[1024];  // Instruction data for creating account
    
    if (!program_create_instruction(
            &create_ix,
            SYSTEM_PROGRAM_ID,
            create_data,
            sizeof(create_data))) {
        return false;
    }

    // Add accounts
    if (!program_add_account(&create_ix, &payer_account, true, true) ||
        !program_add_account(&create_ix, &data_account, true, true)) {
        return false;
    }

    // Execute instruction
    char signature[89];
    if (!program_execute(
            &create_ix,
            &rpc,
            &payer_account.keypair,
            signature,
            sizeof(signature))) {
        Serial.printf("Failed to create program account: %s\n", 
                     solduino_get_last_error());
        return false;
    }

    Serial.printf("Program account created! Signature: %s\n", signature);
    return true;
}

void interact_with_program(CustomInstruction* instruction) {
    ProgramInstruction ix;
    char signature[89];
    
    // Create program instruction
    if (!program_create_instruction(
            &ix,
            (uint8_t*)PROGRAM_ID,
            (uint8_t*)instruction,
            sizeof(CustomInstruction))) {
        Serial.println("Failed to create instruction");
        return;
    }

    // Add accounts
    if (!program_add_account(&ix, &payer_account, true, false) ||
        !program_add_account(&ix, &data_account, false, true)) {
        Serial.println("Failed to add accounts to instruction");
        return;
    }

    // Execute instruction
    if (!program_execute(&ix, &rpc, &payer_account.keypair, signature, sizeof(signature))) {
        Serial.printf("Failed to execute instruction: %s\n", 
                     solduino_get_last_error());
        return;
    }

    Serial.printf("Instruction executed! Signature: %s\n", signature);
    
    // Get updated account data
    uint8_t account_data[1024];
    size_t data_len;
    if (account_get_info(&data_account, &rpc, account_data, &data_len)) {
        Serial.printf("Updated account data (first 4 bytes): %d\n", 
                     *(uint32_t*)account_data);
    }
} 
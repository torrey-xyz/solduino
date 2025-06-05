// Solduino Example: Transfer SOL
// This example demonstrates how to send SOL from the device to a receiver on DevNet.

#include <solduino.h>

// WiFi credentials
const char* ssid = "YOUR_WIFI_SSID";
const char* password = "YOUR_WIFI_PASSWORD";

// Solana DevNet RPC endpoint
const char* rpc_url = "https://api.devnet.solana.com";

// Receiver's public key (base58 encoded)
const char* receiver_public_key_b58 = "RECEIVER_PUBLIC_KEY_BASE58";

// Amount to send (in lamports)
// 0.001 SOL = 1,000,000 lamports (since 1 SOL = 1,000,000,000 lamports)
// However, for an ESP32, we need to be careful with large integer types.
// Let's represent this as a smaller number for now, and handle conversions later.
unsigned long long amount_lamports = 1000000ULL; // 0.001 SOL

// Device's keypair (32-byte seed or private key)
// IMPORTANT: Never hardcode private keys in production. This is for demonstration only.
// Consider using secure storage mechanisms on your device.
unsigned char device_seed[32] = {
    // Replace with your 32-byte seed or private key
    0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07,
    0x08, 0x09, 0x0A, 0x0B, 0x0C, 0x0D, 0x0E, 0x0F,
    0x10, 0x11, 0x12, 0x13, 0x14, 0x15, 0x16, 0x17,
    0x18, 0x19, 0x1A, 0x1B, 0x1C, 0x1D, 0x1E, 0x1F
};

void setup() {
  Serial.begin(115200);
  while (!Serial) {
    ;
  }
  delay(1000);
  Serial.println("Solduino Transfer SOL Example");

  // Connect to WiFi
  Serial.print("Connecting to WiFi: ");
  Serial.println(ssid);
  // WiFi.begin(ssid, password); // ESP32 specific
  // while (WiFi.status() != WL_CONNECTED) { // ESP32 specific
  //   delay(500);
  //   Serial.print(".");
  // }
  Serial.println("\nWiFi connected");
  // Serial.print("IP address: ");
  // Serial.println(WiFi.localIP()); // ESP32 specific

  Serial.println("Setting up Solduino components...");

  // 1. Initialize Keypair from seed
  // Keypair deviceKeypair;
  // keypair_from_seed(&deviceKeypair, device_seed);
  // char device_public_key_b58[45]; // Max length for base58 pubkey
  // keypair_get_public_key_base58(&deviceKeypair, device_public_key_b58, sizeof(device_public_key_b58));
  // Serial.print("Device Public Key: ");
  // Serial.println(device_public_key_b58);

  // 2. Get a recent blockhash
  // char recent_blockhash_b58[45];
  // RpcClient rpcClient(rpc_url);
  // if (rpcClient.getRecentBlockhash(recent_blockhash_b58, sizeof(recent_blockhash_b58))) {
  //   Serial.print("Recent Blockhash: ");
  //   Serial.println(recent_blockhash_b58);
  // } else {
  //   Serial.println("Failed to get recent blockhash. Halting.");
  //   while(1) delay(1000);
  // }

  // 3. Create a transfer transaction
  // Transaction tx;
  // transaction_create_transfer(
  //     &tx, 
  //     deviceKeypair.publicKey, // Sender
  //     receiver_public_key_b58, // Receiver (need to decode from base58)
  //     amount_lamports,
  //     recent_blockhash_b58 // (need to decode from base58)
  // );
  // Serial.println("Transaction created.");

  // 4. Serialize the transaction message
  // unsigned char serialized_tx[MAX_TRANSACTION_SIZE]; // Define MAX_TRANSACTION_SIZE
  // unsigned int serialized_tx_len;
  // if (transaction_serialize_message(&tx, serialized_tx, sizeof(serialized_tx), &serialized_tx_len)) {
  //   Serial.print("Serialized transaction message (length ");
  //   Serial.print(serialized_tx_len);
  //   Serial.println("):");
  //   for (unsigned int i = 0; i < serialized_tx_len; i++) {
  //     if (serialized_tx[i] < 0x10) Serial.print("0");
  //     Serial.print(serialized_tx[i], HEX);
  //     Serial.print(" ");
  //   }
  //   Serial.println();
  // } else {
  //   Serial.println("Failed to serialize transaction message. Halting.");
  //   while(1) delay(1000);
  // }

  // 5. Sign the serialized transaction message
  // Signature signature_result;
  // crypto_sign_message(serialized_tx, serialized_tx_len, deviceKeypair.secretKey, &signature_result);
  // Serial.println("Transaction signed.");
  // Add signature to the transaction structure
  // transaction_add_signature(&tx, &signature_result);

  // 6. Serialize the full transaction (with signature)
  // unsigned char fully_serialized_tx[MAX_TRANSACTION_SIZE_WITH_SIG]; // Define appropriately
  // unsigned int fully_serialized_tx_len;
  // if (transaction_serialize(&tx, fully_serialized_tx, sizeof(fully_serialized_tx), &fully_serialized_tx_len)) {
  //  Serial.print("Fully serialized transaction for broadcast (length ");
  //  Serial.print(fully_serialized_tx_len);
  //  Serial.println(")");
  // } else {
  //  Serial.println("Failed to serialize full transaction. Halting.");
  //  while(1) delay(1000);
  // }
  
  // 7. Send the transaction
  // char signature_b58[89]; // Max length for base58 signature
  // if (rpcClient.sendTransaction(fully_serialized_tx, fully_serialized_tx_len, signature_b58, sizeof(signature_b58))) {
  //   Serial.print("Transaction sent! Signature: ");
  //   Serial.println(signature_b58);
  // } else {
  //   Serial.println("Failed to send transaction.");
  // }

  Serial.println("\nDemo finished. Implement the SDK functions to make this work!");
}

void loop() {
  // Nothing to do here for this example
  delay(10000);
} 
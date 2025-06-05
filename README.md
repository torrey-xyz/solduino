# Solduino

A minimal Solana SDK for ESP32 and Arduino platforms, optimized for memory-constrained devices.

## Features

- 📡 Native Solana transaction support for IoT devices
- 🔑 Ed25519 keypair generation and management
- 📦 Borsh serialization for Solana transactions
- 🌐 HTTP RPC client for Solana DevNet
- 💾 Memory-optimized for embedded systems (~320KB RAM)
- 🔒 Security-focused implementation

## Requirements

- Arduino IDE 2.0.0 or higher
- ESP32 board support package
- ArduinoJson library (for JSON streaming)
- WiFiClientSecure (included with ESP32)

## Installation

### Using Arduino IDE Library Manager

1. Open Arduino IDE
2. Go to `Sketch > Include Library > Manage Libraries`
3. Search for "Solduino"
4. Click Install

### Manual Installation

1. Download this repository
2. Extract to your Arduino libraries folder:
   - Windows: `Documents\Arduino\libraries\`
   - Mac: `~/Documents/Arduino/libraries/`
   - Linux: `~/Arduino/libraries/`
3. Restart Arduino IDE

## Usage

### Basic Example: Sending SOL

```cpp
#include <solduino.h>

// Initialize keypair from seed
uint8_t seed[32] = { /* your seed */ };
Keypair keypair;
keypair_from_seed(&keypair, seed);

// Initialize RPC client
RpcClient rpc;
rpc_client_init(&rpc, "https://api.devnet.solana.com");

// Create and send transaction
void sendSol(const char* recipient, uint64_t amount_lamports) {
    Transaction tx;
    uint8_t recent_blockhash[32];
    
    // Get recent blockhash
    rpc_client_get_recent_blockhash(&rpc, recent_blockhash, sizeof(recent_blockhash));
    
    // Create transfer transaction
    transaction_create_transfer(&tx, keypair.publicKey, recipient, amount_lamports, recent_blockhash);
    
    // Sign and send
    uint8_t signature[64];
    crypto_sign_message(tx.message, tx.message_len, keypair.secretKey, signature);
    transaction_add_signature(&tx, signature);
    
    // Serialize and send
    uint8_t serialized_tx[1024];
    size_t serialized_size;
    transaction_serialize(&tx, serialized_tx, sizeof(serialized_tx), &serialized_size);
    
    char signature_str[89];
    rpc_client_send_transaction(&rpc, serialized_tx, serialized_size, signature_str, sizeof(signature_str));
}
```

### Memory Management

The SDK uses static buffers to avoid heap fragmentation:

```cpp
#define SOLDUINO_MAX_SERIALIZED_SIZE 1024
#define SOLDUINO_MAX_URL_LENGTH 128
#define SOLDUINO_MAX_RESPONSE_SIZE 2048
```

Adjust these values in the headers based on your device's memory constraints.

## Security Considerations

1. Never hardcode private keys in your code
2. Use secure storage (SPIFFS/EEPROM) for sensitive data
3. Consider using a hardware secure element if available
4. Keep your device's firmware updated
5. Use TLS for RPC connections

## Examples

Check the `examples/` directory for more sample code:

- `transfer_sol.ino`: Basic SOL transfer
- `create_account.ino`: Create a new account
- `token_transfer.ino`: SPL token transfer

## Contributing

1. Fork the repository
2. Create a feature branch
3. Commit your changes
4. Push to the branch
5. Create a Pull Request

## License

This project is licensed under the MIT License - see the LICENSE file for details.

## Acknowledgments

- Solana Labs for the protocol specification
- TweetNaCl.js for the Ed25519 implementation
- Arduino community for embedded development insights

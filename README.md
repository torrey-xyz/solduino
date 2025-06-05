# Solduino

A minimal, memory-efficient Solana SDK for ESP32 and Arduino platforms. Solduino enables you to interact with the Solana blockchain directly from your embedded devices.

## Features

- **Optimized for Embedded Systems**
  - Static buffer usage to avoid heap fragmentation
  - Minimal RAM footprint (~32KB required)
  - Hardware-accelerated cryptography on ESP32

- **Core Functionality**
  - Ed25519 key generation and management
  - Transaction signing and verification
  - Secure key storage using SPIFFS
  - SPL Token support
  - Custom program interaction

- **Platform-Specific Optimizations**
  - ESP32: Hardware-accelerated SHA-512 via mbedtls
  - ESP32: Hardware random number generator
  - Arduino: Optimized software implementations
  - Arduino: Enhanced random number generation

## Requirements

### ESP32
- ESP32 board support package
- ESP-IDF (for mbedtls)
- At least 32KB RAM
- WiFi connection for network operations

### Arduino
- Arduino board with at least 32KB RAM
- SHA512 library (install via Library Manager)
- Sufficient flash storage for program

## Installation

1. Clone this repository:
```bash
git clone https://github.com/yourusername/solduino.git
```

2. Copy the `src` folder to your Arduino libraries directory:
```bash
cp -r src ~/Arduino/libraries/solduino
```

3. Install required dependencies:
   - For ESP32: No additional libraries needed
   - For Arduino: Install SHA512 library from Library Manager

## Usage

### Basic Example

```cpp
#include <solduino.h>

void setup() {
    unsigned char public_key[32];
    unsigned char private_key[64];
    
    // Generate a new keypair
    crypto_sign_keypair(public_key, private_key);
    
    // Sign a message
    const char* message = "Hello Solana!";
    unsigned char signature[64];
    unsigned long long siglen;
    
    crypto_sign_detached(
        signature,
        &siglen,
        (unsigned char*)message,
        strlen(message),
        private_key
    );
}
```

### Advanced Examples

Check the `examples` directory for:
- `ed25519_sign.ino`: Basic signing example for Arduino
- `esp32_solana_sign.ino`: ESP32-specific example with hardware acceleration
- More examples coming soon!

## Implementation Details

### Cryptographic Operations

The library uses an optimized version of TweetNaCl for Ed25519 operations:
- Hardware-accelerated SHA-512 on ESP32
- Optimized field arithmetic for 32-bit processors
- Memory-efficient implementation

### Key Storage

- ESP32: Uses SPIFFS for secure key storage
- Arduino: Configurable storage options (EEPROM/SD)
- Key derivation from seed phrases (coming soon)

### Memory Management

- Static buffers for cryptographic operations
- No dynamic memory allocation
- Optimized for embedded constraints
- Careful stack usage

## Security Considerations

1. **Key Management**
   - Never hardcode private keys
   - Use secure storage (SPIFFS/EEPROM)
   - Consider hardware security modules

2. **Random Number Generation**
   - ESP32: Hardware RNG used
   - Arduino: Enhanced seeding mechanism
   - Avoid predictable seeds

3. **Network Security**
   - Use secure connections (HTTPS/WSS)
   - Validate all RPC responses
   - Handle network errors gracefully

## Contributing

Contributions are welcome! Please read our contributing guidelines before submitting pull requests.

## License

MIT License - see LICENSE file for details
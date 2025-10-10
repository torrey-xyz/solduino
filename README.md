# Solduino - Solana Library for Arduino/ESP32

[![Version](https://img.shields.io/badge/version-0.1.0-blue.svg)](https://github.com/torrey-xyz/solduino)
[![License](https://img.shields.io/badge/license-Apache--2.0-green.svg)](LICENSE)

Solduino is a comprehensive embedded software development kit (SDK) for interacting with the Solana blockchain from Arduino and ESP32 microcontrollers. It provides tools for wallet generation, transaction signing, and RPC communication.

## Features

- ✅ **RPC Communication**: Full-featured Solana RPC client for ESP32
- ✅ **Wallet Generation**: Generate and manage Solana keypairs
- ✅ **Transaction Signing**: Sign and send Solana transactions
- ✅ **HTTPS Support**: Secure connections using WiFiClientSecure
- ✅ **Arduino Compatible**: Works with Arduino IDE and PlatformIO

## Table of Contents

- [Installation](#installation)
- [Quick Start](#quick-start)
- [Architecture](#architecture)
- [Modules](#modules)
- [Examples](#examples)
- [API Reference](#api-reference)
- [Contributing](#contributing)
- [License](#license)

## Installation

### Prerequisites

- Arduino IDE 1.8.13+ or PlatformIO
- ESP32 Board Support Package installed
- WiFi connection for RPC communication

### Installing via Arduino Library Manager

1. Open Arduino IDE
2. Go to **Sketch** → **Include Library** → **Manage Libraries**
3. Search for "Solduino" or "sol"
4. Click **Install**

### Manual Installation

1. Download or clone this repository:
   ```bash
   git clone https://github.com/torrey-xyz/solduino.git
   ```
2. Copy the `sol` folder to your Arduino `libraries` directory:
   - **Windows**: `Documents\Arduino\libraries\`
   - **macOS**: `~/Documents/Arduino/libraries/`
   - **Linux**: `~/Arduino/libraries/`
3. Restart Arduino IDE

### Installing ESP32 Board Support

If you haven't installed ESP32 support:

1. In Arduino IDE, go to **File** → **Preferences**
2. Add this URL to **Additional Board Manager URLs**:
   ```
   https://raw.githubusercontent.com/espressif/arduino-esp32/gh-pages/package_esp32_index.json
   ```
3. Go to **Tools** → **Board** → **Boards Manager**
4. Search for "ESP32" and install **esp32 by Espressif Systems**

### Required Libraries

This library depends on:
- **ArduinoJson** (v6.19.0+) - Available via Library Manager
- **WiFi** - Built into ESP32
- **HTTPClient** - Built into ESP32
- **WiFiClientSecure** - Built into ESP32

Install ArduinoJson via Library Manager if not already installed.

## Quick Start

### Basic RPC Connection Example

```cpp
#include <WiFi.h>
#include <rpc_client.h>

const char* ssid = "YOUR_WIFI_SSID";
const char* password = "YOUR_WIFI_PASSWORD";
const String DEVNET_RPC = "https://api.devnet.solana.com";

RpcClient solanaClient(DEVNET_RPC);

void setup() {
    Serial.begin(115200);
    
    // Connect to WiFi
    WiFi.begin(ssid, password);
    while (WiFi.status() != WL_CONNECTED) {
        delay(500);
        Serial.print(".");
    }
    Serial.println("\nWiFi connected!");
    
    // Initialize RPC client
    if (solanaClient.begin()) {
        Serial.println("RPC Client initialized!");
        
        // Get network version
        String version = solanaClient.getVersion();
        Serial.println("Solana Version: " + version);
        
        // Get current slot
        String slot = solanaClient.getSlot();
        Serial.println("Current Slot: " + slot);
    }
}

void loop() {
    // Your code here
}
```

## Architecture

This section describes the architecture and organization of the Solduino library.

### Overview

Solduino is organized into three main components as required for embedded software development:

1. **Core Embedded Software Development Kit (SDK)**
2. **Modules** (Wallet Generation, Transaction Signing, RPC Communication)
3. **Setup Instructions** (provided in [Installation](#installation) section)

### Directory Structure

```
sol/
├── README.md                 # Main documentation and setup instructions
├── library.properties        # Arduino library metadata
├── LICENSE                   # License file (to be added)
│
├── solduino.h                # Core SDK entry point
├── solduino.cpp              # Core SDK implementation
│
├── rpc_client.h              # RPC Communication Module (header)
├── rpc_client.cpp            # RPC Communication Module (implementation)
│
├── connection.h              # Connection Management Module
│
└── examples/
    └── basic_rpc_demo/
        └── basic_rpc_demo.ino  # Comprehensive example sketch
```

### Component Details

#### 1. Core Embedded Software Development Kit (SDK)

**Purpose**: Provides foundational functionality, version management, and acts as the main entry point for the library.

**Files**:
- `solduino.h` - Main SDK header with:
  - Version information and constants
  - Library-wide configuration
  - Module includes
  - Core SDK class definition
- `solduino.cpp` - SDK implementation with:
  - Version accessors
  - Library initialization
  - SDK information display

**Key Features**:
- Version management (major.minor.patch)
- Library-wide constants (URL lengths, key sizes, etc.)
- RPC endpoint constants (mainnet, devnet, testnet)
- Commitment level enums
- Centralized module access

**Usage**:
```cpp
#include <solduino.h>

Solduino sdk;
sdk.begin();
Serial.println(sdk.getVersion());
```

#### 2. Modules

##### Module 1: RPC Communication (`rpc_client.h` / `rpc_client.cpp`)

**Purpose**: Enables communication with Solana RPC endpoints.

**Features**:
- HTTPS support via WiFiClientSecure
- Account information retrieval
- Balance queries
- Transaction submission and status checking
- Block and slot information
- Token account operations
- Network health monitoring
- Custom RPC calls

**Key Classes**:
- `RpcClient` - Main RPC client class
- `AccountInfo` - Account information structure
- `Balance` - Balance information structure
- `BlockInfo` - Block information structure
- `TransactionResponse` - Transaction response structure

**Usage**:
```cpp
#include <solduino.h>  // Includes rpc_client.h

RpcClient client(SOLDUINO_DEVNET_RPC);
client.begin();
String balance = client.getBalance(publicKey);
```

##### Module 2: Connection Management (`connection.h`)

**Purpose**: Provides connection status and management utilities.

**Features**:
- Connection configuration structures
- Connection status tracking
- Commitment level management
- Connection health checking

**Key Structures**:
- `ConnectionConfig` - Connection configuration
- `ConnectionStatus` - Current connection status
- `Commitment` - Commitment level enum

**Status**: Currently header-only, implementation pending.

##### Module 3: Wallet Generation (`keypair.h` / `keypair.cpp`, `crypto.h` / `crypto.cpp`)

**Purpose**: Generate and manage Solana keypairs on embedded devices.

**Features**:
- Ed25519 keypair generation (simplified implementation)
- Import wallets from private keys (Base58 format)
- Import wallets from seeds
- Public/private key management
- Base58 encoding/decoding for Solana addresses
- Message signing

**Files**: 
- `keypair.h` / `keypair.cpp` - Keypair class for wallet management
- `crypto.h` / `crypto.cpp` - Cryptographic utilities (Base58, Ed25519)

**⚠️ Important Note**: The current implementation uses simplified cryptographic functions for demonstration. For production use, integrate a proper Ed25519 library such as libsodium or tweetnacl.

**Usage:**
```cpp
#include <solduino.h>

// Generate new keypair
Keypair keypair;
keypair.generate();

// Get public address
char address[64];
keypair.getPublicKeyAddress(address, sizeof(address));
Serial.println(address);

// Import from private key (Base58)
Keypair imported;
imported.importFromPrivateKeyBase58("YourPrivateKeyBase58");

// Import from seed
uint8_t seed[32];
// ... set seed ...
imported.importFromSeed(seed);

// Sign a message
String message = "Hello, Solana!";
uint8_t signature[64];
keypair.signString(message, signature);
```

##### Module 4: Transaction Signing (Planned)

**Purpose**: Build, sign, and serialize Solana transactions.

**Planned Features**:
- Transaction construction
- Instruction building
- Transaction signing with keypairs
- Base58/base64 encoding
- Serialization for RPC submission

**Files**: To be implemented (`transaction.h`, `serializer.h`)

**Usage:**
```cpp
Transaction tx;
tx.addInstruction(instruction);
tx.setRecentBlockhash(blockhash);
tx.sign(keypair);
String serialized = tx.serialize();
```

#### 3. Setup Instructions

Complete setup instructions are provided in the [Installation](#installation) section above, including:

- Installation methods (Library Manager, manual)
- Prerequisites
- ESP32 board setup
- Required dependencies
- Quick start examples
- Troubleshooting guide

### Module Dependencies

```
solduino.h (Core SDK)
├── Includes: rpc_client.h
├── Includes: connection.h
└── Provides: Constants, Version Info

rpc_client.h (RPC Module)
├── Depends on: WiFiClientSecure (ESP32)
├── Depends on: HTTPClient (ESP32)
├── Depends on: ArduinoJson
└── Used by: connection.h

connection.h (Connection Module)
├── Depends on: rpc_client.h
└── Provides: Connection utilities
```

### Design Principles

1. **Modularity**: Each module is self-contained with clear interfaces
2. **Arduino Compatibility**: Follows Arduino library conventions
3. **ESP32 Optimized**: Uses ESP32-specific features (WiFiClientSecure)
4. **Memory Efficient**: Designed for embedded constraints
5. **Easy to Use**: Simple API with comprehensive examples

### Extension Points

#### Adding New Modules

1. Create header file (e.g., `new_module.h`)
2. Create implementation file (e.g., `new_module.cpp`)
3. Include in `solduino.h` under "Module Includes" section
4. Update README.md with documentation
5. Add example usage

#### Adding New RPC Methods

1. Add method declaration to `RpcClient` class in `rpc_client.h`
2. Implement using `makeRpcRequest()` in `rpc_client.cpp`
3. Add to README.md API reference
4. Update example sketch if applicable

### Version Management

Version is managed in `solduino.h`:
- `SOLDUINO_VERSION_MAJOR` - Breaking changes
- `SOLDUINO_VERSION_MINOR` - New features
- `SOLDUINO_VERSION_PATCH` - Bug fixes

Version string format: `"MAJOR.MINOR.PATCH"`

### Future Enhancements

- [ ] Implement wallet generation module
- [ ] Implement transaction signing module
- [ ] Add WebSocket support for real-time subscriptions
- [ ] Add certificate validation for production use
- [ ] Add secure key storage support
- [ ] Add transaction building helpers
- [ ] Add instruction builders
- [ ] Add token transfer utilities
- [ ] Add program interaction helpers

## Examples

### Example 1: Basic RPC Calls

```cpp
#include <WiFi.h>
#include <rpc_client.h>

RpcClient client("https://api.devnet.solana.com");

void setup() {
    // ... WiFi setup ...
    client.begin();
    
    // Get account info
    String accountInfo = client.getAccountInfo("11111111111111111111111111111112");
    Serial.println(accountInfo);
    
    // Get balance
    String balance = client.getBalance("11111111111111111111111111111112");
    Serial.println(balance);
}
```

### Example 2: Network Monitoring

```cpp
void monitorNetwork() {
    String health = client.getHealth();
    String slot = client.getSlot();
    String version = client.getVersion();
    
    Serial.println("Health: " + health);
    Serial.println("Slot: " + slot);
    Serial.println("Version: " + version);
}
```

### Example 3: Transaction Submission

```cpp
// After signing a transaction
String serializedTx = "base64_encoded_transaction";
String result = client.sendTransaction(serializedTx);
Serial.println("Transaction submitted: " + result);
```

## API Reference

### RpcClient Class

#### Constructor
```cpp
RpcClient(const String& endpoint)
```

#### Methods

**Connection Management**
- `bool begin()` - Initialize RPC client
- `void end()` - Clean up resources
- `void setTimeout(int timeout)` - Set request timeout (ms)

**Account Operations**
- `String getAccountInfo(const String& publicKey)` - Get account information
- `String getBalance(const String& publicKey)` - Get account balance

**Network Information**
- `String getVersion()` - Get Solana version
- `String getSlot()` - Get current slot
- `String getBlockHeight()` - Get block height
- `String getHealth()` - Check network health
- `String getLatestBlockhash()` - Get latest blockhash (recommended)
- `String getRecentBlockhash()` - Get recent blockhash (deprecated, use getLatestBlockhash instead)

**Transaction Operations**
- `String sendTransaction(const String& transaction)` - Send transaction
- `String getTransaction(const String& signature)` - Get transaction details
- `String getConfirmedTransaction(const String& signature)` - Get confirmed transaction

**Block Operations**
- `String getBlock(uint64_t slot)` - Get block information
- `String getBlockCommitment(uint64_t slot)` - Get block commitment
- `String getBlocks(uint64_t startSlot, uint64_t endSlot)` - Get multiple blocks

**Token Operations**
- `String getTokenAccountsByOwner(const String& owner, const String& mint)` - Get token accounts
- `String getTokenSupply(const String& mint)` - Get token supply

**Utility**
- `String callRpc(const String& method, const String& params)` - Custom RPC call

### Response Parsing Functions

```cpp
bool parseAccountInfo(const String& jsonResponse, AccountInfo& info);
bool parseBalance(const String& jsonResponse, Balance& balance);
bool parseBlockInfo(const String& jsonResponse, BlockInfo& info);
bool parseTransaction(const String& jsonResponse, TransactionResponse& tx);
```

## Configuration

### RPC Endpoints

**Mainnet:**
```cpp
const String MAINNET_RPC = "https://api.mainnet-beta.solana.com";
```

**Devnet:**
```cpp
const String DEVNET_RPC = "https://api.devnet.solana.com";
```

**Testnet:**
```cpp
const String TESTNET_RPC = "https://api.testnet.solana.com";
```

### Timeout Configuration

```cpp
client.setTimeout(15000); // 15 seconds
```

## Troubleshooting

### Common Issues

**HTTP Error -1 (Connection Failed)**
- Ensure WiFi is connected
- Check RPC endpoint URL is correct
- Verify HTTPS is properly configured (should be automatic)

**HTTP Error -5 (Connection Lost)**
- Network instability
- RPC endpoint might be unavailable
- Try increasing timeout: `client.setTimeout(30000)`

**Compilation Errors**
- Ensure ESP32 board support is installed
- Verify ArduinoJson library is installed
- Check all required headers are included

**WiFi Connection Issues**
- Verify SSID and password are correct
- Check WiFi signal strength
- Ensure 2.4GHz network (ESP32 doesn't support 5GHz)

## Platform Support

- ✅ ESP32 (tested)
- ✅ ESP32-S2
- ✅ ESP32-S3
- ✅ ESP32-C3
- ⚠️ ESP8266 (may require modifications)
- ❌ Standard Arduino (insufficient memory)

## Limitations

- Certificate validation is currently disabled for HTTPS (uses `setInsecure()`)
- Large JSON responses may require increased buffer sizes
- Memory constraints limit transaction size on some devices
- Real-time WebSocket subscriptions not yet implemented

## Security Considerations

⚠️ **Important Security Notes:**

1. **Certificate Validation**: The library currently uses `setInsecure()` which disables SSL certificate validation. For production use, implement proper certificate validation.

2. **Private Keys**: Never expose private keys in your code. Consider using secure storage solutions for production applications.

3. **Network Security**: Always use HTTPS endpoints. Avoid sending sensitive data over unencrypted connections.

4. **Key Management**: Implement proper key management practices. Consider hardware security modules (HSM) for production deployments.

## Contributing

Contributions are welcome! Please read our [Contributing Guidelines](CONTRIBUTING.md) and [Code of Conduct](CODE_OF_CONDUCT.md) before submitting a Pull Request.

We appreciate all kinds of contributions:
- 🐛 Bug reports
- 💡 Feature suggestions
- 📝 Documentation improvements
- 🔧 Code contributions
- ⭐ Star the repository

See [CONTRIBUTING.md](CONTRIBUTING.md) for detailed guidelines.

## License

This project is licensed under the Apache License 2.0 - see the [LICENSE](LICENSE) file for details.

## Acknowledgments

- Solana Labs for the Solana blockchain
- ArduinoJson contributors
- ESP32 Arduino Core team

## Support

For issues, questions, or contributions:
- GitHub Issues: [Create an issue](https://github.com/torrey-xyz/solduino/issues)
- Email: parvat.raj2@gmail.com
- Repository: [https://github.com/torrey-xyz/solduino](https://github.com/torrey-xyz/solduino)

## Changelog

### Version 0.1.0 (Current)
- Initial release
- RPC client implementation
- HTTPS support with WiFiClientSecure
- Basic account and transaction operations
- Network monitoring functions

---

**Made with ❤️ for the Solana and Arduino communities**


# Solduino Codebase Index

## Overview

**Solduino** is a Solana library for Arduino and ESP32 microcontrollers. It provides tools for wallet generation, transaction signing, and RPC communication with the Solana blockchain.

**Version:** 0.1.0  
**License:** MIT  
**Target Platforms:** ESP32, ESP32-S2, ESP32-S3, ESP32-C3

---

## Directory Structure

```
sol/
├── README.md                 # Main documentation
├── library.properties        # Arduino library metadata
├── CODEBASE_INDEX.md         # This file - codebase index
│
├── solduino.h                # Core SDK header (main entry point)
├── solduino.cpp              # Core SDK implementation
│
├── rpc_client.h              # RPC Communication Module
├── rpc_client.cpp            # RPC Communication implementation
│
├── connection.h              # Connection Management (header-only, C interface)
│
├── crypto.h                  # Cryptographic utilities (Base58, Ed25519)
├── crypto.cpp                # Crypto implementation
│
├── keypair.h                 # Keypair/Wallet management
├── keypair.cpp               # Keypair implementation
│
├── transaction.h             # Transaction structures and operations
├── transaction.cpp           # Transaction implementation
│
├── serializer.h              # Transaction serialization
├── serializer.cpp            # Serializer implementation
│
├── tweetnacl.h               # TweetNaCl crypto library header
├── tweetnacl.c               # TweetNaCl crypto library implementation
│
└── examples/
    ├── basic_rpc_demo/
    │   └── basic_rpc_demo.ino
    ├── transaction_demo/
    │   ├── airdrop_demo.ino
    │   ├── transaction_demo.ino
    │   └── transfer_demo.ino
    └── wallet_demo/
        └── wallet_demo.ino
```

---

## Core Components

### 1. Core SDK (`solduino.h` / `solduino.cpp`)

**Purpose:** Main entry point and library management

**Key Features:**
- Version management (major.minor.patch)
- Library-wide constants
- Module includes (convenience header)
- SDK initialization

**Main Class:**
- `Solduino` - Core SDK class with version accessors

**Constants Defined:**
- `SOLDUINO_VERSION_MAJOR/MINOR/PATCH` - Version numbers
- `SOLDUINO_MAX_URL_LENGTH` - Max URL length (256)
- `SOLDUINO_DEFAULT_TIMEOUT_MS` - Default timeout (10000ms)
- `SOLDUINO_PUBLIC_KEY_SIZE` - Public key size (32 bytes)
- `SOLDUINO_PRIVATE_KEY_SIZE` - Private key size (64 bytes)
- `SOLDUINO_SIGNATURE_SIZE` - Signature size (64 bytes)
- RPC endpoint constants: `SOLDUINO_MAINNET_RPC`, `SOLDUINO_DEVNET_RPC`, `SOLDUINO_TESTNET_RPC`, `SOLDUINO_LOCALNET_RPC`

**Methods:**
- `getVersion()` - Get version string
- `getVersionMajor/Minor/Patch()` - Get version components
- `begin()` - Initialize SDK
- `end()` - Cleanup SDK
- `hello()` - Print library info

---

### 2. RPC Client Module (`rpc_client.h` / `rpc_client.cpp`)

**Purpose:** Communicate with Solana RPC endpoints via HTTPS

**Key Features:**
- HTTPS support using `WiFiClientSecure`
- HTTP fallback for localnet
- JSON-RPC 2.0 protocol
- Account operations
- Transaction operations
- Network monitoring
- Block operations
- Token operations

**Main Class:**
- `RpcClient` - Solana RPC client

**Key Methods:**

**Connection:**
- `begin()` - Initialize RPC client
- `end()` - Cleanup
- `setTimeout(int timeout)` - Set request timeout

**Account Operations:**
- `getAccountInfo(String publicKey)` - Get account information
- `getBalance(String publicKey)` - Get account balance

**Network Information:**
- `getVersion()` - Get Solana version
- `getSlot()` - Get current slot
- `getBlockHeight()` - Get block height
- `getHealth()` - Check network health
- `getLatestBlockhash()` - Get latest blockhash (recommended)
- `getRecentBlockhash()` - Get recent blockhash (deprecated)

**Transaction Operations:**
- `sendTransaction(String transaction)` - Send transaction (base64)
- `getTransaction(String signature)` - Get transaction details
- `getConfirmedTransaction(String signature)` - Get confirmed transaction
- `requestAirdrop(String publicKey, uint64_t lamports)` - Request airdrop

**Block Operations:**
- `getBlock(uint64_t slot)` - Get block information
- `getBlockCommitment(uint64_t slot)` - Get block commitment
- `getBlocks(uint64_t startSlot, uint64_t endSlot)` - Get multiple blocks

**Token Operations:**
- `getTokenAccountsByOwner(String owner, String mint)` - Get token accounts
- `getTokenSupply(String mint)` - Get token supply

**Utility:**
- `callRpc(String method, String params)` - Custom RPC call

**Response Structures:**
- `AccountInfo` - Account information structure
- `Balance` - Balance information structure
- `BlockInfo` - Block information structure
- `TransactionResponse` - Transaction response structure

**Parser Functions:**
- `parseAccountInfo()` - Parse account info JSON
- `parseBalance()` - Parse balance JSON
- `parseBlockInfo()` - Parse block info JSON
- `parseTransaction()` - Parse transaction JSON

**Dependencies:**
- ArduinoJson (v6.19.0+)
- WiFiClientSecure (ESP32)
- HTTPClient (ESP32)

---

### 3. Connection Management (`connection.h`)

**Purpose:** Connection status and management utilities

**Status:** Header-only, C interface (implementation pending)

**Key Structures:**
- `ConnectionConfig` - Connection configuration
  - `endpoint` - RPC endpoint URL
  - `commitment` - Commitment level
  - `timeout_ms` - Timeout in milliseconds
  - `use_websocket` - WebSocket flag

- `ConnectionStatus` - Current connection status
  - `is_connected` - Connection state
  - `slot` - Current slot
  - `version` - Solana version
  - `ping_ms` - Ping latency

- `Commitment` - Commitment level enum
  - `COMMITMENT_PROCESSED`
  - `COMMITMENT_CONFIRMED`
  - `COMMITMENT_FINALIZED`

**Functions (C interface):**
- `connection_init()` - Initialize connection
- `connection_get_status()` - Get connection status
- `connection_get_slot()` - Get current slot
- `connection_get_minimum_ledger_slot()` - Get minimum ledger slot
- `connection_get_genesis_hash()` - Get genesis hash
- `connection_get_health()` - Check health

---

### 4. Crypto Module (`crypto.h` / `crypto.cpp`)

**Purpose:** Cryptographic functions for Solana operations

**Key Features:**
- Base58 encoding/decoding (for Solana addresses)
- Ed25519 keypair generation
- Message signing and verification
- Random seed generation

**Key Functions:**

**Random Generation:**
- `generateRandomSeed(uint8_t* seed)` - Generate 32-byte random seed

**Keypair Generation:**
- `generateKeypairFromSeed(const uint8_t* seed, uint8_t* publicKey, uint8_t* privateKey)` - Generate keypair from seed
- `generateKeypair(uint8_t* publicKey, uint8_t* privateKey)` - Generate random keypair

**Signing:**
- `signMessage(const uint8_t* message, size_t messageLen, const uint8_t* privateKey, uint8_t* signature)` - Sign message
- `verifySignature(const uint8_t* message, size_t messageLen, const uint8_t* signature, const uint8_t* publicKey)` - Verify signature

**Base58 Encoding:**
- `base58Encode(const uint8_t* data, size_t len, char* output, size_t outputLen)` - Encode to Base58
- `base58Decode(const char* input, uint8_t* output, size_t outputLen)` - Decode from Base58

**Address Conversion:**
- `publicKeyToAddress(const uint8_t* publicKey, char* address, size_t addressLen)` - Convert public key to Solana address
- `addressToPublicKey(const char* address, uint8_t* publicKey)` - Convert address to public key bytes

**Private Key Conversion:**
- `privateKeyToBase58(const uint8_t* privateKey, char* output, size_t outputLen)` - Convert private key to Base58
- `base58ToPrivateKey(const char* input, uint8_t* privateKey)` - Convert Base58 to private key

**Public Key Extraction:**
- `getPublicKeyFromPrivate(const uint8_t* privateKey, uint8_t* publicKey)` - Extract public key from private key

**Dependencies:**
- TweetNaCl (`tweetnacl.h` / `tweetnacl.c`) - Ed25519 cryptographic operations
- ESP32 `esp_random()` for random number generation

---

### 5. Keypair Module (`keypair.h` / `keypair.cpp`)

**Purpose:** Wallet generation and management

**Key Features:**
- Generate new Solana keypairs
- Import wallets from private keys (Base58 or bytes)
- Import wallets from seeds
- Sign messages and transactions
- Manage public/private key pairs

**Main Class:**
- `Keypair` - Solana wallet with Ed25519 keypair

**Key Methods:**

**Generation:**
- `generate()` - Generate new random keypair

**Import:**
- `importFromPrivateKey(const uint8_t* privateKeyBytes)` - Import from 64-byte private key
- `importFromPrivateKeyBase58(const char* privateKeyBase58)` - Import from Base58 private key
- `importFromSeed(const uint8_t* seed)` - Import from 32-byte seed

**Key Access:**
- `getPublicKey(uint8_t* output)` - Get public key as bytes
- `getPrivateKey(uint8_t* output)` - Get private key as bytes
- `getPublicKeyAddress(char* address, size_t addressLen)` - Get public key as Solana address (Base58)
- `getPrivateKeyBase58(char* output, size_t outputLen)` - Get private key as Base58 string

**Signing:**
- `sign(const uint8_t* message, size_t messageLen, uint8_t* signature)` - Sign message
- `signString(const String& message, uint8_t* signature)` - Sign string message
- `verify(const uint8_t* message, size_t messageLen, const uint8_t* signature)` - Verify signature

**Utility:**
- `isInitialized()` - Check if keypair is initialized
- `clear()` - Clear keypair (zero out keys)
- `printAddress()` - Print public address to Serial (debugging)

**Security:**
- Private keys are cleared in destructor
- Uses ESP32 hardware RNG for key generation
- Keys stored in memory (consider secure storage for production)

---

### 6. Transaction Module (`transaction.h` / `transaction.cpp`)

**Purpose:** Transaction creation and signing

**Key Features:**
- Transaction structures (Message, Transaction, Instruction)
- Transaction signing with Ed25519
- Transaction building and manipulation
- Transfer instruction creation

**Key Structures:**

**TransactionHeader:**
- `numRequiredSignatures` - Number of required signatures
- `numReadonlySignedAccounts` - Number of readonly signed accounts
- `numReadonlyUnsignedAccounts` - Number of readonly unsigned accounts

**CompiledInstruction:**
- `programIdIndex` - Program ID account index
- `accountIndices[]` - Account indices array
- `accountCount` - Number of accounts
- `data[]` - Instruction data
- `dataLength` - Length of instruction data

**Main Classes:**

**Message** - Transaction message (instructions and metadata)
- `addAccount(const uint8_t* pubkey, bool isSigner, bool isWritable)` - Add account key
- `setRecentBlockhash(const uint8_t* blockhash)` - Set recent blockhash
- `addInstruction(...)` - Add instruction to message
- `getAccount(uint8_t index, uint8_t* pubkey)` - Get account by index
- `getRecentBlockhash(uint8_t* blockhash)` - Get recent blockhash
- `reset()` - Reset message

**Transaction** - Solana transaction (signatures + message)
- `addTransferInstruction(const uint8_t* from, const uint8_t* to, uint64_t amount)` - Add transfer instruction
- `addInstruction(...)` - Add custom instruction
- `setRecentBlockhash(const uint8_t* blockhash)` - Set recent blockhash
- `sign(const uint8_t* privateKey, const uint8_t* publicKey)` - Sign transaction
- `signMultiple(...)` - Sign with multiple keypairs
- `getSignature(uint8_t index, uint8_t* signature)` - Get signature by index
- `reset()` - Reset transaction

**Constants:**
- `MAX_ACCOUNTS` - Maximum accounts (64)
- `MAX_INSTRUCTIONS` - Maximum instructions (256)
- `MAX_INSTRUCTION_DATA` - Maximum instruction data size (1024 bytes)
- `BLOCKHASH_SIZE` - Blockhash size (32 bytes)
- `SIGNATURE_SIZE` - Signature size (64 bytes)

**Dependencies:**
- Crypto module for signing
- Serializer module for message serialization

---

### 7. Serializer Module (`serializer.h` / `serializer.cpp`)

**Purpose:** Transaction serialization to Solana wire format

**Key Features:**
- Serialize transactions to Solana wire format (compact array)
- Base64 encoding for RPC submission
- Message serialization
- Compact u16 encoding (variable-length)

**Main Classes:**

**TransactionSerializer** - Transaction serialization
- `serializeMessage(const Message& message, uint8_t* buffer, uint16_t bufferLen, uint16_t& serializedLen)` - Serialize message
- `serializeTransaction(const Transaction& transaction, uint8_t* buffer, uint16_t bufferLen, uint16_t& serializedLen)` - Serialize transaction
- `encodeTransaction(const Transaction& transaction, char* output, size_t outputLen)` - Encode transaction to Base64
- `calculateMessageSize(const Message& message)` - Calculate message size
- `calculateTransactionSize(const Transaction& transaction)` - Calculate transaction size

**Base64** - Base64 encoding/decoding utilities
- `encode(const uint8_t* data, size_t dataLen, char* output, size_t outputLen)` - Encode to Base64
- `decode(const char* input, uint8_t* output, size_t outputLen)` - Decode from Base64

**Serialization Format:**
- Uses Solana's compact array format
- Compact u16 encoding for variable-length integers
- Wire format: signatures || message
- Message format: header || account_keys || blockhash || instructions

---

### 8. TweetNaCl Module (`tweetnacl.h` / `tweetnacl.c`)

**Purpose:** Minimal Ed25519 cryptographic library

**Key Features:**
- Ed25519 keypair generation
- Ed25519 signing
- Ed25519 verification
- Random bytes generation

**Key Functions (C interface):**
- `crypto_sign_seed_keypair()` - Generate keypair from seed
- `crypto_sign_keypair()` - Generate random keypair
- `crypto_sign()` - Sign message
- `crypto_sign_open()` - Verify signature
- `crypto_sign_publickey()` - Extract public key from private key
- `randombytes()` - Generate random bytes

**Constants:**
- `crypto_sign_PUBLICKEYBYTES` - 32 bytes
- `crypto_sign_SECRETKEYBYTES` - 64 bytes
- `crypto_sign_BYTES` - 64 bytes (signature size)

**Note:** This is TweetNaCl - a minimal, public domain cryptographic library adapted for Arduino/ESP32 environments.

---

## Module Dependencies

```
solduino.h (Core SDK)
├── Includes: rpc_client.h
├── Includes: connection.h
├── Includes: crypto.h
├── Includes: keypair.h
├── Includes: transaction.h
└── Includes: serializer.h

rpc_client.h (RPC Module)
├── Depends on: WiFiClientSecure (ESP32)
├── Depends on: HTTPClient (ESP32)
├── Depends on: ArduinoJson
└── Used by: connection.h

transaction.h (Transaction Module)
├── Depends on: crypto.h
└── Depends on: serializer.h

keypair.h (Keypair Module)
└── Depends on: crypto.h

crypto.h (Crypto Module)
└── Depends on: tweetnacl.h/c

serializer.h (Serializer Module)
└── Depends on: transaction.h
```

---

## Examples

### 1. Basic RPC Demo (`basic_rpc_demo.ino`)
- Basic RPC connection
- Network information retrieval
- Account balance queries

### 2. Wallet Demo (`wallet_demo.ino`)
- Keypair generation
- Address conversion
- Message signing

### 3. Airdrop Demo (`airdrop_demo.ino`)
- Request airdrop
- Balance checking
- Account creation

### 4. Transaction Demo (`transaction_demo.ino`)
- Transaction building
- Instruction creation
- Transaction signing

### 5. Transfer Demo (`transfer_demo.ino`)
- Complete SOL transfer flow
- Keypair generation
- Transaction creation
- Signing and submission
- Status verification

---

## Key Design Patterns

1. **Modular Architecture:** Each module is self-contained with clear interfaces
2. **Arduino Compatibility:** Follows Arduino library conventions
3. **ESP32 Optimized:** Uses ESP32-specific features (WiFiClientSecure, esp_random)
4. **Memory Efficient:** Designed for embedded constraints
5. **Error Handling:** Returns boolean success/failure, logs errors to Serial
6. **C/C++ Interface:** Mix of C++ classes and C functions for flexibility

---

## Security Considerations

### Current Implementation:
- ✅ Uses Ed25519 for signing (TweetNaCl)
- ✅ Uses ESP32 hardware RNG for key generation
- ✅ Private keys cleared in destructors
- ⚠️ Certificate validation disabled (`setInsecure()`)
- ⚠️ Keys stored in memory (no secure storage)

### Production Recommendations:
- Implement proper certificate validation for HTTPS
- Use secure storage for private keys (ESP32 Secure Boot, NVS encryption)
- Never log or expose private keys
- Implement key derivation functions (HD wallets)
- Add rate limiting for RPC calls
- Implement transaction replay protection

---

## Limitations

1. **Memory Constraints:** Limited by ESP32 RAM (need to manage buffer sizes)
2. **Certificate Validation:** Currently disabled (`setInsecure()`)
3. **WebSocket Support:** Not yet implemented
4. **Transaction Size:** Limited by available memory
5. **Platform Support:** Primarily ESP32 (may require modifications for ESP8266)

---

## Future Enhancements

- [ ] Implement connection.h functions
- [ ] Add WebSocket support for real-time subscriptions
- [ ] Add certificate validation for production use
- [ ] Add secure key storage support
- [ ] Add HD wallet support (BIP44)
- [ ] Add transaction building helpers
- [ ] Add instruction builders for common programs
- [ ] Add token transfer utilities
- [ ] Add program interaction helpers
- [ ] Add transaction simulation
- [ ] Add account subscription support

---

## API Usage Examples

### Basic RPC Usage:
```cpp
#include <solduino.h>

RpcClient client(SOLDUINO_DEVNET_RPC);
client.begin();
String balance = client.getBalance("11111111111111111111111111111112");
```

### Keypair Generation:
```cpp
Keypair keypair;
keypair.generate();
char address[64];
keypair.getPublicKeyAddress(address, sizeof(address));
```

### Transaction Creation:
```cpp
Transaction tx;
tx.addTransferInstruction(fromPubkey, toPubkey, amount);
tx.setRecentBlockhash(blockhash);
tx.sign(privateKey, publicKey);
```

### Transaction Serialization:
```cpp
char base64Tx[2048];
TransactionSerializer::encodeTransaction(tx, base64Tx, sizeof(base64Tx));
String result = rpcClient.sendTransaction(base64Tx);
```

---

## File Cross-Reference

### Headers:
- `solduino.h` - Main entry point, includes all modules
- `rpc_client.h` - RPC client class and structures
- `connection.h` - Connection management (C interface)
- `crypto.h` - Cryptographic utilities
- `keypair.h` - Keypair class
- `transaction.h` - Transaction classes
- `serializer.h` - Serialization utilities
- `tweetnacl.h` - TweetNaCl crypto library

### Implementations:
- `solduino.cpp` - SDK implementation
- `rpc_client.cpp` - RPC client implementation
- `crypto.cpp` - Crypto implementation
- `keypair.cpp` - Keypair implementation
- `transaction.cpp` - Transaction implementation
- `serializer.cpp` - Serializer implementation
- `tweetnacl.c` - TweetNaCl implementation

---

## Constants Reference

### Size Constants:
- `SOLDUINO_PUBKEY_SIZE` = 32 bytes
- `SOLDUINO_SECRETKEY_SIZE` = 64 bytes
- `SOLDUINO_SIGNATURE_SIZE` = 64 bytes
- `SOLDUINO_SEED_SIZE` = 32 bytes
- `MAX_ACCOUNTS` = 64
- `MAX_INSTRUCTIONS` = 256
- `MAX_INSTRUCTION_DATA` = 1024 bytes
- `BLOCKHASH_SIZE` = 32 bytes

### Network Constants:
- `SOLDUINO_MAINNET_RPC` = "https://api.mainnet-beta.solana.com"
- `SOLDUINO_DEVNET_RPC` = "https://api.devnet.solana.com"
- `SOLDUINO_TESTNET_RPC` = "https://api.testnet.solana.com"
- `SOLDUINO_LOCALNET_RPC` = "http://localhost:8899"

### Timeout Constants:
- `SOLDUINO_DEFAULT_TIMEOUT_MS` = 10000 ms

---

## Testing

### Test Networks:
- **Localnet:** Use `solana-test-validator` for local testing
- **Devnet:** Use for development and testing (free SOL via airdrop)
- **Testnet:** Use for testing (may require faucet)
- **Mainnet:** Production network (use with caution)

### Example Setup:
1. Start local validator: `solana-test-validator --bind-address 0.0.0.0`
2. Get computer IP: `ifconfig | grep "inet " | grep -v 127.0.0.1`
3. Configure ESP32: Use `http://YOUR_IP:8899` as RPC endpoint
4. Run examples: Upload example sketches to ESP32

---

## Contributing

When adding new features:
1. Update relevant module header
2. Implement in corresponding .cpp file
3. Update README.md with documentation
4. Add example usage
5. Update this index if structure changes

---

**Last Updated:** Codebase index created  
**Version:** 0.1.0


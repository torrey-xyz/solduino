#ifndef SOLDUINO_H
#define SOLDUINO_H

#include "Arduino.h"

// ============================================================================
// Solduino - Solana Library for Arduino/ESP32
// ============================================================================
// Core Embedded Software Development Kit (SDK)
// 
// This is the main entry point for the Solduino library. It provides:
// - Core SDK functionality and version management
// - Module includes for wallet generation, transaction signing, and RPC
// - Library-wide constants and configuration
// ============================================================================

// Version information
#define SOLDUINO_VERSION_MAJOR 0
#define SOLDUINO_VERSION_MINOR 1
#define SOLDUINO_VERSION_PATCH 0
#define SOLDUINO_VERSION_STRING "0.1.0"

// Library configuration constants
#define SOLDUINO_MAX_URL_LENGTH 256
#define SOLDUINO_DEFAULT_TIMEOUT_MS 10000
#define SOLDUINO_MAX_KEYPAIR_SIZE 64
#define SOLDUINO_PUBLIC_KEY_SIZE 32
#define SOLDUINO_PRIVATE_KEY_SIZE 64
#define SOLDUINO_SIGNATURE_SIZE 64

// RPC endpoint constants
#define SOLDUINO_MAINNET_RPC "https://api.mainnet-beta.solana.com"
#define SOLDUINO_DEVNET_RPC "https://api.devnet.solana.com"
#define SOLDUINO_TESTNET_RPC "https://api.testnet.solana.com"
#define SOLDUINO_LOCALNET_RPC "http://localhost:8899"

// Commitment levels
enum SolduinoCommitment {
    SOLDUINO_COMMITMENT_PROCESSED = 0,
    SOLDUINO_COMMITMENT_CONFIRMED = 1,
    SOLDUINO_COMMITMENT_FINALIZED = 2
};

// Forward declarations
class RpcClient;

/**
 * Solduino Core SDK Class
 * 
 * Main entry point for the Solduino library. Provides version information
 * and library initialization.
 */
class Solduino {
public:
    /**
     * Constructor
     * Initializes the Solduino SDK
     */
    Solduino();
    
    /**
     * Get library version string
     * @return Version string (e.g., "0.1.0")
     */
    static const char* getVersion();
    
    /**
     * Get major version number
     * @return Major version
     */
    static int getVersionMajor();
    
    /**
     * Get minor version number
     * @return Minor version
     */
    static int getVersionMinor();
    
    /**
     * Get patch version number
     * @return Patch version
     */
    static int getVersionPatch();
    
    /**
     * Initialize library (placeholder for future initialization logic)
     * @return true if initialization successful
     */
    bool begin();
    
    /**
     * Cleanup library resources
     */
    void end();
    
    /**
     * Test function - prints library information
     */
    void hello(void);
};

// ============================================================================
// Module Includes
// ============================================================================
// Include all Solduino modules for convenient access

// RPC Communication Module
#include "rpc_client.h"

// Connection Management Module
#include "connection.h"

// Wallet Generation and Management Module
#include "crypto.h"
#include "keypair.h"

// Transaction Signing Module
#include "transaction.h"
#include "serializer.h"

#endif // SOLDUINO_H

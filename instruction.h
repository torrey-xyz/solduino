#ifndef SOLDUINO_INSTRUCTION_H
#define SOLDUINO_INSTRUCTION_H

#include <Arduino.h>
#include <stdint.h>
#include "crypto.h"

// ============================================================================
// Solduino Instruction Module
// ============================================================================
// Provides instruction builder API for Solana programs:
// - AccountMeta struct (pubkey + isSigner + isWritable)
// - Instruction class with fluent data-encoding helpers
// - Borsh-compatible little-endian serialization
// ============================================================================

// Per-instruction account limit (override at compile time if needed)
#ifndef MAX_IX_ACCOUNTS
#define MAX_IX_ACCOUNTS 10
#endif

// Per-instruction data limit (override at compile time if needed)
#ifndef MAX_IX_DATA
#define MAX_IX_DATA 256
#endif

/**
 * Account Metadata
 * Bundles a public key with its signer/writable flags,
 * analogous to web3.js AccountMeta.
 *
 * The pubkey is stored by value (32-byte copy) so the
 * struct is self-contained and safe after the source goes
 * out of scope.
 */
struct AccountMeta {
    uint8_t pubkey[SOLDUINO_PUBKEY_SIZE];
    bool isSigner;
    bool isWritable;
};

/**
 * Instruction Builder
 *
 * Builds a single Solana instruction with:
 *   - Program ID
 *   - Ordered list of AccountMeta keys
 *   - Serialized instruction data
 *
 * Usage:
 *   Instruction ix;
 *   ix.setProgram(programId);
 *   ix.addKey(payer, true, true);
 *   ix.addKey(dataAcct, false, true);
 *   ix.writeU8(discriminator);
 *   ix.writeU64LE(value);
 *
 *   Transaction tx;
 *   tx.add(ix);
 */
class Instruction {
private:
    uint8_t programId_[SOLDUINO_PUBKEY_SIZE];
    AccountMeta keys_[MAX_IX_ACCOUNTS];
    uint8_t keyCount_;
    uint8_t data_[MAX_IX_DATA];
    uint16_t dataLen_;
    bool hasProgram_;

public:
    Instruction();

    // ---- Program ID --------------------------------------------------------

    /**
     * Set the program ID this instruction targets.
     * @param programId 32-byte public key of the program
     * @return true if successful
     */
    bool setProgram(const uint8_t* programId);

    /**
     * Set the program ID from a Base58 address string.
     * @param base58Address Null-terminated Base58 program address
     * @return true if successful
     */
    bool setProgramBase58(const char* base58Address);

    // ---- Account Keys ------------------------------------------------------

    /**
     * Add an account key (AccountMeta) to the instruction.
     * Accounts are kept in the order they are added.
     * @param pubkey   32-byte public key
     * @param isSigner true if the account must sign the transaction
     * @param isWritable true if the account data may be modified
     * @return true if successful, false if limit reached or null pointer
     */
    bool addKey(const uint8_t* pubkey, bool isSigner, bool isWritable);

    /**
     * Add an account key from a Base58 address string.
     * @param base58Address Null-terminated Base58 address
     * @param isSigner true if the account must sign the transaction
     * @param isWritable true if the account data may be modified
     * @return true if successful
     */
    bool addKeyBase58(const char* base58Address, bool isSigner, bool isWritable);

    // ---- Data Encoding (little-endian / Borsh-compatible) ------------------

    /**
     * Write a single unsigned byte to the data buffer.
     * @return true if there was room
     */
    bool writeU8(uint8_t value);

    /**
     * Write a 16-bit unsigned integer (little-endian).
     * @return true if there was room
     */
    bool writeU16LE(uint16_t value);

    /**
     * Write a 32-bit unsigned integer (little-endian).
     * @return true if there was room
     */
    bool writeU32LE(uint32_t value);

    /**
     * Write a 64-bit unsigned integer (little-endian).
     * @return true if there was room
     */
    bool writeU64LE(uint64_t value);

    /**
     * Write a 64-bit signed integer (little-endian).
     * @return true if there was room
     */
    bool writeI64LE(int64_t value);

    /**
     * Write a boolean value (Borsh encoding: 1 byte, 0 or 1).
     * @return true if there was room
     */
    bool writeBool(bool value);

    /**
     * Write raw bytes to the data buffer.
     * Use for Anchor discriminators, string payloads, etc.
     * @param bytes Pointer to data
     * @param len   Number of bytes to write
     * @return true if there was room
     */
    bool writeBytes(const uint8_t* bytes, uint16_t len);

    /**
     * Write a 32-byte public key into the data buffer.
     * Useful when instruction data includes an embedded pubkey.
     * @param pubkey 32-byte public key
     * @return true if there was room
     */
    bool writePubkey(const uint8_t* pubkey);

    // ---- Reset -------------------------------------------------------------

    /**
     * Clear only the data buffer (keep program & keys).
     */
    void resetData();

    /**
     * Reset the entire instruction (program, keys, data).
     */
    void reset();

    // ---- Getters -----------------------------------------------------------

    /** Get the program ID (32 bytes). Returns nullptr if not set. */
    const uint8_t* getProgram() const;

    /** Check if a program ID has been set. */
    bool hasProgram() const { return hasProgram_; }

    /** Get the number of account keys. */
    uint8_t getKeyCount() const { return keyCount_; }

    /**
     * Get an account key by index.
     * @param index Zero-based index
     * @return Pointer to AccountMeta, or nullptr if out of range
     */
    const AccountMeta* getKey(uint8_t index) const;

    /** Get pointer to the raw instruction data. */
    const uint8_t* getData() const { return data_; }

    /** Get the current length of instruction data. */
    uint16_t getDataLength() const { return dataLen_; }

    /** Get the remaining capacity in the data buffer. */
    uint16_t getDataCapacity() const { return MAX_IX_DATA - dataLen_; }
};

#endif // SOLDUINO_INSTRUCTION_H

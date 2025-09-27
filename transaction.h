#ifndef SOLDUINO_TRANSACTION_H
#define SOLDUINO_TRANSACTION_H

#include <Arduino.h>
#include <stdint.h>
#include "crypto.h"

// ============================================================================
// Solduino Transaction Module
// ============================================================================
// Provides transaction creation and signing functionality:
// - Transaction structures (Message, Transaction, Instruction)
// - Transaction signing with Ed25519
// - Transaction building and manipulation
// ============================================================================

// Constants
// These defaults are tuned for embedded targets to avoid excessive stack usage
// when instantiating a Transaction on the stack. They can be overridden at
// compile time by defining the macros before including this header.
#ifndef MAX_ACCOUNTS
#define MAX_ACCOUNTS 16
#endif

#ifndef MAX_INSTRUCTIONS
#define MAX_INSTRUCTIONS 8
#endif

#ifndef MAX_INSTRUCTION_DATA
#define MAX_INSTRUCTION_DATA 256
#endif
#define BLOCKHASH_SIZE 32
#define SIGNATURE_SIZE 64

// Forward declarations
class Transaction;
class Message;
class Instruction;
class TransactionSerializer;

/**
 * Transaction Header
 * Contains metadata about required signatures and readonly accounts
 */
struct TransactionHeader {
    uint8_t numRequiredSignatures;
    uint8_t numReadonlySignedAccounts;
    uint8_t numReadonlyUnsignedAccounts;
};

/**
 * Compiled Instruction
 * Represents a Solana instruction in wire format
 */
struct CompiledInstruction {
    uint8_t programIdIndex;
    uint8_t accountIndices[MAX_ACCOUNTS];
    uint8_t accountCount;
    uint8_t data[MAX_INSTRUCTION_DATA];
    uint16_t dataLength;
};

/**
 * Transaction Message
 * Contains the transaction instructions and metadata
 */
class Message {
private:
    TransactionHeader header;
    uint8_t accountKeys[MAX_ACCOUNTS][SOLDUINO_PUBKEY_SIZE];
    uint8_t accountCount;
    uint8_t recentBlockhash[BLOCKHASH_SIZE];
    CompiledInstruction instructions[MAX_INSTRUCTIONS];
    uint8_t instructionCount;
    
    bool isValidAccountIndex(uint8_t index) const;
    uint8_t findAccountIndex(const uint8_t* pubkey) const;

public:
    Message();
    
    /**
     * Add an account key to the message
     * @param pubkey Public key (32 bytes)
     * @return Account index, or -1 on error
     */
    int8_t addAccount(const uint8_t* pubkey, bool isSigner, bool isWritable);
    
    /**
     * Set the recent blockhash
     * @param blockhash Blockhash (32 bytes)
     * @return true if successful
     */
    bool setRecentBlockhash(const uint8_t* blockhash);
    
    /**
     * Add an instruction to the message
     * @param programId Program ID public key (32 bytes)
     * @param accounts Array of account public keys
     * @param accountCount Number of accounts
     * @param data Instruction data
     * @param dataLength Length of instruction data
     * @return true if successful
     */
    bool addInstruction(const uint8_t* programId, 
                       const uint8_t* accounts[], 
                       uint8_t accountCount,
                       const uint8_t* data, 
                       uint16_t dataLength);
    
    /**
     * Get message account count
     */
    uint8_t getAccountCount() const { return accountCount; }
    
    /**
     * Get instruction count
     */
    uint8_t getInstructionCount() const { return instructionCount; }
    
    /**
     * Get account public key by index
     * @param index Account index
     * @param pubkey Output buffer (32 bytes)
     * @return true if successful
     */
    bool getAccount(uint8_t index, uint8_t* pubkey) const;
    
    /**
     * Get recent blockhash
     * @param blockhash Output buffer (32 bytes)
     * @return true if successful
     */
    bool getRecentBlockhash(uint8_t* blockhash) const;
    
    /**
     * Get transaction header
     */
    TransactionHeader getHeader() const { return header; }
    
    /**
     * Reset the message (clear all instructions and accounts)
     */
    void reset();
    
    // Friend classes for access to private members
    friend class TransactionSerializer;
    friend class Transaction;
};

/**
 * Solana Transaction
 * Contains signatures and message
 */
class Transaction {
private:
    uint8_t signatures[MAX_ACCOUNTS][SIGNATURE_SIZE];
    uint8_t signatureCount;
    Message message;
    bool isValid;

public:
    Transaction();
    
    /**
     * Create a transfer instruction and add it to the transaction
     * @param from From account public key (32 bytes)
     * @param to To account public key (32 bytes)
     * @param amount Amount in lamports
     * @return true if successful
     */
    bool addTransferInstruction(const uint8_t* from, 
                               const uint8_t* to, 
                               uint64_t amount);
    
    /**
     * Add a custom instruction to the transaction
     * @param programId Program ID public key (32 bytes)
     * @param accounts Array of account public keys
     * @param accountCount Number of accounts
     * @param data Instruction data
     * @param dataLength Length of instruction data
     * @return true if successful
     */
    bool addInstruction(const uint8_t* programId,
                       const uint8_t* accounts[],
                       uint8_t accountCount,
                       const uint8_t* data,
                       uint16_t dataLength);
    
    /**
     * Set the recent blockhash
     * @param blockhash Blockhash (32 bytes)
     * @return true if successful
     */
    bool setRecentBlockhash(const uint8_t* blockhash);
    
    /**
     * Sign the transaction with a keypair
     * @param privateKey Private key (64 bytes)
     * @param publicKey Public key (32 bytes) - must be a signer account
     * @return true if successful
     */
    bool sign(const uint8_t* privateKey, const uint8_t* publicKey);
    
    /**
     * Sign the transaction with multiple keypairs
     * @param privateKeys Array of private keys (64 bytes each)
     * @param publicKeys Array of public keys (32 bytes each)
     * @param count Number of keypairs
     * @return true if successful
     */
    bool signMultiple(const uint8_t* privateKeys[], 
                     const uint8_t* publicKeys[],
                     uint8_t count);
    
    /**
     * Get the transaction message
     */
    Message& getMessage() { return message; }
    const Message& getMessage() const { return message; }
    
    /**
     * Get signature count
     */
    uint8_t getSignatureCount() const { return signatureCount; }
    
    /**
     * Get a signature by index
     * @param index Signature index
     * @param signature Output buffer (64 bytes)
     * @return true if successful
     */
    bool getSignature(uint8_t index, uint8_t* signature) const;
    
    /**
     * Check if transaction is valid
     */
    bool isValidTransaction() const { return isValid; }
    
    /**
     * Reset the transaction
     */
    void reset();
    
    // Friend class for serializer access
    friend class TransactionSerializer;
};

#endif // SOLDUINO_TRANSACTION_H


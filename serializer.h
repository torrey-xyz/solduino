#ifndef SOLDUINO_SERIALIZER_H
#define SOLDUINO_SERIALIZER_H

#include <Arduino.h>
#include <stdint.h>
#include "transaction.h"

// ============================================================================
// Solduino Serializer Module
// ============================================================================
// Provides transaction serialization functionality:
// - Serialize transactions to Solana wire format (compact array)
// - Base64 encoding for RPC submission
// - Message serialization
// ============================================================================

/**
 * Transaction Serializer
 * Handles serialization of Solana transactions to wire format
 */
class TransactionSerializer {
private:
    /**
     * Write a compact u16 value (variable-length encoding)
     * @param buffer Output buffer
     * @param offset Current offset (updated)
     * @param maxLen Maximum buffer length
     * @param value Value to write
     * @return true if successful
     */
    static bool writeCompactU16(uint8_t* buffer, uint16_t& offset, uint16_t maxLen, uint16_t value);
    
    /**
     * Serialize transaction header
     * @param buffer Output buffer
     * @param offset Current offset (updated)
     * @param maxLen Maximum buffer length
     * @param header Header to serialize
     * @return true if successful
     */
    static bool serializeHeader(uint8_t* buffer, uint16_t& offset, uint16_t maxLen, const TransactionHeader& header);
    
    /**
     * Serialize account keys array
     * @param buffer Output buffer
     * @param offset Current offset (updated)
     * @param maxLen Maximum buffer length
     * @param accountKeys Array of account keys
     * @param accountCount Number of accounts
     * @return true if successful
     */
    static bool serializeAccountKeys(uint8_t* buffer, uint16_t& offset, uint16_t maxLen,
                                     const uint8_t accountKeys[][SOLDUINO_PUBKEY_SIZE],
                                     uint8_t accountCount);
    
    /**
     * Serialize recent blockhash
     * @param buffer Output buffer
     * @param offset Current offset (updated)
     * @param maxLen Maximum buffer length
     * @param blockhash Blockhash (32 bytes)
     * @return true if successful
     */
    static bool serializeBlockhash(uint8_t* buffer, uint16_t& offset, uint16_t maxLen, const uint8_t* blockhash);
    
    /**
     * Serialize compiled instructions
     * @param buffer Output buffer
     * @param offset Current offset (updated)
     * @param maxLen Maximum buffer length
     * @param instructions Array of instructions
     * @param instructionCount Number of instructions
     * @return true if successful
     */
    static bool serializeInstructions(uint8_t* buffer, uint16_t& offset, uint16_t maxLen,
                                     const CompiledInstruction* instructions,
                                     uint8_t instructionCount);
    
    /**
     * Serialize a compiled instruction
     * @param buffer Output buffer
     * @param offset Current offset (updated)
     * @param maxLen Maximum buffer length
     * @param instruction Instruction to serialize
     * @return true if successful
     */
    static bool serializeInstruction(uint8_t* buffer, uint16_t& offset, uint16_t maxLen,
                                    const CompiledInstruction& instruction);

public:
    /**
     * Serialize a message to wire format
     * @param message Message to serialize
     * @param buffer Output buffer
     * @param bufferLen Maximum buffer length
     * @param serializedLen Output: actual serialized length
     * @return true if successful
     */
    static bool serializeMessage(const Message& message, uint8_t* buffer, uint16_t bufferLen, uint16_t& serializedLen);
    
    /**
     * Serialize a transaction to wire format
     * @param transaction Transaction to serialize
     * @param buffer Output buffer
     * @param bufferLen Maximum buffer length
     * @param serializedLen Output: actual serialized length
     * @return true if successful
     */
    static bool serializeTransaction(const Transaction& transaction, uint8_t* buffer, uint16_t bufferLen, uint16_t& serializedLen);
    
    /**
     * Encode transaction to base64 string for RPC submission
     * @param transaction Transaction to encode
     * @param output Output string buffer
     * @param outputLen Maximum output length
     * @return true if successful
     */
    static bool encodeTransaction(const Transaction& transaction, char* output, size_t outputLen);
    
    /**
     * Encode transaction to base58 string for RPC submission
     * @param transaction Transaction to encode
     * @param output Output string buffer
     * @param outputLen Maximum output length
     * @return true if successful
     */
    static bool encodeTransactionBase58(const Transaction& transaction, char* output, size_t outputLen);
    
    /**
     * Calculate serialized message size (for buffer allocation)
     * @param message Message to measure
     * @return Estimated size in bytes
     */
    static uint16_t calculateMessageSize(const Message& message);
    
    /**
     * Calculate serialized transaction size (for buffer allocation)
     * @param transaction Transaction to measure
     * @return Estimated size in bytes
     */
    static uint16_t calculateTransactionSize(const Transaction& transaction);
};

/**
 * Base64 encoding/decoding utilities
 */
class Base64 {
public:
    /**
     * Encode bytes to base64 string
     * @param data Input data
     * @param dataLen Length of data
     * @param output Output string buffer
     * @param outputLen Maximum output length
     * @return Length of encoded string, or 0 on error
     */
    static size_t encode(const uint8_t* data, size_t dataLen, char* output, size_t outputLen);
    
    /**
     * Decode base64 string to bytes
     * @param input Base64 encoded string
     * @param output Output buffer
     * @param outputLen Maximum output length
     * @return Length of decoded data, or 0 on error
     */
    static size_t decode(const char* input, uint8_t* output, size_t outputLen);
};

#endif // SOLDUINO_SERIALIZER_H


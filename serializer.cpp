#include "serializer.h"
#include "crypto.h"
#include <string.h>

// ============================================================================
// TransactionSerializer Implementation
// ============================================================================

bool TransactionSerializer::writeCompactU16(uint8_t* buffer, uint16_t& offset, uint16_t maxLen, uint16_t value) {
    if (!buffer) {
        return false;
    }
    // Solana shortvec encoding (LEB128-like): write low 7 bits first; set MSB if more bytes follow
    do {
        if (offset >= maxLen) {
            return false;
        }
        uint8_t byte = value & 0x7F;
        value >>= 7;
        if (value != 0) {
            byte |= 0x80; // more bytes follow
        }
        buffer[offset++] = byte;
    } while (value != 0);
    return true;
}

bool TransactionSerializer::serializeHeader(uint8_t* buffer, uint16_t& offset, uint16_t maxLen, const TransactionHeader& header) {
    if (!buffer || offset + 3 > maxLen) {
        return false;
    }
    
    buffer[offset++] = header.numRequiredSignatures;
    buffer[offset++] = header.numReadonlySignedAccounts;
    buffer[offset++] = header.numReadonlyUnsignedAccounts;
    
    return true;
}

bool TransactionSerializer::serializeAccountKeys(uint8_t* buffer, uint16_t& offset, uint16_t maxLen,
                                                 const uint8_t accountKeys[][SOLDUINO_PUBKEY_SIZE],
                                                 uint8_t accountCount) {
    if (!buffer || offset + 1 >= maxLen) {
        return false;
    }
    
    // Write account count as compact u16
    if (!writeCompactU16(buffer, offset, maxLen, accountCount)) {
        return false;
    }
    
    // Write each account key (32 bytes each)
    for (uint8_t i = 0; i < accountCount; i++) {
        if (offset + SOLDUINO_PUBKEY_SIZE > maxLen) {
            return false;
        }
        memcpy(buffer + offset, accountKeys[i], SOLDUINO_PUBKEY_SIZE);
        offset += SOLDUINO_PUBKEY_SIZE;
    }
    
    return true;
}

bool TransactionSerializer::serializeBlockhash(uint8_t* buffer, uint16_t& offset, uint16_t maxLen, const uint8_t* blockhash) {
    if (!buffer || !blockhash || offset + BLOCKHASH_SIZE > maxLen) {
        return false;
    }
    
    memcpy(buffer + offset, blockhash, BLOCKHASH_SIZE);
    offset += BLOCKHASH_SIZE;
    
    return true;
}

bool TransactionSerializer::serializeInstruction(uint8_t* buffer, uint16_t& offset, uint16_t maxLen,
                                                 const CompiledInstruction& instruction) {
    if (!buffer || offset + 1 >= maxLen) {
        return false;
    }
    
    // Write program ID index
    buffer[offset++] = instruction.programIdIndex;
    
    // Write account indices count as compact u16
    if (!writeCompactU16(buffer, offset, maxLen, instruction.accountCount)) {
        return false;
    }
    
    // Write account indices
    if (offset + instruction.accountCount > maxLen) {
        return false;
    }
    for (uint8_t i = 0; i < instruction.accountCount; i++) {
        buffer[offset++] = instruction.accountIndices[i];
    }
    
    // Write data length as compact u16
    if (!writeCompactU16(buffer, offset, maxLen, instruction.dataLength)) {
        return false;
    }
    
    // Write instruction data
    if (offset + instruction.dataLength > maxLen) {
        return false;
    }
    if (instruction.dataLength > 0) {
        memcpy(buffer + offset, instruction.data, instruction.dataLength);
        offset += instruction.dataLength;
    }
    
    return true;
}

bool TransactionSerializer::serializeInstructions(uint8_t* buffer, uint16_t& offset, uint16_t maxLen,
                                                  const CompiledInstruction* instructions,
                                                  uint8_t instructionCount) {
    if (!buffer || !instructions || offset + 1 >= maxLen) {
        return false;
    }
    
    // Write instruction count as compact u16
    if (!writeCompactU16(buffer, offset, maxLen, instructionCount)) {
        return false;
    }
    
    // Serialize each instruction
    for (uint8_t i = 0; i < instructionCount; i++) {
        if (!serializeInstruction(buffer, offset, maxLen, instructions[i])) {
            return false;
        }
    }
    
    return true;
}

bool TransactionSerializer::serializeMessage(const Message& message, uint8_t* buffer, uint16_t bufferLen, uint16_t& serializedLen) {
    if (!buffer || bufferLen == 0) {
        return false;
    }
    
    uint16_t offset = 0;
    
    // Serialize header
    TransactionHeader header = message.getHeader();
    if (!serializeHeader(buffer, offset, bufferLen, header)) {
        return false;
    }
    
    // Serialize account keys
    // Get account keys one by one (friend access allows direct access, but we'll use safer method)
    uint8_t accountKeysBuffer[MAX_ACCOUNTS][SOLDUINO_PUBKEY_SIZE];
    uint8_t accountCount = message.getAccountCount();
    for (uint8_t i = 0; i < accountCount; i++) {
        if (!message.getAccount(i, accountKeysBuffer[i])) {
            return false;
        }
    }
    
    if (!serializeAccountKeys(buffer, offset, bufferLen, accountKeysBuffer, accountCount)) {
        return false;
    }
    
    // Serialize recent blockhash
    uint8_t blockhash[BLOCKHASH_SIZE];
    if (!message.getRecentBlockhash(blockhash)) {
        return false;
    }
    if (!serializeBlockhash(buffer, offset, bufferLen, blockhash)) {
        return false;
    }
    
    // Serialize instructions
    if (!serializeInstructions(buffer, offset, bufferLen, message.instructions, message.instructionCount)) {
        return false;
    }
    
    serializedLen = offset;
    return true;
}

bool TransactionSerializer::serializeTransaction(const Transaction& transaction, uint8_t* buffer, uint16_t bufferLen, uint16_t& serializedLen) {
    if (!buffer || bufferLen == 0) {
        return false;
    }
    
    uint16_t offset = 0;
    
    // Serialize signatures count as compact u16
    if (!writeCompactU16(buffer, offset, bufferLen, transaction.signatureCount)) {
        return false;
    }
    
    // Serialize each signature (64 bytes each)
    // Note: All signatures must be present and valid (non-zero) for Solana
    for (uint8_t i = 0; i < transaction.signatureCount; i++) {
        if (offset + SIGNATURE_SIZE > bufferLen) {
            return false;
        }
        // Verify signature is not all zeros (basic validation)
        bool isZero = true;
        for (uint8_t j = 0; j < SIGNATURE_SIZE; j++) {
            if (transaction.signatures[i][j] != 0) {
                isZero = false;
                break;
            }
        }
        if (isZero && transaction.signatureCount > 0) {
            // All signatures must be valid - zero signature is invalid
            return false;
        }
        memcpy(buffer + offset, transaction.signatures[i], SIGNATURE_SIZE);
        offset += SIGNATURE_SIZE;
    }
    
    // Serialize message
    uint16_t messageLen = 0;
    if (!serializeMessage(transaction.message, buffer + offset, bufferLen - offset, messageLen)) {
        return false;
    }
    offset += messageLen;

    serializedLen = offset;

    return true;
}

uint16_t TransactionSerializer::calculateMessageSize(const Message& message) {
    // Header: 3 bytes
    uint16_t size = 3;
    
    // Account keys: compact u16 count + 32 bytes per account
    size += 2; // compact u16
    size += message.accountCount * SOLDUINO_PUBKEY_SIZE;
    
    // Blockhash: 32 bytes
    size += BLOCKHASH_SIZE;
    
    // Instructions: compact u16 count + each instruction
    size += 2; // compact u16
    for (uint8_t i = 0; i < message.instructionCount; i++) {
        const CompiledInstruction& inst = message.instructions[i];
        size += 1; // programIdIndex
        size += 2; // compact u16 for account count
        size += inst.accountCount; // account indices
        size += 2; // compact u16 for data length
        size += inst.dataLength; // instruction data
    }
    
    return size;
}

uint16_t TransactionSerializer::calculateTransactionSize(const Transaction& transaction) {
    // Signatures: compact u16 count + 64 bytes per signature
    uint16_t size = 2; // compact u16
    size += transaction.signatureCount * SIGNATURE_SIZE;
    
    // Message size
    size += calculateMessageSize(transaction.message);
    
    return size;
}

bool TransactionSerializer::encodeTransaction(const Transaction& transaction, char* output, size_t outputLen) {
    if (!output || outputLen == 0) {
        return false;
    }

    // Calculate required buffer size
    uint16_t estimatedSize = calculateTransactionSize(transaction);
    uint8_t* buffer = (uint8_t*)malloc(estimatedSize + 100); // Add padding
    if (!buffer) {
        return false;
    }
    
    // Serialize transaction
    uint16_t serializedLen = 0;
    if (!serializeTransaction(transaction, buffer, estimatedSize + 100, serializedLen)) {
        free(buffer);
        return false;
    }
    
    // Encode to base64
    size_t encodedLen = Base64::encode(buffer, serializedLen, output, outputLen);
    free(buffer);
    
    return encodedLen > 0;
}

bool TransactionSerializer::encodeTransactionBase58(const Transaction& transaction, char* output, size_t outputLen) {
    if (!output || outputLen == 0) {
        return false;
    }

    // Calculate required buffer size
    uint16_t estimatedSize = calculateTransactionSize(transaction);
    uint8_t* buffer = (uint8_t*)malloc(estimatedSize + 100); // Add padding
    if (!buffer) {
        return false;
    }
    
    // Serialize transaction
    uint16_t serializedLen = 0;
    if (!serializeTransaction(transaction, buffer, estimatedSize + 100, serializedLen)) {
        free(buffer);
        return false;
    }
    
    // Encode to base58
    size_t encodedLen = base58Encode(buffer, serializedLen, output, outputLen);
    free(buffer);
    
    return encodedLen > 0;
}

// ============================================================================
// Base64 Implementation
// ============================================================================

static const char base64_chars[] = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";

size_t Base64::encode(const uint8_t* data, size_t dataLen, char* output, size_t outputLen) {
    if (!data || !output || outputLen == 0) {
        return 0;
    }
    
    size_t encodedLen = ((dataLen + 2) / 3) * 4;
    if (encodedLen + 1 > outputLen) { // +1 for null terminator
        return 0;
    }
    
    size_t i = 0, j = 0;
    for (i = 0; i < dataLen - 2; i += 3) {
        output[j++] = base64_chars[(data[i] >> 2) & 0x3F];
        output[j++] = base64_chars[((data[i] & 0x3) << 4) | ((data[i + 1] & 0xF0) >> 4)];
        output[j++] = base64_chars[((data[i + 1] & 0xF) << 2) | ((data[i + 2] & 0xC0) >> 6)];
        output[j++] = base64_chars[data[i + 2] & 0x3F];
    }
    
    if (i < dataLen) {
        output[j++] = base64_chars[(data[i] >> 2) & 0x3F];
        if (i == dataLen - 1) {
            output[j++] = base64_chars[((data[i] & 0x3) << 4)];
            output[j++] = '=';
        } else {
            output[j++] = base64_chars[((data[i] & 0x3) << 4) | ((data[i + 1] & 0xF0) >> 4)];
            output[j++] = base64_chars[((data[i + 1] & 0xF) << 2)];
        }
        output[j++] = '=';
    }
    
    output[j] = '\0';
    return j;
}

size_t Base64::decode(const char* input, uint8_t* output, size_t outputLen) {
    if (!input || !output || outputLen == 0) {
        return 0;
    }
    
    size_t inputLen = strlen(input);
    if (inputLen == 0) {
        return 0;
    }
    
    // Build character lookup table
    uint8_t charTable[256];
    memset(charTable, 0xFF, sizeof(charTable));
    for (int i = 0; i < 64; i++) {
        charTable[(uint8_t)base64_chars[i]] = i;
    }
    
    size_t i = 0, j = 0;
    uint8_t in[4];
    uint8_t inIdx = 0;
    
    while (i < inputLen && j < outputLen) {
        if (input[i] == '=') {
            // Padding encountered
            if (inIdx >= 2) {
                // We have at least 2 characters, can decode
                if (inIdx == 2) {
                    // One byte output
                    output[j++] = (in[0] << 2) | (in[1] >> 4);
                } else if (inIdx == 3) {
                    // Two bytes output
                    output[j++] = (in[0] << 2) | (in[1] >> 4);
                    if (j < outputLen) {
                        output[j++] = ((in[1] & 0xF) << 4) | (in[2] >> 2);
                    }
                }
            }
            break;
        }
        
        uint8_t c = charTable[(uint8_t)input[i]];
        if (c == 0xFF) {
            // Invalid character, skip
            i++;
            continue;
        }
        
        in[inIdx++] = c;
        
        if (inIdx == 4) {
            // Decode 4 characters to 3 bytes
            output[j++] = (in[0] << 2) | (in[1] >> 4);
            if (j < outputLen) {
                output[j++] = ((in[1] & 0xF) << 4) | (in[2] >> 2);
            }
            if (j < outputLen) {
                output[j++] = ((in[2] & 0x3) << 6) | in[3];
            }
            inIdx = 0;
        }
        
        i++;
    }
    
    // Handle remaining characters
    if (inIdx > 0 && j < outputLen) {
        if (inIdx >= 2) {
            output[j++] = (in[0] << 2) | (in[1] >> 4);
            if (inIdx >= 3 && j < outputLen) {
                output[j++] = ((in[1] & 0xF) << 4) | (in[2] >> 2);
            }
        }
    }
    
    return j;
}


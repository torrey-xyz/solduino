#include "transaction.h"
#include "serializer.h"
#include "crypto.h"
#include <string.h>

// ============================================================================
// Message Implementation
// ============================================================================

Message::Message() {
    memset(&header, 0, sizeof(header));
    memset(accountKeys, 0, sizeof(accountKeys));
    accountCount = 0;
    memset(recentBlockhash, 0, sizeof(recentBlockhash));
    memset(instructions, 0, sizeof(instructions));
    instructionCount = 0;
}

bool Message::isValidAccountIndex(uint8_t index) const {
    return index < accountCount;
}

uint8_t Message::findAccountIndex(const uint8_t* pubkey) const {
    if (!pubkey) return 255;
    
    for (uint8_t i = 0; i < accountCount; i++) {
        if (memcmp(accountKeys[i], pubkey, SOLDUINO_PUBKEY_SIZE) == 0) {
            return i;
        }
    }
    return 255; // Not found
}

int8_t Message::addAccount(const uint8_t* pubkey, bool isSigner, bool isWritable) {
    if (!pubkey || accountCount >= MAX_ACCOUNTS) {
        return -1;
    }
    
    // Check if account already exists
    uint8_t existingIndex = findAccountIndex(pubkey);
    if (existingIndex != 255) {
        return existingIndex;
    }
    
    // Solana requires accounts to be ordered as:
    // 1. Writable signers
    // 2. Readonly signers
    // 3. Writable non-signers
    // 4. Readonly non-signers
    // Find the correct insertion point
    uint8_t insertIndex = accountCount;
    
    if (isSigner) {
        if (isWritable) {
            // Writable signer: insert at position (numRequiredSignatures - numReadonlySignedAccounts)
            // This is after existing writable signers, before readonly signers
            insertIndex = header.numRequiredSignatures - header.numReadonlySignedAccounts;
        } else {
            // Readonly signer: insert at position numRequiredSignatures (after all signers)
            insertIndex = header.numRequiredSignatures;
        }
    } else {
        if (isWritable) {
            // Writable non-signer: insert after all signers, before readonly non-signers
            // Position: accountCount - numReadonlyUnsignedAccounts
            insertIndex = accountCount - header.numReadonlyUnsignedAccounts;
        } else {
            // Readonly non-signer: insert at the end
            insertIndex = accountCount;
        }
    }
    
    // Shift existing accounts to make room if needed
    if (insertIndex < accountCount) {
        for (uint8_t i = accountCount; i > insertIndex; i--) {
            memcpy(accountKeys[i], accountKeys[i-1], SOLDUINO_PUBKEY_SIZE);
        }
    }
    
    // Insert the new account
    memcpy(accountKeys[insertIndex], pubkey, SOLDUINO_PUBKEY_SIZE);
    accountCount++;
    
    // Update header based on account type
    if (isSigner) {
        header.numRequiredSignatures++;
        if (!isWritable) {
            header.numReadonlySignedAccounts++;
        }
    } else {
        if (!isWritable) {
            header.numReadonlyUnsignedAccounts++;
        }
    }
    
    return insertIndex;
}

bool Message::setRecentBlockhash(const uint8_t* blockhash) {
    if (!blockhash) return false;
    memcpy(recentBlockhash, blockhash, BLOCKHASH_SIZE);
    return true;
}

bool Message::addInstruction(const uint8_t* programId,
                             const uint8_t* accounts[],
                             uint8_t accountCount,
                             const uint8_t* data,
                             uint16_t dataLength) {
    if (!programId || instructionCount >= MAX_INSTRUCTIONS) {
        return false;
    }
    
    if (dataLength > MAX_INSTRUCTION_DATA) {
        return false;
    }
    
    CompiledInstruction& inst = instructions[instructionCount];
    
    // Find or add program ID account
    int8_t programIdIndex = findAccountIndex(programId);
    if (programIdIndex == 255) {
        // Add program ID as readonly unsigned account
        programIdIndex = addAccount(programId, false, false);
        if (programIdIndex < 0) {
            return false;
        }
    }
    inst.programIdIndex = programIdIndex;
    
    // Add instruction accounts
    inst.accountCount = 0;
    for (uint8_t i = 0; i < accountCount && inst.accountCount < MAX_ACCOUNTS; i++) {
        if (accounts[i]) {
            int8_t accountIndex = findAccountIndex(accounts[i]);
            if (accountIndex == 255) {
                // Account not found - this is an error, accounts must be added first
                return false;
            }
            inst.accountIndices[inst.accountCount++] = accountIndex;
        }
    }
    
    // Copy instruction data
    if (data && dataLength > 0) {
        memcpy(inst.data, data, dataLength);
        inst.dataLength = dataLength;
    } else {
        inst.dataLength = 0;
    }
    
    instructionCount++;
    return true;
}

bool Message::getAccount(uint8_t index, uint8_t* pubkey) const {
    if (!pubkey || !isValidAccountIndex(index)) {
        return false;
    }
    memcpy(pubkey, accountKeys[index], SOLDUINO_PUBKEY_SIZE);
    return true;
}

bool Message::getRecentBlockhash(uint8_t* blockhash) const {
    if (!blockhash) return false;
    memcpy(blockhash, recentBlockhash, BLOCKHASH_SIZE);
    return true;
}

void Message::reset() {
    memset(&header, 0, sizeof(header));
    memset(accountKeys, 0, sizeof(accountKeys));
    accountCount = 0;
    memset(recentBlockhash, 0, sizeof(recentBlockhash));
    memset(instructions, 0, sizeof(instructions));
    instructionCount = 0;
}

// ============================================================================
// Transaction Implementation
// ============================================================================

Transaction::Transaction() {
    memset(signatures, 0, sizeof(signatures));
    signatureCount = 0;
    isValid = false;
}

bool Transaction::addTransferInstruction(const uint8_t* from,
                                        const uint8_t* to,
                                        uint64_t amount) {
    if (!from || !to) {
        return false;
    }
    
    // Reset message to ensure clean state
    message.reset();
    
    // Build canonical accounts: [from(signer,writable), to(writable), system_program(readonly)]
    uint8_t systemProgramId[SOLDUINO_PUBKEY_SIZE];
    memset(systemProgramId, 0, SOLDUINO_PUBKEY_SIZE); // System program is all-zero pubkey

    // Add accounts in correct order using addAccount to ensure proper header updates
    int8_t fromIndex = message.addAccount(from, true, true);   // signer, writable
    if (fromIndex < 0) {
        return false;
    }
    
    int8_t toIndex = message.addAccount(to, false, true);      // non-signer, writable
    if (toIndex < 0) {
        return false;
    }
    
    int8_t systemIndex = message.addAccount(systemProgramId, false, false); // non-signer, readonly
    if (systemIndex < 0) {
        return false;
    }

    // Create transfer instruction data (SystemProgram):
    // u32 discriminator (little-endian) = 2, followed by u64 amount (little-endian)
    uint8_t instructionData[12];
    instructionData[0] = 2 & 0xFF;
    instructionData[1] = (2 >> 8) & 0xFF;
    instructionData[2] = (2 >> 16) & 0xFF;
    instructionData[3] = (2 >> 24) & 0xFF;
    for (int i = 0; i < 8; i++) {
        instructionData[4 + i] = (amount >> (i * 8)) & 0xFF;
    }

    // Add instruction using the proper method
    const uint8_t* accounts[] = {from, to};
    if (!message.addInstruction(systemProgramId, accounts, 2, instructionData, 12)) {
        return false;
    }

    return true;
}

bool Transaction::addInstruction(const uint8_t* programId,
                                const uint8_t* accounts[],
                                uint8_t accountCount,
                                const uint8_t* data,
                                uint16_t dataLength) {
    return message.addInstruction(programId, accounts, accountCount, data, dataLength);
}

bool Transaction::setRecentBlockhash(const uint8_t* blockhash) {
    return message.setRecentBlockhash(blockhash);
}

bool Transaction::sign(const uint8_t* privateKey, const uint8_t* publicKey) {
    if (!privateKey || !publicKey) {
        return false;
    }
    
    // Ensure the signer account is part of the message
    uint8_t signerIndex = message.findAccountIndex(publicKey);
    if (signerIndex == 255) {
        // Account doesn't exist - add it as a writable signer
        // This will add it at the correct position (beginning for writable signers)
        signerIndex = message.addAccount(publicKey, true, true);
        if (signerIndex == 255) {
            return false;
        }
    }
    
    // Critical: In Solana, the fee payer (first signer) MUST be at account index 0
    // Verify the signer is in the correct position for signature verification
    TransactionHeader header = message.getHeader();
    if (signerIndex >= header.numRequiredSignatures) {
        // Signer is not in the signer accounts section - this is invalid
        return false;
    }
    
    // Ensure the signer is at the expected position (index 0 for single signer)
    // For multi-signer transactions, signers must be at indices 0, 1, 2... in order
    if (signerIndex != 0 && signerIndex >= header.numRequiredSignatures) {
        return false;
    }
    
    // Serialize message for signing
    uint8_t messageBuffer[2048];
    uint16_t messageLen = 0;

    if (!TransactionSerializer::serializeMessage(message, messageBuffer, sizeof(messageBuffer), messageLen)) {
        return false;
    }

    // Sign the serialized message
    uint8_t signature[SIGNATURE_SIZE];
    if (!signMessage(messageBuffer, messageLen, privateKey, signature)) {
        return false;
    }
    
    // Critical: In Solana, signatures must be stored sequentially for signer accounts
    // The first numRequiredSignatures accounts are signers, and signatures must be
    // stored at signatures[0], signatures[1], etc. corresponding to account[0], account[1], etc.
    // 
    // If signerIndex is not 0, we have a problem - we can't sign for an account that's not
    // the first signer in a single-signature transaction. This should never happen if
    // accounts are added correctly.
    
    // Ensure signatureCount matches numRequiredSignatures
    signatureCount = header.numRequiredSignatures;
    if (signatureCount == 0) {
        return false;
    }
    
    // Store signature at the position matching the signer's account index
    // Note: signerIndex must be < numRequiredSignatures (we checked earlier)
    if (signerIndex < signatureCount) {
        // Zero out all signatures first to ensure clean state
        memset(signatures, 0, sizeof(signatures));
        // Store the signature at the correct position
        memcpy(signatures[signerIndex], signature, SIGNATURE_SIZE);
    } else {
        return false;
    }
    
    isValid = true;
    return true;
}

bool Transaction::signMultiple(const uint8_t* privateKeys[],
                               const uint8_t* publicKeys[],
                               uint8_t count) {
    if (!privateKeys || !publicKeys || count == 0) {
        return false;
    }
    
    bool allSuccess = true;
    for (uint8_t i = 0; i < count; i++) {
        if (!sign(privateKeys[i], publicKeys[i])) {
            allSuccess = false;
        }
    }
    
    return allSuccess;
}

bool Transaction::getSignature(uint8_t index, uint8_t* signature) const {
    if (!signature || index >= signatureCount) {
        return false;
    }
    memcpy(signature, signatures[index], SIGNATURE_SIZE);
    return true;
}

void Transaction::reset() {
    memset(signatures, 0, sizeof(signatures));
    signatureCount = 0;
    message.reset();
    isValid = false;
}


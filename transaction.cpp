#include "transaction.h"
#include "serializer.h"
#include "crypto.h"
#include "keypair.h"
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
    
    // NOTE: Does NOT call message.reset() so multiple instructions can be
    // composed in a single transaction. Call tx.reset() explicitly if needed.
    
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

bool Transaction::add(const Instruction& instruction) {
    // Validate instruction has a program set
    if (!instruction.hasProgram()) {
        return false;
    }
    
    // 1. Register all account keys from the instruction into the message.
    //    addAccount handles deduplication and correct ordering automatically.
    for (uint8_t i = 0; i < instruction.getKeyCount(); i++) {
        const AccountMeta* meta = instruction.getKey(i);
        if (!meta) return false;
        
        int8_t idx = message.addAccount(meta->pubkey, meta->isSigner, meta->isWritable);
        if (idx < 0) {
            return false;
        }
    }
    
    // 2. Build a pointer array of account pubkeys for the legacy addInstruction path
    const uint8_t* accountPtrs[MAX_IX_ACCOUNTS];
    for (uint8_t i = 0; i < instruction.getKeyCount(); i++) {
        const AccountMeta* meta = instruction.getKey(i);
        accountPtrs[i] = meta->pubkey;
    }
    
    // 3. Delegate to the existing Message::addInstruction which handles
    //    program ID registration and CompiledInstruction creation.
    return message.addInstruction(
        instruction.getProgram(),
        accountPtrs,
        instruction.getKeyCount(),
        instruction.getData(),
        instruction.getDataLength()
    );
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
    // Low-level single-signer entrypoint. Clears any previously placed
    // signatures, then writes this signer's signature at its slot.
    // For multi-signer flows, use `signMultiple(...)` or build up partial
    // signatures via `partialSign(const Keypair&)`.
    memset(signatures, 0, sizeof(signatures));
    signatureCount = 0;
    isValid = false;
    return partialSignRaw(privateKey, publicKey);
}

bool Transaction::partialSignRaw(const uint8_t* privateKey, const uint8_t* publicKey) {
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
    
    // Critical: In Solana, the fee payer (first signer) MUST be at account index 0.
    // Signers must live in the first `numRequiredSignatures` account slots, in order.
    TransactionHeader header = message.getHeader();
    if (signerIndex >= header.numRequiredSignatures) {
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
    
    // signatureCount tracks the number of signer slots the serializer should emit.
    // It must match numRequiredSignatures so every signer slot is written out
    // (unsigned slots remain zeroed until their keypair calls partialSign).
    if (header.numRequiredSignatures == 0) {
        return false;
    }
    if (signatureCount < header.numRequiredSignatures) {
        signatureCount = header.numRequiredSignatures;
    }
    
    // Place this signer's signature at its account index. Other slots are left
    // untouched so previously placed partial signatures survive.
    memcpy(signatures[signerIndex], signature, SIGNATURE_SIZE);
    
    isValid = true;
    return true;
}

bool Transaction::sign(const Keypair& signer) {
    if (!signer.isInitialized()) {
        return false;
    }
    
    uint8_t priv[SOLDUINO_SECRETKEY_SIZE];
    uint8_t pub[SOLDUINO_PUBKEY_SIZE];
    bool ok = signer.getPrivateKey(priv) && signer.getPublicKey(pub);
    if (ok) {
        ok = this->sign(priv, pub);
    }
    // Scrub private key from stack regardless of outcome.
    memset(priv, 0, sizeof(priv));
    return ok;
}

bool Transaction::partialSign(const Keypair& signer) {
    if (!signer.isInitialized()) {
        return false;
    }
    
    uint8_t priv[SOLDUINO_SECRETKEY_SIZE];
    uint8_t pub[SOLDUINO_PUBKEY_SIZE];
    bool ok = signer.getPrivateKey(priv) && signer.getPublicKey(pub);
    if (ok) {
        ok = this->partialSignRaw(priv, pub);
    }
    memset(priv, 0, sizeof(priv));
    return ok;
}

bool Transaction::sign(const Keypair* const signers[], uint8_t count) {
    if (!signers || count == 0) {
        return false;
    }
    
    // Start from a clean slate so a multi-signer sign() matches the
    // single-signer sign() semantics.
    memset(signatures, 0, sizeof(signatures));
    signatureCount = 0;
    isValid = false;
    
    bool allSuccess = true;
    for (uint8_t i = 0; i < count; i++) {
        if (!signers[i] || !partialSign(*signers[i])) {
            allSuccess = false;
        }
    }
    return allSuccess;
}

bool Transaction::signMultiple(const uint8_t* privateKeys[],
                               const uint8_t* publicKeys[],
                               uint8_t count) {
    if (!privateKeys || !publicKeys || count == 0) {
        return false;
    }
    
    // Clear once up front, then accumulate partial signatures. The previous
    // implementation called sign() in a loop, which wiped each prior
    // signature and left only the last signer's signature on the transaction.
    memset(signatures, 0, sizeof(signatures));
    signatureCount = 0;
    isValid = false;
    
    bool allSuccess = true;
    for (uint8_t i = 0; i < count; i++) {
        if (!partialSignRaw(privateKeys[i], publicKeys[i])) {
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


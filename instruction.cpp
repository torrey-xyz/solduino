#include "instruction.h"
#include <string.h>

// ============================================================================
// Instruction Implementation
// ============================================================================

Instruction::Instruction() {
    reset();
}

// ---- Program ID ------------------------------------------------------------

bool Instruction::setProgram(const uint8_t* programId) {
    if (!programId) return false;
    memcpy(programId_, programId, SOLDUINO_PUBKEY_SIZE);
    hasProgram_ = true;
    return true;
}

bool Instruction::setProgramBase58(const char* base58Address) {
    if (!base58Address) return false;
    uint8_t decoded[SOLDUINO_PUBKEY_SIZE];
    size_t len = base58Decode(base58Address, decoded, sizeof(decoded));
    if (len != SOLDUINO_PUBKEY_SIZE) return false;
    return setProgram(decoded);
}

// ---- Account Keys ----------------------------------------------------------

bool Instruction::addKey(const uint8_t* pubkey, bool isSigner, bool isWritable) {
    if (!pubkey || keyCount_ >= MAX_IX_ACCOUNTS) return false;
    memcpy(keys_[keyCount_].pubkey, pubkey, SOLDUINO_PUBKEY_SIZE);
    keys_[keyCount_].isSigner = isSigner;
    keys_[keyCount_].isWritable = isWritable;
    keyCount_++;
    return true;
}

bool Instruction::addKeyBase58(const char* base58Address, bool isSigner, bool isWritable) {
    if (!base58Address) return false;
    uint8_t decoded[SOLDUINO_PUBKEY_SIZE];
    size_t len = base58Decode(base58Address, decoded, sizeof(decoded));
    if (len != SOLDUINO_PUBKEY_SIZE) return false;
    return addKey(decoded, isSigner, isWritable);
}

// ---- Data Encoding ---------------------------------------------------------

bool Instruction::writeU8(uint8_t value) {
    if (dataLen_ + 1 > MAX_IX_DATA) return false;
    data_[dataLen_++] = value;
    return true;
}

bool Instruction::writeU16LE(uint16_t value) {
    if (dataLen_ + 2 > MAX_IX_DATA) return false;
    data_[dataLen_++] = (uint8_t)(value & 0xFF);
    data_[dataLen_++] = (uint8_t)((value >> 8) & 0xFF);
    return true;
}

bool Instruction::writeU32LE(uint32_t value) {
    if (dataLen_ + 4 > MAX_IX_DATA) return false;
    data_[dataLen_++] = (uint8_t)(value & 0xFF);
    data_[dataLen_++] = (uint8_t)((value >> 8) & 0xFF);
    data_[dataLen_++] = (uint8_t)((value >> 16) & 0xFF);
    data_[dataLen_++] = (uint8_t)((value >> 24) & 0xFF);
    return true;
}

bool Instruction::writeU64LE(uint64_t value) {
    if (dataLen_ + 8 > MAX_IX_DATA) return false;
    for (int i = 0; i < 8; i++) {
        data_[dataLen_++] = (uint8_t)((value >> (i * 8)) & 0xFF);
    }
    return true;
}

bool Instruction::writeI64LE(int64_t value) {
    // Signed and unsigned have the same bit representation in two's complement
    return writeU64LE((uint64_t)value);
}

bool Instruction::writeBool(bool value) {
    return writeU8(value ? 1 : 0);
}

bool Instruction::writeBytes(const uint8_t* bytes, uint16_t len) {
    if (!bytes || dataLen_ + len > MAX_IX_DATA) return false;
    memcpy(data_ + dataLen_, bytes, len);
    dataLen_ += len;
    return true;
}

bool Instruction::writePubkey(const uint8_t* pubkey) {
    if (!pubkey) return false;
    return writeBytes(pubkey, SOLDUINO_PUBKEY_SIZE);
}

// ---- Reset -----------------------------------------------------------------

void Instruction::resetData() {
    memset(data_, 0, sizeof(data_));
    dataLen_ = 0;
}

void Instruction::reset() {
    memset(programId_, 0, sizeof(programId_));
    memset(keys_, 0, sizeof(keys_));
    keyCount_ = 0;
    memset(data_, 0, sizeof(data_));
    dataLen_ = 0;
    hasProgram_ = false;
}

// ---- Getters ---------------------------------------------------------------

const uint8_t* Instruction::getProgram() const {
    if (!hasProgram_) return nullptr;
    return programId_;
}

const AccountMeta* Instruction::getKey(uint8_t index) const {
    if (index >= keyCount_) return nullptr;
    return &keys_[index];
}

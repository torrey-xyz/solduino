#include "programs.h"
#include <string.h>
#include <sodium.h>

// ============================================================================
// Well-Known Program IDs
// ============================================================================

// System Program: 11111111111111111111111111111111 (all zeros in binary)
const uint8_t SystemProgram::PROGRAM_ID[SOLDUINO_PUBKEY_SIZE] = {0};

// SPL Token Program: TokenkegQfeZyiNwAJbNbGKPFXCWuBvf9Ss623VQ5DA
// Decoded from Base58
const uint8_t TokenProgram::PROGRAM_ID[SOLDUINO_PUBKEY_SIZE] = {
    0x06, 0xdd, 0xf6, 0xe1, 0xd7, 0x65, 0xa1, 0x93,
    0xd9, 0xcb, 0xe1, 0x46, 0xce, 0xeb, 0x79, 0xac,
    0x1c, 0xb4, 0x85, 0xed, 0x5f, 0x5b, 0x37, 0x91,
    0x3a, 0x8c, 0xf5, 0x85, 0x7e, 0xff, 0x00, 0xa9
};

// Associated Token Account Program: ATokenGPvbdGVxr1b2hvZbsiqW5xWH25efTNsLJA8knL
const uint8_t TokenProgram::ASSOCIATED_TOKEN_PROGRAM_ID[SOLDUINO_PUBKEY_SIZE] = {
    0x8c, 0x97, 0x25, 0x8f, 0x4e, 0x24, 0x89, 0xf1,
    0xbb, 0x3d, 0x10, 0x29, 0x14, 0x8e, 0x0d, 0x83,
    0x0b, 0x5a, 0x13, 0x99, 0xda, 0xff, 0x10, 0x84,
    0x04, 0x8e, 0x7b, 0xd8, 0xdb, 0xe9, 0xf8, 0x59
};

// ============================================================================
// SystemProgram Implementation
// ============================================================================

Instruction SystemProgram::transfer(const uint8_t* from,
                                    const uint8_t* to,
                                    uint64_t lamports) {
    Instruction ix;
    if (!from || !to) return ix;

    ix.setProgram(PROGRAM_ID);
    ix.addKey(from, true, true);    // signer, writable (fee payer / source)
    ix.addKey(to, false, true);     // writable (destination)

    // SystemInstruction::Transfer discriminator = 2 (u32 LE)
    ix.writeU32LE(2);
    // Amount in lamports (u64 LE)
    ix.writeU64LE(lamports);

    return ix;
}

Instruction SystemProgram::createAccount(const uint8_t* payer,
                                         const uint8_t* newAccount,
                                         uint64_t lamports,
                                         uint64_t space,
                                         const uint8_t* owner) {
    Instruction ix;
    if (!payer || !newAccount || !owner) return ix;

    ix.setProgram(PROGRAM_ID);
    ix.addKey(payer, true, true);       // signer, writable (funding account)
    ix.addKey(newAccount, true, true);  // signer, writable (new account)

    // SystemInstruction::CreateAccount discriminator = 0 (u32 LE)
    ix.writeU32LE(0);
    // Lamports to transfer (u64 LE)
    ix.writeU64LE(lamports);
    // Space in bytes (u64 LE)
    ix.writeU64LE(space);
    // Owner program ID (32 bytes)
    ix.writePubkey(owner);

    return ix;
}

Instruction SystemProgram::assign(const uint8_t* account,
                                  const uint8_t* owner) {
    Instruction ix;
    if (!account || !owner) return ix;

    ix.setProgram(PROGRAM_ID);
    ix.addKey(account, true, true);  // signer, writable

    // SystemInstruction::Assign discriminator = 1 (u32 LE)
    ix.writeU32LE(1);
    // New owner program ID (32 bytes)
    ix.writePubkey(owner);

    return ix;
}

Instruction SystemProgram::allocate(const uint8_t* account, uint64_t space) {
    Instruction ix;
    if (!account) return ix;

    ix.setProgram(PROGRAM_ID);
    ix.addKey(account, true, true);  // signer, writable

    // SystemInstruction::Allocate discriminator = 8 (u32 LE)
    ix.writeU32LE(8);
    // Space in bytes (u64 LE)
    ix.writeU64LE(space);

    return ix;
}

// ============================================================================
// TokenProgram Implementation
// ============================================================================

Instruction TokenProgram::transfer(const uint8_t* source,
                                   const uint8_t* dest,
                                   const uint8_t* authority,
                                   uint64_t amount) {
    Instruction ix;
    if (!source || !dest || !authority) return ix;

    ix.setProgram(PROGRAM_ID);
    ix.addKey(source, false, true);     // writable (source token account)
    ix.addKey(dest, false, true);       // writable (destination token account)
    ix.addKey(authority, true, false);  // signer (owner/delegate)

    // TokenInstruction::Transfer discriminator = 3 (single byte)
    ix.writeU8(3);
    // Amount (u64 LE)
    ix.writeU64LE(amount);

    return ix;
}

Instruction TokenProgram::approve(const uint8_t* source,
                                  const uint8_t* delegate,
                                  const uint8_t* authority,
                                  uint64_t amount) {
    Instruction ix;
    if (!source || !delegate || !authority) return ix;

    ix.setProgram(PROGRAM_ID);
    ix.addKey(source, false, true);     // writable (source token account)
    ix.addKey(delegate, false, false);  // readonly (delegate)
    ix.addKey(authority, true, false);  // signer (owner)

    // TokenInstruction::Approve discriminator = 4 (single byte)
    ix.writeU8(4);
    // Amount (u64 LE)
    ix.writeU64LE(amount);

    return ix;
}

Instruction TokenProgram::initializeAccount(const uint8_t* account,
                                            const uint8_t* mint,
                                            const uint8_t* owner) {
    Instruction ix;
    if (!account || !mint || !owner) return ix;

    // Rent sysvar: SysvarRent111111111111111111111111111111111
    static const uint8_t RENT_SYSVAR[SOLDUINO_PUBKEY_SIZE] = {
        0x06, 0xa7, 0xd5, 0x17, 0x18, 0x7b, 0xd1, 0x6c,
        0xdd, 0x36, 0xdc, 0xf3, 0x05, 0xe6, 0x8d, 0x16,
        0xc0, 0x01, 0x6c, 0x68, 0x05, 0x5a, 0x2d, 0x69,
        0x10, 0x05, 0x10, 0x03, 0x35, 0x47, 0xb5, 0x6b
    };

    ix.setProgram(PROGRAM_ID);
    ix.addKey(account, false, true);    // writable (token account to init)
    ix.addKey(mint, false, false);      // readonly (mint)
    ix.addKey(owner, false, false);     // readonly (owner)
    ix.addKey(RENT_SYSVAR, false, false); // readonly (rent sysvar)

    // TokenInstruction::InitializeAccount discriminator = 1 (single byte)
    ix.writeU8(1);

    return ix;
}

// ============================================================================
// PDA Derivation Implementation
// ============================================================================

// The string appended to the hash input per the Solana PDA spec
static const char PDA_MARKER[] = "ProgramDerivedAddress";
static const size_t PDA_MARKER_LEN = 21; // strlen("ProgramDerivedAddress")

bool createProgramAddress(const uint8_t* seeds[],
                          const size_t seedLens[],
                          uint8_t seedCount,
                          const uint8_t* programId,
                          uint8_t bump,
                          uint8_t* outAddress) {
    if (!programId || !outAddress) return false;
    if (seedCount > MAX_PDA_SEEDS) return false;
    if (sodium_init() < 0) return false;

    // Build the hash input: [seeds... | [bump] | programId | "ProgramDerivedAddress"]
    crypto_hash_sha256_state state;
    crypto_hash_sha256_init(&state);

    // Hash each seed
    for (uint8_t i = 0; i < seedCount; i++) {
        if (seeds[i] && seedLens[i] > 0) {
            crypto_hash_sha256_update(&state, seeds[i], seedLens[i]);
        }
    }

    // Hash the bump seed as a single byte
    uint8_t bumpByte = bump;
    crypto_hash_sha256_update(&state, &bumpByte, 1);

    // Hash the program ID (32 bytes)
    crypto_hash_sha256_update(&state, programId, SOLDUINO_PUBKEY_SIZE);

    // Hash the marker string "ProgramDerivedAddress"
    crypto_hash_sha256_update(&state, (const uint8_t*)PDA_MARKER, PDA_MARKER_LEN);

    // Finalize the hash
    uint8_t hash[32];
    crypto_hash_sha256_final(&state, hash);

    // A valid PDA must NOT be on the Ed25519 curve.
    // crypto_core_ed25519_is_valid_point returns 1 if the point IS on the curve.
    if (crypto_core_ed25519_is_valid_point(hash) == 1) {
        // Point is on curve -- not a valid PDA for this bump
        return false;
    }

    // Valid PDA found
    memcpy(outAddress, hash, SOLDUINO_PUBKEY_SIZE);
    return true;
}

bool findProgramAddress(const uint8_t* seeds[],
                        const size_t seedLens[],
                        uint8_t seedCount,
                        const uint8_t* programId,
                        uint8_t* outAddress,
                        uint8_t* outBump) {
    if (!programId || !outAddress || !outBump) return false;
    if (seedCount > MAX_PDA_SEEDS) return false;

    // Try bump seeds from 255 down to 0
    for (int bump = 255; bump >= 0; bump--) {
        if (createProgramAddress(seeds, seedLens, seedCount, programId,
                                 (uint8_t)bump, outAddress)) {
            *outBump = (uint8_t)bump;
            return true;
        }
    }

    // No valid PDA found (extremely rare)
    return false;
}

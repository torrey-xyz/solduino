#ifndef SOLDUINO_PROGRAMS_H
#define SOLDUINO_PROGRAMS_H

#include <Arduino.h>
#include <stdint.h>
#include "crypto.h"
#include "instruction.h"

// ============================================================================
// Solduino Programs Module
// ============================================================================
// Pre-built helpers for well-known Solana programs and PDA derivation:
// - SystemProgram  (11111111111111111111111111111111)
// - TokenProgram   (TokenkegQfeZyiNwAJbNbGKPFXCWuBvf9Ss623VQ5DA)
// - AssociatedTokenProgram
// - findProgramAddress() for PDA derivation
// ============================================================================

// Maximum number of seeds for PDA derivation
#ifndef MAX_PDA_SEEDS
#define MAX_PDA_SEEDS 16
#endif

// ============================================================================
// System Program
// ============================================================================

/**
 * SystemProgram helpers
 * 
 * Provides static methods that return ready-to-use Instruction objects
 * for the Solana System Program (address: 11111111111111111111111111111111).
 *
 * Usage:
 *   Transaction tx;
 *   tx.add(SystemProgram::transfer(from, to, lamports));
 */
class SystemProgram {
public:
    /** System Program ID (all zeros) */
    static const uint8_t PROGRAM_ID[SOLDUINO_PUBKEY_SIZE];

    /**
     * Create a SOL transfer instruction.
     * @param from  Fee payer / source (signer, writable)
     * @param to    Destination (writable)
     * @param lamports Amount to transfer
     * @return Instruction ready to add to a Transaction
     */
    static Instruction transfer(const uint8_t* from, const uint8_t* to, uint64_t lamports);

    /**
     * Create Account instruction.
     * @param payer      Funding account (signer, writable)
     * @param newAccount New account to create (signer, writable)
     * @param lamports   Lamports to transfer to new account
     * @param space      Space in bytes for account data
     * @param owner      Program that will own the new account
     * @return Instruction ready to add to a Transaction
     */
    static Instruction createAccount(const uint8_t* payer,
                                     const uint8_t* newAccount,
                                     uint64_t lamports,
                                     uint64_t space,
                                     const uint8_t* owner);

    /**
     * Assign instruction -- assign account to a program.
     * @param account Account to assign (signer, writable)
     * @param owner   Program to assign ownership to
     * @return Instruction ready to add to a Transaction
     */
    static Instruction assign(const uint8_t* account, const uint8_t* owner);

    /**
     * Allocate instruction -- allocate space in an account.
     * @param account Account to allocate space for (signer, writable)
     * @param space   Number of bytes to allocate
     * @return Instruction ready to add to a Transaction
     */
    static Instruction allocate(const uint8_t* account, uint64_t space);
};

// ============================================================================
// SPL Token Program
// ============================================================================

/**
 * TokenProgram helpers
 * 
 * Provides static methods that return ready-to-use Instruction objects
 * for the SPL Token Program (TokenkegQfeZyiNwAJbNbGKPFXCWuBvf9Ss623VQ5DA).
 *
 * Usage:
 *   Transaction tx;
 *   tx.add(TokenProgram::transfer(source, dest, authority, amount));
 */
class TokenProgram {
public:
    /** SPL Token Program ID */
    static const uint8_t PROGRAM_ID[SOLDUINO_PUBKEY_SIZE];

    /** Associated Token Account Program ID */
    static const uint8_t ASSOCIATED_TOKEN_PROGRAM_ID[SOLDUINO_PUBKEY_SIZE];

    /**
     * SPL Token Transfer instruction.
     * @param source    Source token account (writable)
     * @param dest      Destination token account (writable)
     * @param authority Token owner / delegate (signer)
     * @param amount    Amount of tokens (in smallest unit)
     * @return Instruction ready to add to a Transaction
     */
    static Instruction transfer(const uint8_t* source,
                                const uint8_t* dest,
                                const uint8_t* authority,
                                uint64_t amount);

    /**
     * SPL Token Approve instruction.
     * @param source    Source token account (writable)
     * @param delegate  Delegate account
     * @param authority Token owner (signer)
     * @param amount    Maximum amount delegate can transfer
     * @return Instruction ready to add to a Transaction
     */
    static Instruction approve(const uint8_t* source,
                               const uint8_t* delegate,
                               const uint8_t* authority,
                               uint64_t amount);

    /**
     * SPL Token InitializeAccount instruction.
     * Initializes a new token account for a given mint and owner.
     * @param account Token account to initialize (writable)
     * @param mint    Mint for the token type
     * @param owner   Owner of the token account
     * @return Instruction ready to add to a Transaction
     */
    static Instruction initializeAccount(const uint8_t* account,
                                         const uint8_t* mint,
                                         const uint8_t* owner);
};

// ============================================================================
// PDA (Program Derived Address) Derivation
// ============================================================================

/**
 * Find a Program Derived Address (PDA).
 *
 * Iterates bump seeds from 255 down to 0, hashing
 * [seeds... | bump | programId | "ProgramDerivedAddress"]
 * with SHA-256 until the result is NOT on the Ed25519 curve.
 *
 * Requires libsodium with Ed25519 ristretto / core support.
 *
 * @param seeds      Array of seed byte pointers
 * @param seedLens   Array of seed lengths (one per seed)
 * @param seedCount  Number of seeds
 * @param programId  32-byte program ID
 * @param outAddress Output buffer for the 32-byte PDA (must be pre-allocated)
 * @param outBump    Output bump seed value (1 byte)
 * @return true if a valid PDA was found, false otherwise
 */
bool findProgramAddress(const uint8_t* seeds[],
                        const size_t seedLens[],
                        uint8_t seedCount,
                        const uint8_t* programId,
                        uint8_t* outAddress,
                        uint8_t* outBump);

/**
 * Create a Program Derived Address with a known bump seed.
 * Faster than findProgramAddress when you already know the bump.
 *
 * @param seeds      Array of seed byte pointers
 * @param seedLens   Array of seed lengths (one per seed)
 * @param seedCount  Number of seeds
 * @param programId  32-byte program ID
 * @param bump       Known bump seed
 * @param outAddress Output buffer for the 32-byte PDA
 * @return true if the resulting address is a valid PDA (off-curve)
 */
bool createProgramAddress(const uint8_t* seeds[],
                          const size_t seedLens[],
                          uint8_t seedCount,
                          const uint8_t* programId,
                          uint8_t bump,
                          uint8_t* outAddress);

#endif // SOLDUINO_PROGRAMS_H

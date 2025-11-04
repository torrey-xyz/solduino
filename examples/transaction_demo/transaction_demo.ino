/**
 * Solduino Generic Transaction Demo (Guide)
 *
 * This file is intentionally mostly COMMENTARY.
 * It explains the exact steps to perform *any* Solana transaction with Solduino,
 * and how to view the relevant transaction information.
 *
 * For complete working examples, see:
 * - examples/transaction_demo/airdrop_demo.ino  (requestAirdrop via RPC)
 * - examples/transaction_demo/transfer_demo.ino (build/sign/send a SystemProgram transfer)
 *
 * ---------------------------------------------------------------------------
 * A) The 9-step recipe to perform ANY transaction
 * ---------------------------------------------------------------------------
 *
 * 1) Connect to WiFi
 *    - Required for RPC calls.
 *
 * 2) Create an RpcClient for your cluster (localnet/devnet/testnet/mainnet)
 *    - Example (localnet): start validator with:
 *        solana-test-validator --bind-address 0.0.0.0
 *      then set RPC endpoint to your computer IP, e.g. "http://192.168.1.100:8899".
 *
 * 3) Prepare signer keypair(s)
 *    - For most transactions you need at least one signer: the fee payer.
 *    - Use Keypair.generate() (demo) or import keys for real use.
 *
 * 4) Build your transaction instructions
 *    You have two options:
 *
 *    (a) Use convenience helpers
 *        - Example: Transaction::addTransferInstruction(from, to, lamports)
 *          (see transfer_demo.ino)
 *
 *    (b) Build a custom instruction (for *any* program)
 *        - You must:
 *          - Add ALL accounts used by your instruction into the message first
 *          - Then call tx.addInstruction(programId, accounts[], accountCount, data, dataLen)
 *
 *        IMPORTANT (Solduino internal rule):
 *        - Message::addInstruction() will FAIL if an account in accounts[] was not previously
 *          added to the message with addAccount().
 *        - It will auto-add the programId as a readonly, non-signer account if missing.
 *
 *        Pseudocode:
 *
 *        Transaction tx;
 *        Message& msg = tx.getMessage();
 *
 *        // Fee payer MUST be a signer. For simple txs, keep it account index 0.
 *        msg.addAccount(feePayerPubkey, /*isSigner*/true,  /*isWritable*/true);
 *
 *        // Add every account your instruction touches (signer/writable flags matter!)
 *        msg.addAccount(someWritableAcct, /*isSigner*/false, /*isWritable*/true);
 *        msg.addAccount(someReadonlyAcct, /*isSigner*/false, /*isWritable*/false);
 *
 *        // Program ID (optional to add; addInstruction can auto-add it readonly)
 *        // msg.addAccount(programId, false, false);
 *
 *        const uint8_t* ixAccounts[] = { feePayerPubkey, someWritableAcct, someReadonlyAcct };
 *        uint8_t ixData[] = { /* program-specific instruction data */ };
 *        tx.addInstruction(programId, ixAccounts, 3, ixData, sizeof(ixData));
 *
 * 5) Fetch a *fresh* recent blockhash RIGHT before signing
 *    - This matters a lot: old blockhash => "Blockhash not found".
 *
 *    uint8_t blockhash[BLOCKHASH_SIZE];
 *    rpcClient.getLatestBlockhashBytes(blockhash);
 *    tx.setRecentBlockhash(blockhash);
 *
 * 6) Sign
 *    - For single-signer transactions: sign with the fee payer.
 *    - Note: Solana requires the first signer to be the fee payer.
 *
 *    tx.sign(feePayerPrivateKey64, feePayerPubkey32);
 *
 * 7) Serialize for RPC submission
 *    - Solduino provides Base64 and Base58 encoders:
 *      - TransactionSerializer::encodeTransaction(...)        => base64 string
 *      - TransactionSerializer::encodeTransactionBase58(...)  => base58 string
 *
 *    char txB58[2048];
 *    TransactionSerializer::encodeTransactionBase58(tx, txB58, sizeof(txB58));
 *
 * 8) Send via RPC
 *    - IMPORTANT: sendTransactionBase58() returns a JSON string.
 *      You must parse it to extract the signature.
 *
 *    String sendRespJson = rpcClient.sendTransactionBase58(txB58);
 *    // Parse JSON like transfer_demo.ino does:
 *    // - if sendRespJson contains {"result":"<signature>"} => success
 *    // - else {"error":...} => failure
 *
 * 9) Confirm + inspect results
 *    - Minimal confirmation in this library:
 *      bool rpcClient.getTransaction(signature, TransactionResponse& tx)
 *      and TransactionResponse contains:
 *        - tx.signature
 *        - tx.slot
 *        - tx.status   (meta.status)
 *        - tx.error    (meta.err if present)
 *
 *    The transfer demo uses this style:
 *      - Poll getTransaction(sig, tx) until it returns true (or timeout)
 *
 * ---------------------------------------------------------------------------
 * B) How to see the "relevant info" for your transaction
 * ---------------------------------------------------------------------------
 *
 * 1) Immediate info (on-device)
 *    - After send: print the signature (Base58)
 *    - After confirm (getTransaction): print:
 *      - slot
 *      - status
 *      - error (if any)
 *
 * 2) Full details (logs, balance changes, inner instructions)
 *    Solduino's TransactionResponse is intentionally small.
 *    To inspect *full* runtime details, use ONE of these approaches:
 *
 *    (a) Use the signature on desktop (best UX)
 *        - Solana CLI:
 *          - localnet: solana confirm -v <SIG> --url http://<YOUR_IP>:8899
 *          - devnet:   solana confirm -v <SIG> --url https://api.devnet.solana.com
 *
 *    (b) Use Explorer (devnet/testnet/mainnet)
 *        - devnet:  https://explorer.solana.com/tx/<SIG>?cluster=devnet
 *        - testnet: https://explorer.solana.com/tx/<SIG>?cluster=testnet
 *        - mainnet: https://explorer.solana.com/tx/<SIG>
 *      (Localnet generally has no public explorer.)
 *
 *    (c) Pull full JSON from RPC inside Arduino code
 *        - Use callRpc("getTransaction", params) and print / parse JSON.
 *        - Example params (more verbose, includes jsonParsed):
 *
 *          String params = "[\"" + sig + "\", {\"encoding\":\"jsonParsed\", \"maxSupportedTransactionVersion\":0}]";
 *          String fullJson = rpcClient.callRpc("getTransaction", params);
 *          Serial.println(fullJson);
 *
 *        Then look for fields like:
 *          result.meta.logMessages
 *          result.meta.preBalances / postBalances
 *          result.transaction.message.accountKeys
 *
 * ---------------------------------------------------------------------------
 * C) Where to copy/paste real code from
 * ---------------------------------------------------------------------------
 * - For RPC airdrop flow (signature + balance check):
 *     examples/transaction_demo/airdrop_demo.ino
 *
 * - For end-to-end "build + sign + serialize + send + confirm":
 *     examples/transaction_demo/transfer_demo.ino
 */

// Minimal stub so this example compiles as an Arduino sketch.
// (All actionable instructions are in the big comment above.)

void setup() {
  Serial.begin(115200);
  delay(1000);
  Serial.println("See the top comment in transaction_demo.ino for the generic transaction guide.");
}

void loop() {
  delay(1000);
}

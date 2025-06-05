#ifndef TWEETNACL_H
#define TWEETNACL_H

#ifdef __cplusplus
extern "C" {
#endif

// Ed25519 constants
#define crypto_sign_BYTES 64
#define crypto_sign_PUBLICKEYBYTES 32
#define crypto_sign_SECRETKEYBYTES 64
#define crypto_sign_SEEDBYTES 32

// SHA-512 constants
#define SHA512_SIZE 64

// Ed25519 functions
int crypto_sign_keypair(unsigned char *pk, unsigned char *sk);
int crypto_sign_detached(unsigned char *sig, unsigned long long *siglen,
                        const unsigned char *m, unsigned long long mlen,
                        const unsigned char *sk);
int crypto_sign_verify_detached(const unsigned char *sig,
                               const unsigned char *m,
                               unsigned long long mlen,
                               const unsigned char *pk);

#ifdef __cplusplus
}
#endif

#endif // TWEETNACL_H 
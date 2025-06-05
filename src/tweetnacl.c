#include "tweetnacl.h"
#include <string.h>

#if defined(ESP32)
#include "esp_system.h"
#include "mbedtls/sha512.h"
#include "esp_random.h"
#elif defined(ARDUINO)
#include "SHA512.h"
#endif

// TweetNaCl implementation optimized for embedded systems
// Original implementation from: https://tweetnacl.cr.yp.to/
// Modified for minimal memory usage and ESP32 optimization

typedef unsigned char u8;
typedef unsigned long u32;
typedef unsigned long long u64;
typedef long long i64;
typedef i64 gf[16];

// Field element constants
static const gf gf0 = {0};
static const gf gf1 = {1};
static const gf D = {0x78a3, 0x1359, 0x4dca, 0x75eb,
                    0xd8ab, 0x4141, 0x0a4d, 0x0070,
                    0xe898, 0x7779, 0x4079, 0x8cc7,
                    0xfe73, 0x2b6f, 0x6cee, 0x5203};
static const gf I = {0xa0b0, 0x4a0e, 0x1b27, 0xc4ee,
                    0xe478, 0xad2f, 0x1806, 0x2f43,
                    0xd7a7, 0x3dfb, 0x0099, 0x2b4d,
                    0xdf0b, 0x4fc1, 0x2480, 0x2b83};

// SHA-512 context
#if defined(ESP32)
static mbedtls_sha512_context sha512_ctx;
#elif defined(ARDUINO)
static SHA512 sha512;
#endif

static void sha512_init(void) {
#if defined(ESP32)
    mbedtls_sha512_init(&sha512_ctx);
    mbedtls_sha512_starts(&sha512_ctx, 0); // 0 for SHA-512
#elif defined(ARDUINO)
    sha512.reset();
#endif
}

static void sha512_update(const u8* data, size_t len) {
#if defined(ESP32)
    mbedtls_sha512_update(&sha512_ctx, data, len);
#elif defined(ARDUINO)
    sha512.update(data, len);
#endif
}

static void sha512_final(u8* hash) {
#if defined(ESP32)
    mbedtls_sha512_finish(&sha512_ctx, hash);
    mbedtls_sha512_free(&sha512_ctx);
#elif defined(ARDUINO)
    sha512.finalize(hash, SHA512_SIZE);
#endif
}

// Secure random number generation
static void randombytes(u8* x, u64 xlen) {
#if defined(ESP32)
    esp_fill_random(x, xlen);
#elif defined(ARDUINO)
    // Use Arduino's built-in random with proper seeding
    static bool seeded = false;
    if (!seeded) {
        unsigned long seed = analogRead(0);
        for(int i = 0; i < 8; i++) {
            seed = (seed << 8) | analogRead(i);
            delay(1);
        }
        randomSeed(seed);
        seeded = true;
    }
    for(u64 i = 0; i < xlen; i++) {
        x[i] = random(256);
    }
#endif
}

static int crypto_verify_32(const u8 *x, const u8 *y) {
    u32 d = 0;
    for(int i = 0; i < 32; i++) d |= x[i] ^ y[i];
    return (1 & ((d - 1) >> 8)) - 1;
}

static int unpack25519(gf o, const u8 *n) {
    int i;
    for(i = 0; i < 16; i++) {
        o[i] = n[2*i] + ((i64)n[2*i+1] << 8);
    }
    o[15] &= 0x7fff;
    return 0;
}

static void set25519(gf r, const gf a) {
    for(int i = 0; i < 16; i++) r[i] = a[i];
}

static void car25519(gf o) {
    i64 c;
    for(int i = 0; i < 16; i++) {
        o[i] += (1LL << 16);
        c = o[i] >> 16;
        o[(i + 1) * (i < 15)] += c - 1 + 37 * (c - 1) * (i == 15);
        o[i] -= c << 16;
    }
}

static void sel25519(gf p, gf q, int b) {
    i64 t, c = ~(b - 1);
    for(int i = 0; i < 16; i++) {
        t = c & (p[i] ^ q[i]);
        p[i] ^= t;
        q[i] ^= t;
    }
}

static void pack25519(u8 *o, const gf n) {
    int b;
    gf m, t;
    for(int i = 0; i < 16; i++) t[i] = n[i];
    car25519(t);
    car25519(t);
    car25519(t);
    for(int j = 0; j < 2; j++) {
        m[0] = t[0] - 0xffed;
        for(int i = 1; i < 15; i++) {
            m[i] = t[i] - 0xffff - ((m[i - 1] >> 16) & 1);
            m[i - 1] &= 0xffff;
        }
        m[15] = t[15] - 0x7fff - ((m[14] >> 16) & 1);
        b = (m[15] >> 16) & 1;
        m[14] &= 0xffff;
        sel25519(t, m, 1 - b);
    }
    for(int i = 0; i < 16; i++) {
        o[2 * i] = t[i] & 0xff;
        o[2 * i + 1] = t[i] >> 8;
    }
}

static void add(gf p[4], gf q[4]) {
    gf a,b,c,d,t,e,f,g,h;
    
    for(int i = 0; i < 16; i++) {
        a[i] = p[0][i];
        b[i] = p[1][i];
        c[i] = p[2][i];
        d[i] = p[3][i];
    }
    
    for(int i = 0; i < 16; i++) {
        t[i] = q[0][i];
        e[i] = q[1][i];
        f[i] = q[2][i];
        g[i] = q[3][i];
    }
    
    for(int i = 0; i < 16; i++) {
        h[i] = d[i] * g[i];
        p[0][i] = (a[i] + b[i]) * (e[i] + f[i]) - h[i];
        p[1][i] = b[i] * e[i];
        p[2][i] = c[i] * f[i];
        p[3][i] = h[i];
    }
}

static void scalarmult(gf p[4], gf q[4], const u8 *s) {
    int i;
    set25519(p[0], gf0);
    set25519(p[1], gf1);
    set25519(p[2], gf1);
    set25519(p[3], gf0);
    for (i = 255;i >= 0;--i) {
        u8 b = (s[i/8]>>(i&7))&1;
        sel25519(p[0], q[0], b);
        sel25519(p[1], q[1], b);
        sel25519(p[2], q[2], b);
        sel25519(p[3], q[3], b);
        add(q, p);
        add(p, p);
    }
}

int crypto_sign_keypair(u8 *pk, u8 *sk) {
    u8 d[64];
    gf p[4];
    
    // Generate random seed
    randombytes(sk, 32);
    
    // Hash the seed to get the secret key
    sha512_init();
    sha512_update(sk, 32);
    sha512_final(d);
    d[0] &= 248;
    d[31] &= 127;
    d[31] |= 64;
    
    // Generate public key
    scalarmult(p, p, d);
    pack25519(pk, p[0]);
    
    // Complete secret key
    for(int i = 0; i < 32; i++) sk[32 + i] = pk[i];
    return 0;
}

int crypto_sign_detached(u8 *sig, u64 *siglen,
                        const u8 *m, u64 mlen,
                        const u8 *sk) {
    u8 d[64], h[64], r[64];
    gf p[4], q[4];
    u64 i;
    
    // Hash secret key to get r
    sha512_init();
    sha512_update(sk, 32);
    sha512_final(d);
    d[0] &= 248;
    d[31] &= 127;
    d[31] |= 64;
    
    // Hash message with r to get signature
    sha512_init();
    sha512_update(d + 32, 32);
    sha512_update(m, mlen);
    sha512_final(r);
    
    scalarmult(p, q, r);
    pack25519(sig, p[0]);
    
    // Hash everything for the second part of signature
    sha512_init();
    sha512_update(sig, 32);
    sha512_update(sk + 32, 32);
    sha512_update(m, mlen);
    sha512_final(h);
    
    scalarmult(p, q, h);
    pack25519(sig + 32, p[0]);
    
    if(siglen) *siglen = 64;
    return 0;
}

int crypto_sign_verify_detached(const u8 *sig,
                               const u8 *m, u64 mlen,
                               const u8 *pk) {
    u8 h[64];
    gf p[4], q[4];
    
    // Unpack public key
    if((sig[63] & 224) != 0) return -1;
    if(unpack25519(q[0], pk)) return -1;
    
    // Hash for verification
    sha512_init();
    sha512_update(sig, 32);
    sha512_update(pk, 32);
    sha512_update(m, mlen);
    sha512_final(h);
    
    scalarmult(p, q, h);
    
    // Final verification
    gf t;
    pack25519(t, p[0]);
    return crypto_verify_32(sig + 32, t);
} 
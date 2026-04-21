#include "../include/crypto.h"

/* XOR encryption/decryption
   Since XOR is symmetric, encrypt and decrypt are the same operation.
   key    = pointer to key bytes
   keylen = length of key
   data   = data buffer to encrypt/decrypt in-place
   len    = length of data */
void xor_crypt(unsigned char *data, unsigned int len,
               const unsigned char *key, unsigned int keylen) {
    for (unsigned int i = 0; i < len; i++) {
        data[i] ^= key[i % keylen];
    }
}

/* Simple hash function (djb2 algorithm)
   Used to verify password without storing it in plaintext */
unsigned int djb2_hash(const unsigned char *data, unsigned int len) {
    unsigned int hash = 5381;
    for (unsigned int i = 0; i < len; i++) {
        hash = ((hash << 5) + hash) + data[i];  /* hash * 33 + byte */
    }
    return hash;
}

/* Secure memory wipe — overwrite buffer with zeros
   Used to erase sensitive data from RAM after use */
void secure_wipe(unsigned char *buf, unsigned int len) {
    volatile unsigned char *p = (volatile unsigned char *)buf;
    while (len--) {
        *p++ = 0x00;
    }
}

/* Verify password against stored hash
   Returns 1 if match, 0 if fail */
int verify_password(const unsigned char *input, unsigned int input_len,
                    unsigned int stored_hash) {
    unsigned int hash = djb2_hash(input, input_len);
    return (hash == stored_hash) ? 1 : 0;
}


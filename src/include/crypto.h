#ifndef CRYPTO_H
#define CRYPTO_H

/* XOR Encryption key length */
#define KEY_LEN 16

/* Pre-computed djb2 hash of the boot password
   To change password, run: tools/genhash.py "newpassword"
   and replace this value */
#define PASSWORD_HASH 0x7c9d79a9

/* Function Prototypes */
void xor_crypt(unsigned char *data, unsigned int len,
               const unsigned char *key, unsigned int keylen);

unsigned int djb2_hash(const unsigned char *data, unsigned int len);

void secure_wipe(unsigned char *buf, unsigned int len);

int verify_password(const unsigned char *input, unsigned int input_len,
                    unsigned int stored_hash);

#endif


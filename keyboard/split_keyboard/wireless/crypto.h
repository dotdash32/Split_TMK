#ifndef CRYPTO_H
#define CRYPTO_H

#include <stdint.h>
#include "aes/aes.h"

#define AES_KEY_LEN 16 // 128 bit
#define AES_BUF_LEN 16

typedef aes128_ctx_t aes_ctx_t;

typedef enum role_t {
  MASTER_DEVICE,
  LEFT_DEVICE,
  RIGHT_DEVICE,
} role_t;

typedef struct aes_key_t {
  uint8_t key[AES_KEY_LEN];
} aes_key_t;

typedef struct aes_state_t {
  uint8_t data[AES_BUF_LEN];
  uint8_t iv[AES_BUF_LEN];
} aes_state_t;

void encrypt(aes_state_t *state, aes_ctx_t *ctx);
void decrypt(aes_state_t *state, aes_ctx_t *ctx);
void crypto_init(aes_state_t *state, aes_ctx_t *ctx, role_t role);
void xor_buf(uint8_t *buf0, uint8_t *buf1, uint8_t len);

#endif

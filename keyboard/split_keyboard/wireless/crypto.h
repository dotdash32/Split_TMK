#ifndef CRYPTO_H
#define CRYPTO_H

#include <stdint.h>

#include "aes/aes.h"

#define AES_KEY_LEN 16 // 128 bit
#define AES_BUF_LEN 16

#define ECB_PAYLOAD_LEN 9

typedef aes128_ctx_t aes_ctx_t;

enum block_mode_t {
  ECB_MODE,
  CBC_MODE
};

typedef struct cbc_state_t {
  uint8_t data[AES_BUF_LEN];
  uint8_t iv[AES_BUF_LEN];
} cbc_state_t;

typedef struct ecb_state_t {
  uint8_t payload[ECB_PAYLOAD_LEN];
  uint8_t device_id;   // used to gaurantee that packets cannot be used in a replay attack from a phony device
  uint32_t message_id; // use a packet id that is incremented for each packet received to prevent against replay attacks.
                       // On reset, this value is set to 0.
  uint16_t checksum;
} ecb_state_t; // size of this struct should be 16 bytes

void crypto_init(aes_ctx_t *ctx);

void ecb_init(ecb_state_t *state, uint8_t device_id);
void ecb_encrypt(ecb_state_t *state, aes_ctx_t *ctx, uint8_t *payload);
uint8_t ecb_decrypt(ecb_state_t *state, aes_ctx_t *ctx, uint8_t *input);

void cbc_init(cbc_state_t *state, aes_ctx_t *ctx);
void cbc_encrypt(cbc_state_t *state, aes_ctx_t *ctx);
void cbc_decrypt(cbc_state_t *state, aes_ctx_t *ctx);

void xor_buf(uint8_t *buf0, uint8_t *buf1, uint8_t len);

#endif

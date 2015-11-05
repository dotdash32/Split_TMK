/* #include <Arduino.h> */
#include <string.h>
#include <avr/eeprom.h>
#include "crypto.h"
#include "split-config.h"


void xor_buf(uint8_t *buf0, uint8_t *buf1, uint8_t len) {
  for (int i = 0; i < len; ++i) {
    buf0[i] ^= buf1[i];
  }
}

void crypto_init(aes_state_t *state, aes_ctx_t *ctx, role_t role) {
  aes_key_t key;
  eeprom_read_block(key.key, EECONFIG_AES_KEY, AES_KEY_LEN);
  aes128_init(key.key, ctx);

  if (role == MASTER_DEVICE) {
    return;
  } else {
    // AES should use an iv that is both unique and unpredictable.
    // counter => uniqueness
    // encrypt(counter) => uniqueness + unpredictability
    //
    // The bitwise inverse of the counter is used in the other half to ensure
    // uniqueness for both halves.
    uint32_t nonce_counter = eeprom_read_dword(EECONFIG_NONCE_COUNTER);
    nonce_counter++;
    eeprom_update_dword(EECONFIG_NONCE_COUNTER, nonce_counter);
    // use different nonce for left and right hands
    switch (role) {
      case LEFT_DEVICE:  nonce_counter = nonce_counter; break;
      case RIGHT_DEVICE: nonce_counter = !nonce_counter; break;
      default: break; // should be unreachable
    }
    for (int i = 0; i < AES_BUF_LEN; i+=sizeof(uint32_t)) {
      memcpy(state->iv+i, (uint8_t*)nonce_counter, sizeof(uint32_t));
    }
    aes128_enc(state->iv, ctx);
  }
}

void encrypt(aes_state_t *state, aes_ctx_t *ctx) {
  xor_buf(state->data, state->iv, AES_BUF_LEN);
  aes128_enc(state->data, ctx);
  memcpy(state->iv, state->data, AES_BUF_LEN);
}

void decrypt(aes_state_t *state, aes_ctx_t *ctx) {
  uint8_t this_cipher[AES_BUF_LEN];
  memcpy(this_cipher, state->data, AES_BUF_LEN);
  aes128_dec(state->data, ctx);
  xor_buf(state->data, state->iv, AES_BUF_LEN);
  memcpy(state->iv, this_cipher, AES_BUF_LEN);
}

/* #include <Arduino.h> */
#include <string.h>
#include <avr/eeprom.h>
#include "crypto.h"

// AES128-CBC refer to:
// AES: https://en.wikipedia.org/wiki/Advanced_Encryption_Standard
// CBC: https://en.wikipedia.org/wiki/Block_cipher_mode_of_operation#Cipher_Block_Chaining_.28CBC.29

void crypto_init(aes_state_t *state, aes_ctx_t *ctx, device_settings_t *settings) {
  uint8_t key[16];
  uint8_t nonce[16];
  const uint8_t device_num = settings->device_num;
  /* TODO: generate keys for both master and slave */
  eeprom_read_block(key, EECONFIG_AES_KEY, AES_KEY_LEN);
  aes128_init(key, ctx);

#if MASTER_DEVICE
    // do nothing, the master receives iv from slave
#else
    // AES should use an iv that is both unique and unpredictable.
    // Here we use a counter and encryption to meet these requirements:
    //   * counter => uniqueness
    //   * encrypt(counter) => uniqueness + unpredictability
    //
    // The bitwise inverse of the counter is used in the other half to ensure
    // uniqueness for both halves.

    uint32_t nonce_counter = eeprom_read_dword(EECONFIG_NONCE_COUNTER);
    nonce_counter++;
    eeprom_update_dword(EECONFIG_NONCE_COUNTER, nonce_counter);
    /* // use different nonce for left and right hands */
    switch (device_num) {
      case 0: nonce_counter = nonce_counter; break;
      case 1: nonce_counter = ~nonce_counter; break;
      default: break;
    }
    for (int i = 0; i < AES_BUF_LEN/sizeof(uint32_t); i++) {
      ((uint32_t*)state->iv)[i] = nonce_counter;
    }

    aes128_enc(state->iv, ctx);
#endif
}

void xor_buf(uint8_t *buf0, uint8_t *buf1, uint8_t len) {
  for (int i = 0; i < len; ++i) {
    buf0[i] ^= buf1[i];
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

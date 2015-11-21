#include <string.h>
#include <avr/eeprom.h>
#include "../split-config.h"
#include "crypto.h"

// AES128-CBC refer to:
// AES: https://en.wikipedia.org/wiki/Advanced_Encryption_Standard
// CBC: https://en.wikipedia.org/wiki/Block_cipher_mode_of_operation#Cipher_Block_Chaining_.28CBC.29
// EBC: https://en.wikipedia.org/wiki/Block_cipher_mode_of_operation#Electronic_Codebook_.28ECB.29

/* WARN: Currently the CBC implementation does nothing to perevent against
 * replay attacks.*/

#define MESSAGE_ID_UPDATE_INTERVAL 10000

void crypto_init(aes_ctx_t *ctx) {
  uint8_t key[16];
  eeprom_read_block(key, EECONFIG_AES_KEY, AES_KEY_LEN);
  aes128_init(key, ctx);
}

/* void cbc_init(cbc_state_t *state, aes_ctx_t *ctx) { */
/* #if DEVICE_ID==MASTER_DEVICE_ID */
/*   // do nothing, the master receives iv from slave */
/* #else */
/*   // AES-CBC should use an iv that is both unique and unpredictable. */
/*   // Here we use a counter and encryption to meet these requirements: */
/*   //   * counter => uniqueness */
/*   //   * encrypt(counter) => uniqueness + unpredictability */
/*   // */
/*   // The bitwise inverse of the counter is used in the other half to ensure */
/*   // uniqueness for both halves. */

/*   uint32_t nonce_counter = eeprom_read_dword(EECONFIG_NONCE_COUNTER); */
/*   nonce_counter++; */
/*   eeprom_update_dword(EECONFIG_NONCE_COUNTER, nonce_counter); */
/*   /1* // use different nonce for left and right hands *1/ */
/*   switch (DEVICE_ID) { */
/*     case 0: nonce_counter = nonce_counter; break; */
/*     case 1: nonce_counter = ~nonce_counter; break; */
/*     default: break; */
/*   } */
/*   for (int i = 0; i < AES_BUF_LEN/sizeof(uint32_t); i++) { */
/*     ((uint32_t*)state->iv)[i] = nonce_counter; */
/*   } */

/*   aes128_enc(state->iv, ctx); */
/* #endif */

/* } */

uint32_t* get_msg_id_addr(uint8_t device_id) {
  switch (device_id) {
    case 0: return EECONFIG_MESSAGE_ID0;
    case 1: return EECONFIG_MESSAGE_ID1;
    default: return EECONFIG_MESSAGE_ID0;
  }
}

uint16_t calc_checksum16(uint8_t *buf, uint8_t len) {
  uint16_t result = 0;
  for (int i = 0; i < len; ++i) {
    result += (buf[i]);
    result += (~buf[i]) << 8;
  }
  return result;
}

void xor_buf(uint8_t *buf0, uint8_t *buf1, uint8_t len) {
  for (int i = 0; i < len; ++i) {
    buf0[i] ^= buf1[i];
  }
}

void ecb_init(ecb_state_t *state, uint8_t device_id) {
  state->device_id = device_id;
  state->message_id = eeprom_read_dword(get_msg_id_addr(device_id));
}

void ecb_encrypt(ecb_state_t *state, aes_ctx_t *ctx, uint8_t *output) {
  state->message_id++;
  if (state->message_id % MESSAGE_ID_UPDATE_INTERVAL == 0) {
    eeprom_update_dword(get_msg_id_addr(state->device_id), state->message_id);
  }
  state->checksum = calc_checksum16((uint8_t*)state, sizeof(ecb_state_t)-sizeof(uint16_t));

  memcpy(output, state, AES_BUF_LEN);
  // encrypt the data
  aes128_enc(output, ctx);
}

uint8_t ecb_decrypt(ecb_state_t *state, aes_ctx_t *ctx, uint8_t *input) {
  ecb_state_t packet;

  memcpy(&packet, input, AES_BUF_LEN);
  aes128_dec(&packet, ctx);

  const uint16_t checksum = calc_checksum16((uint8_t*)&packet, AES_BUF_LEN-sizeof(uint16_t));

  if (checksum != packet.checksum) {
    return 1;
  }
  /* if (packet.message_id <= state->message_id) { */
  /*   return 2; */
  /* } */
  if (packet.device_id != state->device_id) {
    return 3;
  }

  /* if (state->message_id % MESSAGE_ID_UPDATE_INTERVAL == 0) { */
  /*   eeprom_update_dword(get_msg_id_addr(state->device_id), packet.message_id); */
  /* } */

  memcpy(state, &packet, sizeof(ecb_state_t));
  return 0;
}

void cbc_encrypt(cbc_state_t *state, aes_ctx_t *ctx) {
  xor_buf(state->data, state->iv, AES_BUF_LEN);
  aes128_enc(state->data, ctx);
  memcpy(state->iv, state->data, AES_BUF_LEN);
}

void cbc_decrypt(cbc_state_t *state, aes_ctx_t *ctx) {
  uint8_t this_cipher[AES_BUF_LEN];
  memcpy(this_cipher, state->data, AES_BUF_LEN);
  aes128_dec(state->data, ctx);
  xor_buf(state->data, state->iv, AES_BUF_LEN);
  memcpy(state->iv, this_cipher, AES_BUF_LEN);
}

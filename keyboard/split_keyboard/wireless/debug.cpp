#include <Arduino.h>
#include "debug.h"

#include "nrf.h"
#include "nRF24L01.h"

void print_hex(uint8_t x) {
  if (x < 16) {Serial.print("0");}
  Serial.print(x, HEX);
}

void print_hexln(uint8_t x) {
  if (x < 16) {Serial.print("0");}
  Serial.println(x, HEX);
}

void print_hex_buf(uint8_t *x, uint8_t len) {
  for(uint8_t i=0; i<len; i++) {
    if (x[i] < 16) {Serial.print("0");}
    Serial.print(x[i], HEX);
  }
  Serial.println();
}

#ifdef DEBUG
void debug_info(uint8_t verbose) {
  uint8_t temp_buf[32] = {0};
  Serial.println("--debug--");
  Serial.print("CONFIG: ");
  print_hexln(read_reg(CONFIG));
  Serial.print("STATUS: ");
  print_hexln(read_reg(NRF_STATUS));
  Serial.print("FIFO_STATUS: ");
  print_hexln(read_reg(FIFO_STATUS));
  Serial.print("TX_ADDR: ");
  read_buf(TX_ADDR, temp_buf, RF_ADDRESS_LEN);
  print_hex_buf(temp_buf, RF_ADDRESS_LEN);
  Serial.print("RX_ADDR_P0: ");
  read_buf(RX_ADDR_P0, temp_buf, RF_ADDRESS_LEN);
  print_hex_buf(temp_buf, RF_ADDRESS_LEN);
  if (verbose) {
    Serial.print("RX_ADDR_P1: ");
    read_buf(RX_ADDR_P1, temp_buf, RF_ADDRESS_LEN);
    print_hex_buf(temp_buf, RF_ADDRESS_LEN);
    Serial.print("EN_AA: ");
    print_hexln(read_reg(EN_AA));
    Serial.print("RF_CH: ");
    print_hexln(read_reg(RF_CH));
    Serial.print("RF_SETUP: ");
    print_hexln(read_reg(RF_SETUP));
    Serial.print("SETUP_RETR: ");
    print_hexln(read_reg(SETUP_RETR));
    Serial.print("SETUP_AW: ");
    print_hexln(read_reg(SETUP_AW));
    Serial.print("OBSERVE_TX: ");
    print_hexln(read_reg(OBSERVE_TX));
    Serial.print("RPD: ");
    print_hexln(read_reg(RPD));
    Serial.print("DYNPD: ");
    print_hexln(read_reg(DYNPD));
    Serial.print("RX_PW_P0: ");
    print_hexln(read_reg(RX_PW_P0));
    Serial.print("RX_PW_P1: ");
    print_hexln(read_reg(RX_PW_P1));
  }
  Serial.println("--debug--\n");
}
#else
void debug_info(void) {}
#endif


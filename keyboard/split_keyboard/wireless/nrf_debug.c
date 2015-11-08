#include "nrf_debug.h"
#include "debug.h"

#include "print.h"
#include "nrf.h"
#include "nRF24L01.h"

void print_hex_buf(uint8_t *x, uint8_t len) {
  for(uint8_t i=0; i<len; i++) {
    print_hex8(x[i]);
  }
}

void nrf_debug_info(uint8_t verbose) {
  uint8_t temp_buf[32] = {0};
  print("CONFIG: ");
  phex(read_reg(CONFIG));
  print("\tSTATUS: ");
  phex(read_reg(NRF_STATUS));
  print("\tFIFO_STATUS: ");
  phex(read_reg(FIFO_STATUS));
  println();

  print("RF_CH: ");
  phex(read_reg(RF_CH));
  print("\tRF_SETUP: ");
  phex(read_reg(RF_SETUP));
  print("\tSETUP_RETR: ");
  phex(read_reg(SETUP_RETR));
  print("\tSETUP_AW: ");
  phex(read_reg(SETUP_AW));
  print("\tOBSERVE_TX: ");
  phex(read_reg(OBSERVE_TX));
  print("\tRPD: ");
  phex(read_reg(RPD));
  println();

  print("EN_AA: ");
  phex(read_reg(EN_AA));
  print("\tDYNPD: ");
  phex(read_reg(DYNPD));
  print("\tRX_PW_P0: ");
  phex(read_reg(RX_PW_P0));
  print("\tRX_PW_P1: ");
  phex(read_reg(RX_PW_P1));
  println();


  print("TX_ADDR: ");
  read_buf(TX_ADDR, temp_buf, RF_ADDRESS_LEN);
  print_hex_buf(temp_buf, RF_ADDRESS_LEN);
  print("  RX_ADDR_P0: ");
  read_buf(RX_ADDR_P0, temp_buf, RF_ADDRESS_LEN);
  print_hex_buf(temp_buf, RF_ADDRESS_LEN);
  print("  RX_ADDR_P1: ");
  read_buf(RX_ADDR_P1, temp_buf, RF_ADDRESS_LEN);
  print_hex_buf(temp_buf, RF_ADDRESS_LEN);
  println();

}

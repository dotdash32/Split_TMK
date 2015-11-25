#include "nrf.h"
#include "../split-config.h"
#include "nRF24L01.h"
#include <avr/io.h>
#include <avr/interrupt.h>
#include <avr/eeprom.h>
#include <util/delay.h>

#define CREATE_BIT_SETTER(func, port, port_bit) \
  void func(uint8_t val) { \
    if (val) { \
      PORT##port |= (1 << port_bit);\
    } else { \
      PORT##port &= ~(1 << port_bit);\
    } \
  } \
  void ddr_##func(uint8_t val) { \
    if (val) { \
      DDR##port |= (1 << port_bit);\
    } else { \
      DDR##port &= ~(1 << port_bit);\
    } \
  } \

#if defined(__AVR_ATmega32U4__)
CREATE_BIT_SETTER(ce, B, CE);
CREATE_BIT_SETTER(csn, B, CSN);
#elif defined(__AVR_ATmega328P__)
CREATE_BIT_SETTER(ce, B, CE);
CREATE_BIT_SETTER(csn, B, CSN);
#endif

inline
void nrf_enable(uint8_t val) {
  ce(val);
}

void spi_setup(void) {
  /* TODO: change this define to an appropriate one for the promicro */
#if defined(__AVR_ATmega32U4__)
  // SS(B0) is connected to the led on the pro micro so we can reach it.
  // When setting up need to set B0 as output high, to get SPI to work.
  DDRB = 0xff;
  PORTB = 0xff;
#endif
  ddr_csn(1);
  ddr_ce(1);
  DDRB |= (0<<MISO)|(1<<MOSI)|(1<<SCK);
  csn(1);
  ce(0);

  // nrf supports 10Mbs on spi
  // setup spi with: msb, mode0, clk/2
  SPCR = (1<<SPI2X) | (1<<SPE) | (1<<MSTR);
}

uint8_t spi_transceive(uint8_t data) {
  SPDR = data;
  while(!(SPSR & (1<<SPIF)));
  return SPDR;
}

void read_buf(uint8_t reg, uint8_t *buf, uint8_t len) {
  csn(0);
  spi_transceive(R_REGISTER | (REGISTER_MASK & reg));
  for (int i = 0; i < len; ++i) {
    buf[i] = spi_transceive(NOP);
  }
  csn(1);
}

uint8_t read_reg(uint8_t reg) {
  uint8_t result;
  read_buf(reg, &result, 1);
  return result;
}

uint8_t write_buf(uint8_t reg, const uint8_t *buf, uint8_t len) {
  uint8_t status;
  csn(0);
  status = spi_transceive(W_REGISTER | (REGISTER_MASK & reg));
  for (int i = 0; i < len; ++i) {
    status = spi_transceive(buf[i]);
  }
  csn(1);
  return status;
}

uint8_t write_reg(uint8_t reg, uint8_t data) {
  return write_buf(reg, &data, sizeof(uint8_t));
}

uint8_t spi_command(uint8_t command) {
  uint8_t status;
  csn(0);
  status = spi_transceive(command);
  csn(1);
  return status;
}

void load_eeprom_settings(device_settings_t *settings) {
   settings->rf_power     = eeprom_read_byte(EECONFIG_RF_POWER) & 0x03;
   settings->rf_channel   = eeprom_read_byte(EECONFIG_RF_CHANNEL) & 0x7f;
   switch (eeprom_read_byte(EECONFIG_RF_DATA_RATE)) {
      case 0: settings->rf_data_rate = NRF_250KBS; break;
      case 1: settings->rf_data_rate = NRF_1MBS; break;
      case 2: settings->rf_data_rate = NRF_2MBS; break;
      default: settings->rf_data_rate = NRF_2MBS;
   }
   /* settings->rf_data_rate = NRF_2MBS; */

   /* // set address width */
   /* switch (eeprom_read_byte(EECONFIG_RF_ADDRESS_LEN)) { */
   /*    /1* case 2: write_reg(SETUP_AW, 0x0); break; *1/ */
   /*    case 3: write_reg(SETUP_AW, 0x1); break; */
   /*    case 4: write_reg(SETUP_AW, 0x2); break; */
   /*    case 5: write_reg(SETUP_AW, 0x3); break; */
   /*    default: write_reg(SETUP_AW, 0x3); break; */
   /* } */

   eeprom_read_block(settings->addr0, EECONFIG_DEVICE_ADDR_0, 5);
   eeprom_read_block(settings->addr1, EECONFIG_DEVICE_ADDR_1, 5);
}

uint8_t nrf_clear_flags(void) {
  return write_reg(NRF_STATUS, 0x70);
}

// CRC = 1 byte
uint8_t nrf_power_set(bool on) {
#if DEVICE_ID==MASTER_DEVICE_ID
  return write_reg(CONFIG, (0<<CRCO) | (1<<EN_CRC) | (on<<PWR_UP) | (1<<PRIM_RX));
#else // slave
  return write_reg(CONFIG, (0<<CRCO) | (1<<EN_CRC) | (on<<PWR_UP) | (0<<PRIM_RX));
#endif
}

/* TODO: add rf channel customization */
void nrf_setup(device_settings_t *settings) {
  nrf_power_set(0);

  #if DEVICE_ID==MASTER_DEVICE_ID
    write_reg(SETUP_RETR, 0);
    write_reg(EN_AA, (1<<ENAA_P0) | (1<<ENAA_P1)); // enable auto ack, on P0,P1
  #else // slave, with auto ack
    // ARD: delay for packet resend
    // ARC: max auto retransmit attempts
    /* NOTE: Most dropped packets occur when both slaves transmit at the same
     * time. When using auto retransmit, we need to make sure that we send the
     * replacement packets at different times, or otherwise we will keep having
     * an on-air collision every time we try to retransmit. So here, we give
     * each of the slaves a differnet delay to avoid this. */
    write_reg(SETUP_RETR, (((DEVICE_ID) & 0x7) << ARD) | ((RF_MAX_RETRANSMIT & 0xf) << ARC));
    write_reg(EN_AA, (1<<ENAA_P0)); // enable auto ack, on P0
  #endif

  write_reg(RF_CH, settings->rf_channel & 0x7f);
  write_reg(RF_SETUP, settings->rf_data_rate |
                      ((settings->rf_power & 0x3) << RF_PWR_LOW));

  /* write_reg(SETUP_AW, settings->rf_address_len); */

  // set address width
  switch (RF_ADDRESS_LEN) {
    case 3: write_reg(SETUP_AW, 0x1); break;
    case 4: write_reg(SETUP_AW, 0x2); break;
    case 5: write_reg(SETUP_AW, 0x3); break;
    default: break; //
  }

#if DEVICE_ID==MASTER_DEVICE_ID
  uint8_t zero_addr[RF_BUFFER_LEN] = { 0 }; // debug
  write_reg(EN_RXADDR, (1<<ERX_P0) | (1<<ERX_P1)); // enable rx pipes: P0, P1
  // set rx pipe addresses
  write_buf(RX_ADDR_P0, settings->addr0, RF_ADDRESS_LEN);
  write_buf(RX_ADDR_P1, settings->addr1, RF_ADDRESS_LEN);
  // set pipe packet sizes for pipes
  write_reg(RX_PW_P0, RF_BUFFER_LEN);
  write_reg(RX_PW_P1, RF_BUFFER_LEN);
  write_buf(TX_ADDR, zero_addr, RF_ADDRESS_LEN);
#else // slave
  // set slave rx addr to its tx address for auto ack
  write_reg(EN_RXADDR, (1<<ERX_P0)); // enable rx pipes: P0, P1
  write_reg(RX_PW_P0, 0);
  const uint8_t *this_addr = (DEVICE_ID == 0) ? settings->addr0 :
                                                    settings->addr1;
  write_buf(RX_ADDR_P0, this_addr, RF_ADDRESS_LEN);
  write_buf(TX_ADDR,    this_addr, RF_ADDRESS_LEN);
#endif

  // misc features
  write_reg(DYNPD, 0);
  write_reg(FEATURE, 0);

  // empty the buffers
  spi_command(FLUSH_RX);
  spi_command(FLUSH_TX);
  nrf_clear_flags();

  nrf_power_set(1);
  _delay_us(100);
}

uint8_t nrf_load_tx_fifo(uint8_t *buf, uint8_t len) {
  uint8_t status;
  csn(0);
  status = spi_transceive(W_TX_PAYLOAD);
  for (int i=0; i < len; ++i) {
    spi_transceive(buf[i]);
  }
  csn(1);
  return status;
}

void nrf_send_one(void) {
  ce(1);
  _delay_us(11); // need to hold CE for at least 10μs
  ce(0);
}

uint8_t nrf_send_all(void) {
  uint8_t status;
  ce(1);
  _delay_us(11);
  while( !((status = read_reg(FIFO_STATUS)) & (1<<TX_EMPTY)) &&
         !((status = read_reg(NRF_STATUS)) & (1<<MAX_RT)) );
  ce(0);
  return status;
}

void nrf_read_rx_fifo(uint8_t *buf, uint8_t len) {
  csn(0);
  spi_transceive(R_RX_PAYLOAD);
  for (int i=0; i < len; ++i) {
    buf[i] = spi_transceive(NOP);
  }
  csn(1);
}

uint8_t nrf_rx_pipe_number(void) {
  return (read_reg(NRF_STATUS)>>RX_P_NO) & 0b111;
}

#ifndef NRF_HPP_4CVBSNFA
#define NRF_HPP_4CVBSNFA

#include <stdint.h>
#include <stdbool.h>
#include "nRF24L01.h"
#include "../split-config.h"

#if defined(__AVR_ATmega32U4__)
#ifndef CE
  #define CE 5   // b5
#endif
#ifndef CSN
  #define CSN 6  // b6
#endif
#define MOSI 2   // b2
#define MISO 3   // b3
#define SCK  1   // b1
#elif defined(__AVR_ATmega328P__)
#ifndef CE
  #define CE 1   // b1
#endif
#ifndef CSN
  #define CSN 2  // b2
#endif
#define MOSI 3   // b3
#define MISO 4   // b4
#define SCK  5   // b5
#endif

#define RF_BUFFER_LEN 16 // 0-32 bytes

#ifndef RF_MAX_RETRANSMIT
#define RF_MAX_RETRANSMIT 15 // 0-15
#endif

#ifndef RF_ADDRESS_LEN
#define RF_ADDRESS_LEN 3 // 3-5 bytes
#endif

enum nrf_data_rate_t {
  NRF_250KBS = (1<<RF_DR_LOW) | (0<<RF_DR_HIGH),
  NRF_1MBS   = (0<<RF_DR_LOW) | (0<<RF_DR_HIGH),
  NRF_2MBS   = (0<<RF_DR_LOW) | (1<<RF_DR_HIGH),
};

typedef struct device_settings_t {
  uint8_t rf_power;
  uint8_t rf_channel;
  /* enum nrf_data_rate_t rf_data_rate; */
  uint8_t rf_data_rate;
  uint8_t rf_address_len;
  uint8_t addr0[5];
  uint8_t addr1[5];
  uint8_t extra_address_lsb[4];
} device_settings_t;

// spi functions
void spi_setup(void);
uint8_t spi_transceive(uint8_t data);
void read_buf(uint8_t reg, uint8_t *buf, uint8_t len);
uint8_t read_reg(uint8_t reg);
uint8_t write_buf(uint8_t reg, const uint8_t *buf, uint8_t len);
uint8_t write_reg(uint8_t reg, uint8_t data);
uint8_t spi_command(uint8_t command);

// setup and control
void load_eeprom_settings(device_settings_t *settings);
void nrf_setup(device_settings_t *settings);
uint8_t nrf_power_set(bool on);
void nrf_enable(uint8_t val);
uint8_t nrf_clear_flags(void);

// send
uint8_t nrf_load_tx_fifo(uint8_t *buf, uint8_t len);
uint8_t nrf_send_all(void);
void nrf_send_one(void);

// receive
void nrf_read_rx_fifo(uint8_t *buf, uint8_t len);
uint8_t nrf_rx_pipe_number(void);

#endif /* end of include guard: NRF_HPP_4CVBSNFA */

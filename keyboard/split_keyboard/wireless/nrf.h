#ifndef NRF_HPP_4CVBSNFA
#define NRF_HPP_4CVBSNFA

#include <stdint.h>
#include <stdbool.h>
#include "nRF24L01.h"
#include "../split-config.h"
#include "../split-util.h"

#if defined(__AVR_ATmega32U4__)
#define CE 7   // f7
#define CSN 6  // b6
#define MOSI 2 // b3
#define MISO 3 // b4
#define SCK  1 // b1
#elif defined(__AVR_ATmega328P__)
#ifndef CE
  #define CE 0   // b0
#endif
#ifndef CSN
  #define CSN 2  // b2
#endif
#define MOSI 3 // b3
#define MISO 4 // b4
#define SCK  5 // b5
#endif

// move some of these settings t eeprom

#define MAX_RETRANSMIT 15 // 0-15

#define RF_BUFFER_LEN 16 // 0-32 bytes
#define RF_ADDRESS_LEN 3 // 3-5 bytes

// spi functions
void spi_setup(bool double_speed);
uint8_t spi_transceive(uint8_t data);
void read_buf(uint8_t reg, uint8_t *buf, uint8_t len);
uint8_t read_reg(uint8_t reg);
uint8_t write_buf(uint8_t reg, const uint8_t *buf, uint8_t len);
uint8_t write_reg(uint8_t reg, uint8_t data);
uint8_t spi_command(uint8_t command);

// setup and control
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

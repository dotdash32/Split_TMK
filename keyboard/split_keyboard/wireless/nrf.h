#ifndef NRF_HPP_4CVBSNFA
#define NRF_HPP_4CVBSNFA

#include <stdint.h>
#include <stdbool.h>

#if defined(__AVR_ATmega32U4__)
#define CE 7   // f7
#define CSN 6  // b6
#define MOSI 2 // b3
#define MISO 3 // b4
#define SCK  1 // b1
#elif defined(__AVR_ATmega328P__)
#define CE 0   // b0
#define CSN 2  // b2
#define MOSI 3 // b3
#define MISO 4 // b4
#define SCK  5 // b5
#endif

#define MAX_RETRANSMIT 15 // 0-15
#define RF_PWR_LEVEL   0 // 0-3 ((-18 + x*6) dBm)

#define RF_BUFFER_LEN 16 // 0-32
#define RF_ADDRESS_LEN 3 // 3-5 bytes

void spi_setup(void);
uint8_t spi_transceive(uint8_t data);
void read_buf(uint8_t reg, uint8_t *buf, uint8_t len);
uint8_t read_reg(uint8_t reg);
uint8_t write_buf(uint8_t reg, const uint8_t *buf, uint8_t len);
uint8_t write_reg(uint8_t reg, uint8_t data);
uint8_t spi_command(uint8_t command);
void nrf_setup(uint8_t device_num);
uint8_t nrf_load_tx_fifo(uint8_t *buf, uint8_t len);
void nrf_send_all(void);
void nrf_send_one(void);
void nrf_clear_flags(void);
void nrf_read_rx_fifo(uint8_t *buf, uint8_t len);
uint8_t nrf_rx_pipe_number(void);
void nrf_power_set(bool on);
void ce(uint8_t val);
void nrf_enable(uint8_t val);

#endif /* end of include guard: NRF_HPP_4CVBSNFA */

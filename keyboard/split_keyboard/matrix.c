#include <avr/power.h>
#include <util/delay.h>
#include <string.h>
#include "debug.h"
#include "matrix.h"
#include "print.h"
#include "pro-micro.h"
#include "timer.h"
#include "wireless/crypto.h"
#include "wireless/nrf.h"
#include "wireless/nrf_debug.h"

/* matrix state(1:on, 0:off) */
static matrix_row_t matrix[MATRIX_ROWS];

struct package_stats_t {
    uint16_t idle;
    uint16_t bad[NUM_SLAVES];
    uint16_t miss[NUM_SLAVES];
    uint16_t rate[NUM_SLAVES];
    uint32_t count[NUM_SLAVES];
    uint32_t _last[NUM_SLAVES];
    uint32_t _rate[NUM_SLAVES];
    uint32_t _idle;
} stats = {0};

static aes_ctx_t aes_ctx = { 0 };
/* static aes_state_t aes_state[2] = { 0 }; */
static ecb_state_t ecb_state[2] = { 0 };
static device_settings_t settings;

inline
uint8_t matrix_rows(void)
{
    return MATRIX_ROWS;
}

inline
uint8_t matrix_cols(void)
{
    return MATRIX_COLS;
}

// this code runs before the usb and keyboard is initialized
void matrix_setup(void) {
    /* split_keyboard_setup(); */

    load_eeprom_settings(&settings);

    /* uint8_t key[16]; */
    /* eeprom_read_block(key, EECONFIG_AES_KEY, AES_KEY_LEN); */
    /* print_hex_buf(key, 16); */
    /* print("\n"); */

    /* crypto_init(aes_state, &aes_ctx, &settings); */

    crypto_init(&aes_ctx);
    for (int i = 0; i < NUM_SLAVES; ++i) {
      ecb_init(&ecb_state[i], i);
    }

    /* if (!has_usb()) { */
    /*     keyboard_slave_loop(); */
    /* } */
}

void wireless_init(void) {
  power_spi_enable();
  spi_setup();
  nrf_setup(&settings);
  nrf_enable(true);
}

void matrix_init(void)
{
    /* debug_enable = true; */
    /* debug_matrix = true; */
    /* debug_mouse = true; */
   wireless_init();
   /* TX_RX_LED_INIT; */

    print("\n");
   timer_init();

    // initialize matrix state: all keys off
    for (uint8_t i=0; i < MATRIX_ROWS; i++) {
        matrix[i] = 0;
    }
}

uint8_t calc_checksum(uint8_t *buf, uint8_t len) {
  uint8_t result = 0;
  for (int i = 0; i < len; ++i) {
    result += buf[i];
  }
  return result;
}

void update_device_matrix(uint8_t *buf, bool device_num) {
  uint8_t offset = device_num * ROWS_PER_HAND;
  for (int i = 0; i < ROWS_PER_HAND; ++i) {
    matrix[i+offset] = buf[i];
  }
}

void reset_device_matrix(bool device_num) {
  uint8_t offset = device_num * ROWS_PER_HAND;
  for (int i = 0; i < ROWS_PER_HAND; ++i) {
    matrix[i+offset] = 0;
  }
}

/* TODO: remove */
void matrix_slave_scan(void) {}

static uint32_t start_time = 0;
static uint32_t seconds = 0;
static uint8_t disconnect_counters[2] = { 0 };

uint8_t matrix_scan(void)
{
  bool is_idle = true;

  int pipe_num = nrf_rx_pipe_number();
  while( pipe_num < NUM_SLAVES ) {
    uint8_t buf[AES_BUF_LEN];
    nrf_read_rx_fifo(buf, AES_BUF_LEN);

#ifdef DEBUG_VERBOSE
    print_dec(pipe_num);
    print(": ");
    print_hex_buf(buf, AES_BUF_LEN);
#endif

    /* decrypt(&aes_state[pipe_num], &aes_ctx); */
    uint8_t err = ecb_decrypt(&ecb_state[pipe_num], &aes_ctx, buf);

#ifdef DEBUG_VERBOSE
    print(" => ");
    aes128_dec(buf, &aes_ctx);
    print_hex_buf(buf, AES_BUF_LEN);
    print(" err: ");
    phex(err);
    print("\n");
#endif

    is_idle = false;
    stats.count[pipe_num]++;
    /* uint8_t checksum = calc_checksum(aes_state[pipe_num].data, ROWS_PER_HAND); */
    /* if (aes_state[pipe_num].data[PACKET_CHECKSUM0] != checksum || */
    /*     aes_state[pipe_num].data[PACKET_CHECKSUM1] != checksum ) { */
    /*   stats.bad[pipe_num]++; */
    /* } else { */
    /*   update_device_matrix(aes_state[pipe_num].data, pipe_num); */
    /*   disconnect_counters[pipe_num] = 0; */
    /* } */
    if (err) {
      stats.bad[pipe_num]++;
    } else {
      update_device_matrix(ecb_state[pipe_num].payload, pipe_num);
      const uint32_t msg_id = ecb_state[pipe_num].message_id;
      if ( msg_id - stats._last[pipe_num] != 1 ) {
        stats.miss[pipe_num]++;
      }
      stats._last[pipe_num] = msg_id;
      disconnect_counters[pipe_num] = 0;
    }

    // check again
    pipe_num = nrf_rx_pipe_number();
  }

  if (is_idle) {
    stats._idle++;
  }

  uint32_t now = timer_read();
  if (now - start_time > 1000) {
    // update counters
    start_time = now;
    seconds++;

    stats.idle = stats._idle;
    stats._idle = 0;

    for (int i = 0; i < NUM_SLAVES; ++i) {
      stats.rate[i] = stats.count[i] - stats._rate[i];
      stats._rate[i] = stats.count[i];

      if (disconnect_counters[i] >= DISCONNECT_TIME) {
        reset_device_matrix(i);
        disconnect_counters[i] = 0;
        /* #ifdef DEBUG */
        /* print("Device: "); */
        /* print_dec(i); */
        /* print(" disconnected."); */
        /* #endif */
      } else {
        disconnect_counters[i]++;
      }
    }

    // check nrf still working by checking some register values
    uint8_t current_addr[RF_ADDRESS_LEN];
    read_buf(RX_ADDR_P0, current_addr, RF_ADDRESS_LEN);
    if (memcmp(settings.addr0, current_addr, RF_ADDRESS_LEN) != 0) {
      #ifdef DEBUG
        println("Master RX module error!");
        nrf_debug_info(1);
        println("Trying to reset...");
      #endif
      wireless_init();
      _delay_ms(100);
      #ifdef DEBUG
        nrf_debug_info(1);
      #endif
    }

    #ifdef DEBUG
    if (stats.count[0] == 0 && stats.count[1] == 0) {
      nrf_debug_info(1);
    }
    #endif


  #ifdef DEBUG
    print("count(L: ");
    xprintf("%06lu", stats.count[0]);
    print(", R: ");
    xprintf("%06lu", stats.count[1]);
    print(")");
    print(" miss(L: ");
    xprintf("%04u", stats.miss[0]);
    print(", R: ");
    xprintf("%04u", stats.miss[1]);
    print(")");
    print(" bad(L: ");
    xprintf("%04u", stats.bad[0]);
    print(", R: ");
    xprintf("%04u", stats.bad[1]);
    print(")");
    print(" rate(L: ");
    xprintf("%03u", stats.rate[0]);
    print(", R: ");
    xprintf("%03u", stats.rate[1]);
    print(")");
    print(" idle: ");
    xprintf("%05u", stats.idle);
    print(" timer: ");
    xprintf("%05lu", seconds);
    print("\n");
  #endif
  }

  return 0;
}

/* bool matrix_is_modified(void) */
/* { */
/*     if (debouncing) return false; */
/*     return true; */
/* } */

/* inline */
/* bool matrix_is_on(uint8_t row, uint8_t col) */
/* { */
/*     return (matrix[row] & ((matrix_row_t)1<<col)); */
/* } */

inline
matrix_row_t matrix_get_row(uint8_t row)
{
    return matrix[row];
}

void matrix_print(void)
{
    print("\nr/c 0123456789ABCDEF\n");
    for (uint8_t row = 0; row < MATRIX_ROWS; row++) {
        phex(row); print(": ");
        pbin_reverse16(matrix_get_row(row));
        print("\n");
    }
}

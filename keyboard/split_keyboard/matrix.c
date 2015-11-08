#include <avr/power.h>
#include "debug.h"
#include "matrix-wireless.h"
#include "matrix.h"
#include "print.h"
#include "pro-micro.h"
#include "split-util.h"
#include "timer.h"
#include "wireless/crypto.h"
#include "wireless/nrf.h"
#include "wireless/nrf_debug.h"

/* matrix state(1:on, 0:off) */
static matrix_row_t matrix[MATRIX_ROWS];

struct package_stats_t {
    uint32_t idle;
    uint32_t count[NUM_SLAVES];
    uint32_t bad[NUM_SLAVES];
    uint32_t rate[NUM_SLAVES];
    uint32_t _idle;
} stats = {0};

static aes_ctx_t aes_ctx = { 0 };
static aes_state_t aes_state[2] = { 0 };
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

    crypto_init(aes_state, &aes_ctx, &settings);

    /* if (!has_usb()) { */
    /*     keyboard_slave_loop(); */
    /* } */
}

void wireless_init(void) {
  power_spi_enable();
  spi_setup(false);
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
  int num_processed = 0;
  int pipe_num = nrf_rx_pipe_number();
  while( pipe_num < NUM_SLAVES ) {
    nrf_read_rx_fifo(aes_state[pipe_num].data, RF_BUFFER_LEN);

#ifdef DEBUG_VERBOSE
    print_dec(pipe_num);
    print(": ");
    print_hex_buf(aes_state[pipe_num].data, 16);
#endif

    decrypt(&aes_state[pipe_num], &aes_ctx);

#ifdef DEBUG_VERBOSE
    print(" => ");
    print_hex_buf(aes_state[pipe_num].data, 16);
    print("\n");
#endif

    stats.count[pipe_num]++;
    uint8_t ccsum = calc_checksum(aes_state[pipe_num].data, ROWS_PER_HAND);
    if (aes_state[pipe_num].data[CHECK_SUM_POSITION] != ccsum) {
      stats.bad[pipe_num]++;
    } else {
      update_device_matrix(aes_state[pipe_num].data, pipe_num);
      disconnect_counters[pipe_num] = 0;
    }

    num_processed++;

    // check again
    pipe_num = nrf_rx_pipe_number();
  }

  if (num_processed == 0) {
    stats._idle++;
  }

  uint32_t now = timer_read();
  if (now - start_time > 1000) {
    start_time = now;
    seconds++;

    for (int i = 0; i < NUM_SLAVES; ++i) {
      stats.rate[i] = stats.count[i] - stats.rate[i];
      if (disconnect_counters[i] >= DISCONNECT_TIME) {
        reset_device_matrix(i);
      } else {
        disconnect_counters[i]++;
      }
    }

    stats.idle = stats._idle;
    stats._idle = 0;
  #ifdef DEBUG
    print("count(L: ");
    xprintf("%06lu", stats.count[0]);
    print(" , R: ");
    xprintf("%06lu", stats.count[1]);
    print(")");
    print(" bad(L: ");
    xprintf("%04lu", stats.bad[0]);
    print(" , R: ");
    xprintf("%04lu", stats.bad[1]);
    print(")");
    print(" rate(L: ");
    xprintf("%03lu", stats.rate[0]);
    print(", R: ");
    xprintf("%03lu", stats.rate[1]);
    print(")");
    print(" idle: ");
    xprintf("%05lu", stats.idle);
    print(" timer: ");
    xprintf("%05lu", seconds);
    print("\n");
/* #ifdef DEBUG_VERBOSE */
    nrf_debug_info(1);
/* #endif */
    print("\n");
  #endif
    for (int i = 0; i < NUM_SLAVES; ++i) {
      stats.rate[i] = stats.count[i];
    }
  }

   /* for (int i = 0; i < 4; ++i) { */
   /*   phex(aes_state[pipe_num]); */
   /* } */
   /* pbin_reverse16(matrix_get_row(row)); */
   /* print("\n"); */
   /* TXLED0; */

   /* if( serial_transaction() ) { */
   /*      // turn on the indicator led when halves are disconnected */
   /*      TXLED1; */

   /*      error_count++; */

   /*      if (error_count > ERROR_DISCONNECT_COUNT) { */
   /*          // reset other half if disconnected */
   /*          int slaveOffset = (isLeftHand) ? (ROWS_PER_HAND) : 0; */
   /*          for (int i = 0; i < ROWS_PER_HAND; ++i) { */
   /*              matrix[slaveOffset+i] = 0; */
   /*          } */
   /*      } */
   /*  } else { */
   /*      // turn off the indicator led on no error */
   /*      TXLED0; */
   /*      error_count = 0; */
   /*  } */

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

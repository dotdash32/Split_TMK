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

static const int ROWS_PER_HAND = MATRIX_ROWS/2;

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
static aes_key_t aes_key = { 0 };

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
    split_keyboard_setup();
    // wireless setup

    aes128_init(aes_key.key, &aes_ctx);
    /* crypto_init(aes_key_t *key, aes_state_t *state, role_t role); */

    if (!has_usb()) {
        keyboard_slave_loop();
    }
}

void wireless_init(void) {
  power_spi_enable();
  spi_setup(false);
  nrf_setup(0);
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

void update_half(uint8_t *buf, bool device_num) {
  uint8_t offset = device_num * ROWS_PER_HAND;
  for (int i = 0; i < ROWS_PER_HAND; ++i) {
    matrix[i+offset] = buf[i];
  }
}

/* TODO: remove */
void matrix_slave_scan(void) {}

void print_hex_buf(uint8_t* data, uint8_t len) {
   for (int i = 0; i < len; ++i) {
      print_hex8(data[i]);
   }
}

static uint32_t start_time = 0;
static uint32_t seconds = 0;

uint8_t matrix_scan(void)
{
  int num_processed = 0;
  int pipe_num = nrf_rx_pipe_number();
  while( pipe_num < NUM_SLAVES ) {
    nrf_read_rx_fifo(aes_state[pipe_num].data, RF_BUFFER_LEN);

/* #ifdef DEBUG */
/*     print_hex_buf(aes_state[pipe_num].data, 16); */
/* #endif */

    decrypt(&aes_state[pipe_num], &aes_ctx);

/* #ifdef DEBUG */
/*     print(" => "); */
/*     print_hex_buf(aes_state[pipe_num].data, 16); */
/*     print("\n"); */
/* #endif */

    stats.count[pipe_num]++;
    uint8_t ccsum = calc_checksum(aes_state[pipe_num].data, ROWS_PER_HAND);
    if (aes_state[pipe_num].data[CHECK_SUM_POSITION] != ccsum) {
      stats.bad[pipe_num]++;
    } else {
      update_half(aes_state[pipe_num].data, pipe_num);
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
    }
    stats.idle = stats._idle;
    stats._idle = 0;
  #ifdef PRINT_STATS
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

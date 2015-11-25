#include <util/delay.h>
#include <avr/io.h>
#include <avr/interrupt.h>
#include <stdlib.h>

#include "./matrix.h"
#include "./clock.h"

static matrix_row_t matrix[ROWS_PER_HAND];
static matrix_row_t matrix_debouncing[ROWS_PER_HAND];

static matrix_row_t read_cols(void);
static void init_cols(void);
static void unselect_rows(void);
static void select_row(uint8_t row);

const uint8_t col_pin_mask = 0b00111111;
const uint8_t row_pin_mask = 0b00001111;

void matrix_init(void)
{
  // initialize row and col
  unselect_rows();
  init_cols();

  // initialize matrix state: all keys off
  for (uint8_t i=0; i < ROWS_PER_HAND; i++) {
    matrix[i] = 0;
    matrix_debouncing[i] = 0;
  }
}

// NOTE: the debouncing time of this scaning method is controlled by
// that rate at which this function is called.
inline
void matrix_scan_slow(matrix_scan_t *scan) {
  matrix_row_t cols;
  scan->changed = 0;
  scan->active = 0;

  for (uint8_t i = 0; i < ROWS_PER_HAND; i++) {
    select_row(i);
    /* clock_delay_slow_us(30);  // without this wait read unstable value. */
    asm volatile("nop");
    const uint8_t old_row_val = matrix[i]; // instead of delay

    cols = read_cols();
    matrix[i] = matrix_debouncing[i] & cols;
    scan->active |= matrix[i];
    scan->changed |= old_row_val != matrix[i];
    matrix_debouncing[i] = cols;
    unselect_rows();
  }
}

  inline
matrix_row_t matrix_get_row(uint8_t row)
{
  return matrix[row];
}

// Setup the matrix so it will trigger an interrupt when a pin changes
void matrix_interrupt_mode(void) {
#if DEVICE_ID==0
  // cols input, pull-up
  DDRC  &= ~col_pin_mask;
  PORTC |= col_pin_mask;

  // rows output, low
  DDRD  |=  row_pin_mask;
  PORTD &= ~row_pin_mask;

  // cols interrupt, on pin change
  PCICR  =  (1<<PCIE1);
  PCMSK1 = col_pin_mask;
  PCIFR  = 0x00;
#elif DEVICE_ID==1
  // cols input, pull-up
  DDRC  &= ~col_pin_mask;
  PORTC |= col_pin_mask;

  // rows output, low
  DDRD  |=  row_pin_mask;
  PORTD &= ~row_pin_mask;

  // cols interrupt, on pin change
  PCICR  =  (1<<PCIE1);
  PCMSK1 = col_pin_mask;
  PCIFR  = 0x00;
#endif
}

ISR(PCINT0_vect) {
  PCICR  = 0;
  PCMSK0 = 0;
  PCIFR  = 0;
}
ISR(PCINT1_vect) {
  PCICR  = 0;
  PCMSK1 = 0;
  PCIFR  = 0;
}
ISR(PCINT2_vect) {
  PCICR  = 0;
  PCMSK2 = 0;
  PCIFR  = 0;
}

/* TODO: update with *real* values */
/* Column pin configuration
 * left
 *   col: 0   1   2   3   4   5
 *   pin: C0  C1  C2  C3  C4  C5
 * right
 *   col: 0   1   2   3   4   5
 *   pin: C0  C1  C2  C3  C4  C5
 */
  inline
static void  init_cols(void)
{
  // Input with pull-up(DDR:0, PORT:1)
#if DEVICE_ID==0
  DDRC  &= ~(col_pin_mask);
  PORTC |=  (col_pin_mask);
#elif DEVICE_ID==1
  DDRC  &= ~(col_pin_mask);
  PORTC |=  (col_pin_mask);
#endif
}

  inline
static matrix_row_t read_cols(void)
{
#if DEVICE_ID==0
  return ~PINC & col_pin_mask;
#elif DEVICE_ID==1
  return ~PINC & col_pin_mask;
#endif
}

/* Row pin configuration
 * left
 *   row: 0  1  2  3
 *   pin: D0 D1 D2 D3
 * right:
 *   row: 0  1  2  3
 *   pin: D0 D1 D2 D3
 */
inline
static void unselect_rows(void)
{
  // Hi-Z(DDR:0, PORT:0) to unselect
#if DEVICE_ID==0
  DDRD  &= ~row_pin_mask;
  PORTD &= ~row_pin_mask;
#elif DEVICE_ID==1
  DDRD  &= ~row_pin_mask;
  PORTD &= ~row_pin_mask;
#endif
}

inline
static void select_row(uint8_t row)
{
  // Output low(DDR:1, PORT:0) to select
#if DEVICE_ID==0
  DDRD  |= (1<<row);
  PORTD &= ~(1<<row);
#elif DEVICE_ID==1
  DDRD  |= (1<<(3-row));
  PORTD &= ~(1<<(3-row));
#endif
}

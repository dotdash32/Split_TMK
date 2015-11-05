#include <util/delay.h>
#include <avr/io.h>
#include <avr/interrupt.h>
#include <stdlib.h>

#include "config.h"
#include "matrix.h"

#define debug(X) NULL
#define debug_hex(X) NULL

#ifndef DEBOUNCE
#   define DEBOUNCE  5
#endif

static uint8_t debouncing = DEBOUNCE;

/* matrix state(1:on, 0:off) */
static matrix_row_t matrix[MATRIX_ROWS];
static matrix_row_t matrix_debouncing[MATRIX_ROWS];

static matrix_row_t read_cols(void);
static void init_cols(void);
static void unselect_rows(void);
static void select_row(uint8_t row);

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

void matrix_init(void)
{
    //debug_enable = true;
    //debug_matrix = true;
    //debug_mouse = true;
    // initialize row and col
    unselect_rows();
    init_cols();

    // initialize matrix state: all keys off
    for (uint8_t i=0; i < MATRIX_ROWS; i++) {
        matrix[i] = 0;
        matrix_debouncing[i] = 0;
    }
}

void matrix_scan2(void) {
    for (uint8_t i = 0; i < MATRIX_ROWS; i++) {
        select_row(i);
        /* _delay_us(30 / div);  // without this wait read unstable value. */
        _delay_us(1);  // without this wait read unstable value.
        matrix_row_t cols = read_cols();
        matrix[i] = matrix_debouncing[i] & cols;
        matrix_debouncing[i] = cols;
        unselect_rows();
    }
}

matrix_row_t matrix_get_row2(uint8_t row)
{
    return matrix[row];
}

bool matrix_is_modified(void)
{
    if (debouncing) return false;
    return true;
}

inline
bool matrix_is_on(uint8_t row, uint8_t col)
{
    return (matrix[row] & ((matrix_row_t)1<<col));
}

/* inline */
matrix_row_t matrix_get_row(uint8_t row)
{
    return matrix[row];
}

#if IS_MASTER_DEVICE!=1
// matrix sleep mode
void matrix_interrupt_mode(void) {
  /* // set cols in the matrix to output high */
  /* // set the reset to output low */
  DDRB = 0xff;
  DDRC = 0xff; // rows as input
  DDRD = 0x03;
  PORTB = 0x00;
  PORTC = 0x00;
  PORTD = 0xfc;

  PCICR  =  (1<<PCIE2);
  PCMSK2 = 0xfc;
  PCIFR  = 0x00;
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
#endif

/* Column pin configuration
 * col: 0   1   2   3   4   5
 * pin: D0  D1  D2  D3  D4  D5
 */
static void  init_cols(void)
{
    // Input with pull-up(DDR:0, PORT:1)
  DDRD  &= ~(0xfc);
  PORTD |=  (0xfc);
}

static matrix_row_t read_cols(void)
{
    /* return (PIND&(1<<0) ? 0 : (1<<0)) | */
    /*        (PIND&(1<<1) ? 0 : (1<<1)) | */
    /*        (PIND&(1<<2) ? 0 : (1<<2)) | */
    /*        (PIND&(1<<3) ? 0 : (1<<3)) | */
    /*        (PIND&(1<<4) ? 0 : (1<<4)) | */
    /*        (PIND&(1<<5) ? 0 : (1<<5)); */
    return (~PIND & 0xfc) >> 2;
}

/* Row pin configuration
 * row: 0  1  2  3
 * pin: C0 C1 C2 C3
 */
static void unselect_rows(void)
{
    // Hi-Z(DDR:0, PORT:0) to unselect
    DDRC  &= ~0xF;
    PORTC &= ~0xF;
}

static void select_row(uint8_t row)
{
    // Output low(DDR:1, PORT:0) to select
    switch (row) {
        case 0:
            DDRC  |= (1<<0);
            PORTC &= ~(1<<0);
            break;
        case 1:
            DDRC  |= (1<<1);
            PORTC &= ~(1<<1);
            break;
        case 2:
            DDRC  |= (1<<2);
            PORTC &= ~(1<<2);
            break;
        case 3:
            DDRC  |= (1<<3);
            PORTC &= ~(1<<3);
            break;
    }
}

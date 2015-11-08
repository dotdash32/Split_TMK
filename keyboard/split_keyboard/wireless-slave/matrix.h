#ifndef MATRIX_H_VGYP1JIV
#define MATRIX_H_VGYP1JIV

#include "../config.h"
#include <stdbool.h>

typedef uint8_t matrix_row_t;

typedef struct matrix_scan_t {
  uint8_t changed;  // any key state changed
  uint8_t active;   // any key is down
} matrix_scan_t;

void matrix_init(void);
/* uint8_t matrix_scan_slow(void); */
void matrix_scan_slow(matrix_scan_t *scan);
matrix_row_t matrix_get_row(uint8_t row);
void matrix_interrupt_mode(void);

#endif /* end of include guard: MATRIX_H_VGYP1JIV */

#ifndef DEBUG_H
#define DEBUG_H

#include <stdint.h>

void print_hex(uint8_t x);
void print_hexln(uint8_t x);
void print_hex_buf(uint8_t *x, uint8_t len);
void debug_info(void);

#endif

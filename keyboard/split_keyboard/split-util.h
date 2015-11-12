#ifndef SPLIT_KEYBOARD_UTIL_H
#define SPLIT_KEYBOARD_UTIL_H

#include <stdbool.h>
#include "split-config.h"

typedef struct device_settings_t {
  uint8_t rf_power;
  uint8_t rf_channel;
  uint8_t addr0[5];
  uint8_t addr1[5];
  uint8_t extra_address_lsb[4];
} device_settings_t;

// slave version of matix scan, defined in matrix.c
void matrix_slave_scan(void);

void load_eeprom_settings(device_settings_t *settings);
void split_keyboard_setup(void);
bool has_usb(void);
void keyboard_slave_loop(void);

#endif

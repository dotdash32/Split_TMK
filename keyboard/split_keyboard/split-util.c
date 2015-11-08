#include <avr/io.h>
#include <avr/wdt.h>
#include <avr/power.h>
#include <avr/interrupt.h>
#include <util/delay.h>
#include <avr/eeprom.h>
#include "matrix.h"
#include "split-util.h"
#ifdef WIRED_CONNECTION
/* #include "wired/i2c.h" */
/* #include "wired/serial.h" */
#endif

void load_eeprom_settings(device_settings_t *settings) {
   settings->device_num = eeprom_read_byte(EECONFIG_DEVICE_NUMBER);
   settings->rf_channel = eeprom_read_byte(EECONFIG_RF_CHANNEL);
   eeprom_read_block(settings->addr0, EECONFIG_DEVICE_ADDR_0, 5);
   eeprom_read_block(settings->addr1, EECONFIG_DEVICE_ADDR_1, 5);
}

#ifdef WIRED_CONNECTION
/* static void keyboard_master_setup(void) { */
/* #ifdef USE_I2C */
/*    i2c_master_init(); */
/* #else */
/*    serial_master_init(); */
/* #endif */
/* } */

/* static void keyboard_slave_setup(void) { */
/* #ifdef USE_I2C */
/*    i2c_slave_init(); */
/* #else */
/*    serial_slave_init(); */
/* #endif */
/* } */

/* bool has_usb(void) { */
/*    USBCON |= (1 << OTGPADE); //enables VBUS pad */
/*    _delay_us(5); */
/*    return (USBSTA & (1<<VBUS));  //checks state of VBUS */
/* } */

/* void split_keyboard_setup(void) { */
/*    /1* setup_handedness(); *1/ */

/*    if (has_usb()) { */
/*       keyboard_master_setup(); */
/*    } else { */
/*       keyboard_slave_setup(); */
/*    } */
/*    sei(); */
/* } */

/* void keyboard_slave_loop(void) { */
/*    matrix_init(); */

/*    while (1) { */
/*       matrix_slave_scan(); */
/*    } */
/* } */
#endif

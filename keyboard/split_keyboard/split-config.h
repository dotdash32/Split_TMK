#ifndef EECONFIG_H_MKEYEMUZ
#define EECONFIG_H_MKEYEMUZ

#define EECONFIG_BOOTMAGIC_END (uint8_t *) 0x0007

#define EECONFIG_AES_KEY       (uint8_t*)  0x0010 // 16 bytes
#define EECONFIG_NONCE_COUNTER (uint32_t*) 0x0020 // 4  bytes

#define EECONFIG_DEVICE_NUMBER (uint8_t*)  0x0024 // 1 byte
#define EECONFIG_RF_CHANNEL    (uint8_t*)  0x0025 // 1 byte

#define EECONFIG_DEVICE_ADDR_0 (uint8_t*)  0x0030 // 5  bytes
#define EECONFIG_DEVICE_ADDR_1 (uint8_t*)  0x0035 // 5  bytes

// affects last byte of addr_2, see nrf24l0 datasheet
// #define EECONFIG_DEVICE_ADDR_2 (uint8_t*)0x0036 // 1  bytes
// #define EECONFIG_DEVICE_ADDR_3 (uint8_t*)0x0037 // 1  bytes
// #define EECONFIG_DEVICE_ADDR_4 (uint8_t*)0x0048 // 1  bytes
// #define EECONFIG_DEVICE_ADDR_5 (uint8_t*)0x0049 // 1  bytes

#define SLAVE_I2C_ADDRESS 0x32

#define NUM_SLAVES 2

// How long the keyboard can be inactive before it goes to sleep. The keyboard
// is considered active if any key is down.
/* #define INACTIVITY_TIMEOUT 15 // 0-255 seconds */

// If no keys are either presses or released in this time, the keyboard
// will go to sleep. If something gets left on the keyboard, this value will
// let it know when it is safe to assume nobody is using it, but it also
// limits how long you can hold down a key for.
/* #define UNCHANGED_TIMEOUT 30 // 0-255 seconds */

#endif /* end of include guard: EECONFIG_H_MKEYEMUZ */


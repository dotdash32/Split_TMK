#ifndef EECONFIG_H_MKEYEMUZ
#define EECONFIG_H_MKEYEMUZ

#define EECONFIG_BOOTMAGIC_END (uint8_t *) 0x0007

#define EECONFIG_AES_KEY       (uint8_t*)  0x0010 // 16 bytes
#define EECONFIG_NONCE_COUNTER (uint32_t*) 0x0020 // 4  bytes

#define EECONFIG_RF_POWER      (uint8_t*)  0x0024 // 1 byte
#define EECONFIG_RF_CHANNEL    (uint8_t*)  0x0025 // 1 byte
/* #define EECONFIG_RF_POWE       (uint8_t*)  0x0024 // 1 byte */

#define EECONFIG_DEVICE_ADDR_0 (uint8_t*)  0x0030 // 5  bytes
#define EECONFIG_DEVICE_ADDR_1 (uint8_t*)  0x0035 // 5  bytes

// affects last byte of addr_2, see nrf24l0 datasheet
// #define EECONFIG_DEVICE_ADDR_2 (uint8_t*)0x0036 // 1  bytes
// #define EECONFIG_DEVICE_ADDR_3 (uint8_t*)0x0037 // 1  bytes
// #define EECONFIG_DEVICE_ADDR_4 (uint8_t*)0x0048 // 1  bytes
// #define EECONFIG_DEVICE_ADDR_5 (uint8_t*)0x0049 // 1  bytes

#define NUM_SLAVES 2
#ifndef DEVICE_ID
#error "Need to set DEVICE_ID to build"
#endif

#define MASTER_DEVICE_ID 255

#ifdef WIRED
#define SLAVE_I2C_ADDRESS 0x32
#endif

#define PACKET_CHECKSUM0 ROWS_PER_HAND
#define PACKET_CHECKSUM1 (ROWS_PER_HAND+1)

#define RF_PWR_LEVEL   3 // 0-3 ((-18 + x*6) dBm)

// If the master doesn't receive a response from a slave in this time,
// it will consider it disconnected, and reset its key state.
#define DISCONNECT_TIME 2

#endif /* end of include guard: EECONFIG_H_MKEYEMUZ */

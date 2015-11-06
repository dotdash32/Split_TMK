#ifndef EECONFIG_H_MKEYEMUZ
#define EECONFIG_H_MKEYEMUZ

#define EECONFIG_BOOTMAGIC_END      (uint8_t *)7
#define EECONFIG_HANDEDNESS         EECONFIG_BOOTMAGIC_END

#define EECONFIG_AES_KEY       (uint8_t*)0x0010 // 16 bytes
#define EECONFIG_NONCE_COUNTER (uint32_t*)0x0020 // 4  bytes

#define SLAVE_I2C_ADDRESS           0x32

#define NUM_SLAVES 2

#endif /* end of include guard: EECONFIG_H_MKEYEMUZ */


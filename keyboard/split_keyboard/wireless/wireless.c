/*
* Getting Started example sketch for nRF24L01+ radios
* This is a very basic example of how to send data from one node to another
* Updated: Dec 2014 by TMRh20
*/

/* #include "config.h" */
#include "nRF24L01.h"
#include <avr/eeprom.h>
#include <avr/interrupt.h>
#include <avr/io.h>
#include <avr/pgmspace.h>
#include <avr/power.h>
#include <avr/sleep.h>
#include <util/delay.h>
/* #include <avr/wdt.h> */
/* #include <util/setbaud.h> */

#include "nrf.h"
#include "crypto.h"
#include "debug.h"
#include "matrix.h"

/* #define CLOCK_DIV 32 */
/* #define TIMER_DIV 1024 */
/* volatile uint32_t milliseconds; */

/* void timer2_init() { */
/*     TCCR0B = (0b101 << CS00); // prescale 1024 */
/*     TCNT2 = 0; // reset counter */
/*     TIMSK0 = (1 << TOIE2); // enable interrupt */
/*     milliseconds = 0; */
/* } */

/* ISR(TIMER2_OVF_vect) { */
/*     milliseconds += (uint32_t)(255.0 * (((float)F_CPU / CLOCK_DIV) / TIMER_DIV)); */
/* } */



/* #if !IS_MASTER_DEVICE */
/* void slave_sleep(void) { */
/*   // turn off nrf */

/*   // disable ADC */
/*   ADCSRA = 0; */

/*   set_sleep_mode(SLEEP_MODE_PWR_DOWN); */
/*   cli(); */
/*   sleep_enable(); */

/*   // turn off brown-out enable in software */
/*   MCUCR = (1<<BODS) | (1<<BODSE); */
/*   MCUCR = (1<<BODS); */

/*   matrix_interrupt_mode(); */

/*   // setup pin change interrupts */
/*   sei(); */
/*   sleep_cpu (); */
/*   cli(); */
/* } */

/* void slave_wakeup(void) { */
/*   PCICR  = 0; */
/*   PCIFR  = 0; */
/*   setup_hardware(); */
/*   sei(); */
/* } */
/* #endif */

/* #define SCAN_RATE 100 // approximate value at 0.5MHz */
/* #define INACTIVITY_TIMEOUT 10 // (seconds) time to sleep */

/* uint16_t inactive_count = 0; */
/* uint16_t inactive_time = 0; */
/* uint32_t start_time; */
/* uint16_t seconds = 0; */
/* uint8_t nrf_status = 0; */

/* void slave_task(void) { */
/*   uint8_t is_active = 0; */
/*   uint8_t check_sum = 0; */
/*   uint8_t stat = 0; */

/*   matrix_scan2(); */
/*   for (int i = 0; i < MATRIX_ROWS; ++i) { */
/*     const uint8_t row = matrix_get_row2(i); */
/*     aes_state[isLeftHand].data[i] = row; */
/*     is_active |= row; */
/*     check_sum += row; */
/*   } */
/*   aes_state[isLeftHand].data[MATRIX_ROWS] = check_sum; */

/*   // this function takes about 3ms at 0.5MHz */
/*   encrypt(&aes_state[isLeftHand], &aes_ctx); */

/*   if(nrf_status & (1<<MAX_RT)) { */
/*     nrf_send_all(); */
/*     spi_command(FLUSH_TX); */
/*     nrf_clear_flags(); */
/*   } */

/*   nrf_status = nrf_load_tx_fifo(aes_state[isLeftHand].data, RF_BUFFER_LEN); */
/*   nrf_send_all(); */
/*   /1* nrf_send_one(); *1/ */
/*   stats.total++; */

/*   if (is_active) { */
/*     inactive_count = 0; */
/*     inactive_time = millis(); */
/*   } else { */
/*     inactive_count++; */
/*     // TODO clock div */
/*     /1* if (inactive_count > (INACTIVITY_TIMEOUT * SCAN_RATE)) { *1/ */
/*     if ((millis()-inactive_time) > (INACTIVITY_TIMEOUT*1000/32)) { */
/*       /1* slave_sleep(); *1/ */
/*       /1* slave_wakeup(); *1/ */
/*       /1* inactive_count = 0; *1/ */
/*       /1* inactive_time = millis(); *1/ */
/*     } */
/*   } */

/* #ifdef DEBUG */
/*   uint32_t now = millis() - start_time; */
/*   Serial.print(" time: "); */
/*   Serial.println(now / 1000); */
/*   _delay_ms(3); */
/* #endif */
/* } */

/* void master_task(void) { */
/*   stats.total += recv_update(); */
/*   uint32_t now = millis() - start_time; */
/*   /1* if( (now/1000) > seconds ) { *1/ */
/*   if( (now/1000) > (seconds) ) { */
/*     seconds = now/1000; */

/*     /1* Serial.print(stats.total); *1/ */
/*     /1* Serial.print(" bad: "); *1/ */
/*     /1* Serial.println(stats.bad); *1/ */

/*     /1* debug_info(); *1/ */

/*     Serial.print(" count: "); */
/*     Serial.print(stats.total); */
/*     Serial.print(" rate: "); */
/*     float rate = stats.total / ((float)now/1000); */
/*     Serial.print(rate); */
/*     Serial.print(" bad: "); */
/*     Serial.print(stats.bad); */
/*     Serial.print(" bad%: "); */
/*     Serial.print((float)stats.bad / stats.total); */
/*     Serial.print(" time: "); */
/*     Serial.println(seconds); */

/*     print_hex(aes_state[1].data[0]); */
/*     print_hex(aes_state[1].data[1]); */
/*     print_hex(aes_state[1].data[2]); */
/*     print_hex(aes_state[1].data[3]); */
/*     Serial.print(" "); */
/*     print_hex(aes_state[0].data[0]); */
/*     print_hex(aes_state[0].data[1]); */
/*     print_hex(aes_state[0].data[2]); */
/*     print_hex(aes_state[0].data[3]); */

/*   } */
/* } */

/* void setup(void) { */
/* #if IS_MASTER_DEVICE!=1 */
/* #ifdef DEBUG */
/*   Serial.begin(MONITOR_BAUDRATE); */
/*   Serial.print("#### "); */
/*   Serial.print(F(__FILE__)); */
/*   Serial.println(" ####"); */
/* #endif */
/* #else */
/*   Serial.begin(MONITOR_BAUDRATE); */
/*   /1* Serial1.begin(MONITOR_BAUDRATE); *1/ */
/*   Serial.print("#### "); */
/*   Serial.print(F(__FILE__)); */
/*   Serial.println(" ####"); */
/* #endif */

/*   /1* delay(1000); *1/ */

/*   setup_hardware(); */

/*   for (int i = 0; i < 16; ++i) { */
/*     aes_key.key[i] = 0; */
/*   } */
/*   aes128_init(aes_key.key, &aes_ctx); */
/*   /1* crypto_init(aes_key_t *key, aes_state_t *state, role_t role); *1/ */

/*   sei(); */
/* #if IS_MASTER_DEVICE==1 */
/*   debug_info(); */
/*   Serial.print("==master==\n"); */
/*   start_time = millis(); */
/*   // start listening */
/*   ce(1); */
/*   delay(100); */
/* #else */
/* #ifdef DEBUG */
/*   debug_info(); */
/*   delay(100); */
/* #else */
/*   clock_prescale_set(clock_div_32); */
/*   _delay_us(1); */
/* #endif */
/* #endif */
/* } */

/* void loop(void) { */
/* #if IS_MASTER_DEVICE==1 */
/*   master_task(); */
/* #else */
/*   slave_task(); */
/* #endif */
/* } // Loop */

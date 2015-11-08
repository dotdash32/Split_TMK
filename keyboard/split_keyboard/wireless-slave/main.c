#include <avr/eeprom.h>
#include <avr/interrupt.h>
#include <avr/io.h>
#include <avr/pgmspace.h>
#include <avr/power.h>
#include <avr/sleep.h>
#include <util/delay.h>
#include <avr/wdt.h>

#include "../config.h"
#include "../wireless/nrf.h"
#include "../wireless/nRF24L01.h"
#include "../wireless/crypto.h"
#include "../split-util.h"
#include "./matrix.h"
#include "./clock.h"

// How long the keyboard can be inactive before it goes to sleep. The keyboard
// is considered active if any key is down.
#ifndef INACTIVITY_TIMEOUT
#define INACTIVITY_TIMEOUT 0 // 0-255 seconds
#endif

// If no keys are either presses or released in this time, the keyboard
// will go to sleep. If something gets left on the keyboard, this value will
// let it know when it is safe to assume nobody is using it, but it also
// limits how long you can hold down a key for.
#ifndef UNCHANGED_TIMEOUT
#define UNCHANGED_TIMEOUT 30 // 0-255 seconds
#endif

// // how many iterations the main loop can do in a second
// NOTE: if this value is raised much higher, a different debounce algorithm
// may be necessary.
// WARN: this is value was measured for the current clock speeds and does
// not control the given scan rate.
#ifdef FAST_SCAN
#define SCAN_RATE 340
#else
#define SCAN_RATE 180
#endif

aes_ctx_t aes_ctx = { 0 };
aes_state_t aes_state = { {0} };
device_settings_t settings = { 0 };

void disable_unused_hardware(void) {
  wdt_disable();

  ADCSRA = 0; // disable ADC
  power_adc_disable();

  power_usart0_disable();
  power_timer0_disable();
  power_timer1_disable();
  power_timer2_disable();
  power_twi_disable();
}

void disable_hardware(void) {
  power_spi_disable();
}

void enable_hardware(void) {
  power_spi_enable();
}

void reset_hardware(void) {
  // default state, output low
  DDRB = 0xff;
  DDRC = 0xff;
  DDRD = 0xff;
  PORTB = 0x00;
  PORTC = 0x00;
  PORTD = 0x00;

  spi_setup(true);  // use clk/2 for SCK
  nrf_setup(&settings);
  matrix_init();
}

void slave_sleep(void) {
  nrf_power_set(0);

  disable_hardware();

  set_sleep_mode(SLEEP_MODE_PWR_DOWN);
  cli();
  sleep_enable();

  // set the matrix to fire interrupt on pin change
  matrix_interrupt_mode();

  // want to be fast on wakeup
  clock_fast();

  // Turn off brown out detection during sleep. Saves about 35μA in sleep mode.
  // Brown-out detection will be automatically reenabled on wake up.
  // WARN: This is a timed sequence. Have 4 clock cycles, from this point
  // to go to sleep, or otherwise BODS will be reenabled.
  MCUCR = (1<<BODS) | (1<<BODSE); // special write sequence, see datasheet
  MCUCR = (1<<BODS);
  sei();
  sleep_cpu ();
  cli();
}

void slave_wakeup(void) {
  enable_hardware();
  reset_hardware();
  sei();
}

void setup() {
  clock_fast();

  disable_unused_hardware();
  load_eeprom_settings(&settings);
  _delay_ms(100); // nrf takes about this long to start from no power
  reset_hardware();

  // TODO: provide key
  crypto_init(&aes_state, &aes_ctx,  &settings);

  sei();
}

int main(void) {
  matrix_scan_t scan = {0};
  uint8_t nrf_status = 0;
  uint8_t inactive_time = 0;
  uint8_t unchanged_time = 0;
#ifdef FAST_SCAN
  uint16_t scan_rate_counter = 0;
#else
  uint8_t scan_rate_counter = 0;
#endif

  setup();
  // send our iv to the master
  nrf_status = nrf_load_tx_fifo(aes_state.iv, RF_BUFFER_LEN);
  nrf_send_one();
  clock_slow();

  while(1) {
    // WARN: the debounce of this scan is effectively the rate at which this
    // loop runs.
    matrix_scan_slow(&scan);

    // see if our previous message has been sent yet.
    if(nrf_status & (1<<MAX_RT)) {
      clock_fast();
      // last ditch effort to get our key change registered
      nrf_send_all();
      spi_command(FLUSH_TX);
      nrf_clear_flags();
      nrf_status = 0;
      clock_slow();
    }

    const uint8_t should_ping = scan_rate_counter == 0;
    // only send the scan result if something changed
    if (scan.changed || should_ping) {
      clock_fast();

      uint8_t checksum = 0;
      for (int i = 0; i < ROWS_PER_HAND; ++i) {
        aes_state.data[i] = matrix_get_row(i);
        checksum += aes_state.data[i];
      }
      aes_state.data[ROWS_PER_HAND] = checksum;

      encrypt(&aes_state, &aes_ctx);

      nrf_status = nrf_load_tx_fifo(aes_state.data, RF_BUFFER_LEN);
      nrf_send_one();

      if(scan.changed) {
        unchanged_time = 0;
      }

      if (scan.active) {
        inactive_time = 0;
      }

      clock_slow();
    }

    scan_rate_counter++;
    if (scan_rate_counter == SCAN_RATE) {
      scan_rate_counter = 0;

      if ( (inactive_time > INACTIVITY_TIMEOUT) ||
          (unchanged_time > UNCHANGED_TIMEOUT)) {
        inactive_time = 0;
        unchanged_time = 0;
        slave_sleep();
        slave_wakeup();
      }

      inactive_time++;
      unchanged_time++;
    }
  }
}

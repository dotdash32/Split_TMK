#include <avr/eeprom.h>
#include <avr/interrupt.h>
#include <avr/io.h>
#include <avr/pgmspace.h>
#include <avr/power.h>
#include <avr/sleep.h>
#include <util/delay.h>
#include <avr/wdt.h>

#include "../wireless/nrf.h"
#include "../wireless/nRF24L01.h"
#include "../wireless/crypto.h"
#include "matrix.h"
#include "clock.h"

// How long the keyboard can be inactive before it goes to sleep. The keyboard
// is considered active if any key is down.
#ifndef INACTIVITY_TIMEOUT
#define INACTIVITY_TIMEOUT 15 // 0-255 (seconds)
#endif

// If no keys are either presses or released in this time, the keyboard
// will go to sleep. If something gets left on the keyboard, this value will
// let it know when it is safe to assume nobody is using it.
#ifndef NO_CHANGE_TIMEOUT
#define NO_CHANGE_TIMEOUT 100 // >=0 seconds
#endif

// WARN: this is value was measured for the current clock speeds and does
// not control the given scan rate.
// NOTE: 200Hz is a good scan rate for Cherry MX while using our simple
// debounce algorithm, since their debounce time is ≤5ms.
#define SCAN_RATE 170 // how many iterations the main loop can do in a second

aes_ctx_t aes_ctx = { 0 };
aes_state_t aes_state = { 0 };
const uint8_t device_number = DEVICE_NUMBER;

typedef struct loop_control_t {
  uint32_t inactive_start_time;
  uint32_t last_changed_time;
  uint32_t seconds; // very, very low accuracy
  uint8_t nrf_status;
  uint8_t scan_rate_counter;
  uint8_t should_ping; // ping master to let it now we are still here
} loop_control_t;

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
  nrf_setup(device_number);
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

  _delay_ms(100); // want to make sure nrf is ready
  reset_hardware();

  // TODO: provide key
  crypto_init(&aes_state, &aes_ctx,  device_number);

  sei();
}

void slave_task(loop_control_t *loop) {
  matrix_scan_t scan;

  // WARN: the debounce of this scan is effectively the rate at which this
  // loop runs.
  matrix_scan_slow(&scan);

  // see if our previous message has been sent yet.
  if(loop->nrf_status & (1<<MAX_RT)) {
    clock_fast();
    // last ditch effort to get our key change registered
    nrf_send_all();
    spi_command(FLUSH_TX);
    nrf_clear_flags();
    loop->nrf_status = 0;
    clock_slow();
  }

  // only send the scan result if something changed
  if (scan.changed || loop->should_ping) {
    // speed up for encryption and transmit
    clock_fast();

    for (int i = 0; i < MATRIX_ROWS; ++i) {
      aes_state.data[i] = matrix_get_row(i);
    }
    aes_state.data[MATRIX_ROWS] = scan.checksum;

    encrypt(&aes_state, &aes_ctx);

    loop->nrf_status = nrf_load_tx_fifo(aes_state.data, RF_BUFFER_LEN);
    nrf_send_one();

    if(scan.changed) {
      loop->last_changed_time = loop->seconds;
    }
    clock_slow();
  }

  if (scan.active) {
    loop->inactive_start_time = loop->seconds;
  } else {
    if ((loop->seconds - loop->inactive_start_time) > (INACTIVITY_TIMEOUT)) {
      /* slave_sleep(); */
      /* slave_wakeup(); */
      loop->inactive_start_time = loop->seconds;
    }
  }
}

int main(void) {
  setup();
  loop_control_t loop;

  while(1) {
    slave_task(&loop);
    loop.scan_rate_counter++;
    if (loop.scan_rate_counter == SCAN_RATE) {
      loop.scan_rate_counter = 0;
      loop.should_ping = true;
      loop.seconds++;
    } else {
      loop.should_ping = false;
    }
  }
}

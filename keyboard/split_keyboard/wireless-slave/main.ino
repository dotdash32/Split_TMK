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

aes_ctx_t aes_ctx = { 0 };
aes_state_t aes_state = { 0 };
uint8_t device_number = 1;

#define BASE_CLOCK_DIV clock_div_16
#define DOUBLE_SPEED_DIV clock_div_8
#define SLOWEST_SPEED_DIV clock_div_256

volatile uint32_t milliseconds = 0;

/* void timer2_init(void) { */
/*   TCCR0B = (0b101 << CS00); // prescale 1024 */
/*   TCNT2 = 0; // reset counter */
/*   TIMSK0 = (1 << TOIE2); // enable interrupt */
/*   milliseconds = 0; */
/* } */

/* ISR(TIMER2_OVF_vect) { */
/*   milliseconds += (uint32_t)(255.0 * (((float)F_CPU / CLOCK_DIV) / TIMER_DIV)); */
/* } */

void reset_hardware(void) {
  wdt_disable();

  power_adc_disable();
  power_usart0_disable();
  /* power_timer0_disable(); */
  /* power_timer1_disable(); */
  /* power_timer2_disable(); */
  power_twi_disable();

  ADCSRA = 0; // disable ADC

  // default state, output low
  DDRB = 0xff;
  DDRC = 0xff;
  DDRD = 0xff;
  PORTB = 0x00;
  PORTC = 0x00;
  PORTD = 0x00;

  power_spi_enable(); // only need spi
  spi_setup();
  nrf_setup(device_number);
  matrix_init();
}

void slave_sleep(void) {
  nrf_power_set(0);

  set_sleep_mode(SLEEP_MODE_PWR_DOWN);
  cli();
  sleep_enable();

  // turn off brown-out enable in software
  MCUCR = (1<<BODS) | (1<<BODSE);
  MCUCR = (1<<BODS);

  // set the matrix to fire interrupt on pin change
  matrix_interrupt_mode();

  clock_prescale_set(DOUBLE_SPEED_DIV);

  sei();
  sleep_cpu ();
  cli();
}

inline
uint8_t my_clock_prescale_get(void) {
  const uint8_t current = clock_prescale_get();
  if (current <= BASE_CLOCK_DIV) {
    return BASE_CLOCK_DIV;
  } else {
    return current - BASE_CLOCK_DIV;
  }
}

void slave_wakeup(void) {
  PCICR  = 0;
  PCIFR  = 0;
  reset_hardware();
  sei();
}

#define INACTIVITY_TIMEOUT 15 // (seconds??) time to sleep

uint32_t inactive_start_time = 0;
uint16_t inactive_count = 0;
uint32_t start_time;
uint32_t seconds = 0;
uint8_t nrf_status = 0;
bool is_slow = false;

void slave_task(void) {
  uint8_t is_active = 0;
  uint8_t check_sum = 0;
  uint8_t changed = 0;

  /* matrix_scan2(); */
  changed = matrix_scan2(is_slow);

  for (int i = 0; i < MATRIX_ROWS; ++i) {
    const uint8_t row = matrix_get_row2(i);
    aes_state.data[i] = row;
    is_active |= row;
    check_sum += row;
  }
  aes_state.data[MATRIX_ROWS] = check_sum;


  if(nrf_status & (1<<MAX_RT)) {
    // TODO: if we reach MAX_RT frequently, it's probably a good idea
    // to slow done the number of packets we send to save power.
    nrf_send_all();
    spi_command(FLUSH_TX);
    nrf_clear_flags();
    nrf_status = 0;
  }

  // only send data if we have keys pressed
  if (changed) {
    // speed up
    clock_prescale_set(DOUBLE_SPEED_DIV);
    is_slow = false;
    encrypt(&aes_state, &aes_ctx);

    nrf_status = nrf_load_tx_fifo(aes_state.data, RF_BUFFER_LEN);
    nrf_send_one();
    // normal speed
    clock_prescale_set(BASE_CLOCK_DIV);
  } else {
    // slow down
    clock_prescale_set(SLOWEST_SPEED_DIV);
    is_slow = true;
  }

  /* if (is_active) { */
  /*   inactive_start_time = milliseconds; */
  /* } else { */
  /*   if ((milliseconds-inactive_start_time) > (INACTIVITY_TIMEOUT)) { */
  /*     slave_sleep(); */
  /*     slave_wakeup(); */
  /*     inactive_start_time = milliseconds; */
  /*   } */
  /* } */
  if (is_active) {
    inactive_count = 0;
  } else {
    inactive_count++;
    if ((inactive_count) > (INACTIVITY_TIMEOUT*122L)) {
      /* slave_sleep(); */
      /* slave_wakeup(); */
      /* inactive_count = 0; */
    }
  }
}

void setup() {
  clock_prescale_set(BASE_CLOCK_DIV);

  /* aes_key_t aes_key = { 0 }; */
  _delay_ms(100); // want to make sure nrf is ready
  reset_hardware();

  crypto_init(&aes_state, &aes_ctx,  device_number);

  sei();
}

void loop() {
  slave_task();
}

/* int main(int argc, char *argv[]) { */
/*   /1* clock_prescale_set(BASE_CLOCK_DIV); *1/ */


/*   /1* sei(); *1/ */

/*   /1* while (1) { *1/ */
/*   /1*   slave_task(); *1/ */
/*   /1*   /2* _delay_ms(3); *2/ *1/ */
/*   /1* } *1/ */
/* } */

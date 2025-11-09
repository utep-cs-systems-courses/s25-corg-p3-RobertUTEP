#include <msp430.h>

#include "led.h"



#define GREEN BIT6

#define LEDS  GREEN



void led_init() {

  P1DIR |= LEDS;

  P1OUT &= ~LEDS;

}



void green_on() {

  P1OUT |= GREEN;

}



void green_off() {

  P1OUT &= ~GREEN;

}


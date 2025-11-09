#include <msp430.h>
#include "libTimer.h"
#include "buzzer.h"

/* LEDs */
#define RED   BIT0      /* P1.0 */
#define GREEN BIT6      /* P1.6 */

/* Buttons on expansion board (pressed = 0) */
#define B0 BIT0         /* P2.0: Red only */
#define B1 BIT1         /* P2.1: Green only */
#define B2 BIT2         /* P2.2: Both */
#define B3 BIT3         /* P2.3: Sound toggle */
#define BUTTONS (B0|B1|B2|B3)

/* Modes */
enum { MODE_RED=0, MODE_GREEN=1, MODE_BOTH=2 };

/* State */
static volatile unsigned char mode    = MODE_RED;
static volatile unsigned char level   = 16;   /* 16..1 then reset */
static volatile unsigned char pwmStep = 0;    /* 0..15 */
static volatile unsigned char soundOn = 1;    /* start with sound ON */
static          unsigned char prevPressed = 0;

/* --- buttons: GPIO inputs with pull-ups, active-low --- */
static inline void buttons_init(void){
  P2SEL  &= ~BUTTONS;
  P2SEL2 &= ~BUTTONS;
  P2DIR  &= ~BUTTONS;
  P2REN  |=  BUTTONS;            /* enable resistors */
  P2OUT  |=  BUTTONS;            /* pull-ups -> idle=1 */
  prevPressed = 0;
}

/* one-shot edges where a button just became pressed */
static inline unsigned char buttons_fell(void){
  unsigned char rawPressed = (~P2IN) & BUTTONS;  /* pressed -> 1 */
  unsigned char fell = rawPressed & (unsigned char)~prevPressed;
  prevPressed = rawPressed;
  return fell;
}

/* return bitmap of buttons currently pressed (active-high) */

static inline unsigned char buttons_pressed(void){

  return (~P2IN) & BUTTONS;   // pressed -> 1

}

/* --- sound on TA0.1 (P2.6) --- */
static inline unsigned short tone_for_mode(unsigned char m){
  return (m==MODE_RED)?1000 : (m==MODE_GREEN)?800 : 1200;  /* periods */
}

static inline void buzzer_apply(unsigned short period){
  if (!soundOn) { CCR1 = 0; return; }
  if (period < 50) period = 50;
  CCR0 = period - 1;        /* PWM base, also clocks LED PWM ISR */
  CCR1 = period >> 1;       /* 50% duty on TA0.1 -> P2.6 */
}

int main(void){
  configureClocks();
  buzzer_init();

  
  
  /* LEDs */
  P1DIR |= (RED|GREEN);
  P1OUT &= ~(RED|GREEN);

  /* Buttons */
  buttons_init();

  /* Timer A base + buzzer route (TA0.1 -> P2.6) */
  P2SEL2 &= ~(BIT6|BIT7);
  P2SEL  |=  BIT6;
  P2DIR  |=  BIT6;

  TACTL = TASSEL_2 | MC_1 | ID_0;   /* SMCLK, up mode */
  CCR0  = 1000-1;                   /* ~2 kHz when SMCLK≈2 MHz */
  CCTL0 = CCIE;                     /* TA0CCR0 interrupt on */
  CCTL1 = OUTMOD_7;                 /* reset/set */

  buzzer_apply(tone_for_mode(mode));

  /* WDT ~100 Hz for dim step + button polling */
  enableWDTInterrupts();
  __bis_SR_register(GIE);

  for(;;) __bis_SR_register(LPM0_bits);  /* sleep; ISRs run the toy */
}

/* ~100 Hz: fade 1 step per second; handle buttons */
void __interrupt_vec(WDT_VECTOR) WDT_ISR(void){
  static unsigned int ticks = 0;
  if (++ticks >= 100){                 /* ~1 s */
    ticks = 0;
    level = (level > 1) ? (level-1) : 16;   /* fade then reset */
  }

  unsigned char fell = buttons_fell();
  unsigned char pressed = buttons_pressed();

  
  if (fell & B0) { mode = MODE_RED;   buzzer_apply(tone_for_mode(mode)); }
  if (fell & B1) { mode = MODE_GREEN; buzzer_apply(tone_for_mode(mode)); }
  if (fell & B2) { mode = MODE_BOTH;  buzzer_apply(tone_for_mode(mode)); }
  /* play tone when button 4 is pressed */

  if (pressed & B3) buzzer_set_period(1000);

  else buzzer_set_period(0);
}

/* ~kHz fast tick: 16-step software PWM → ~125 Hz effective, no flicker */
void __interrupt_vec(TIMER0_A0_VECTOR) TA0CCR0_ISR(void){
  pwmStep = (pwmStep + 1) & 15;

  unsigned char dRed=0, dGreen=0;
  if (mode == MODE_RED)   { dRed = level; }
  if (mode == MODE_GREEN) { dGreen = level; }
  if (mode == MODE_BOTH)  { dRed = level; dGreen = level; }

  if (pwmStep < dRed)   P1OUT |= RED;   else P1OUT &= ~RED;
  if (pwmStep < dGreen) P1OUT |= GREEN; else P1OUT &= ~GREEN;
}

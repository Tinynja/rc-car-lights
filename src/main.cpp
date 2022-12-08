// TO CHANGE ON ATTINY13:
// OSCCAL
// DDRD -> DDRB
// PIND -> PINB
// PD5 -> PB0
// PD6 -> PB1
// PCICR -> GIMSK
// TIMER0_OVF_vect -> TIM0_OVF_vect

#define F_CPU 16000000UL

#define bit_is_set_bool(sfr, bit) (bit_is_set(sfr, bit) >> bit)

#include <avr/io.h>
#include <std>
#include <avr/interrupt.h>

unsigned long clk = 0;

/*
20MHz:
Clock step: 1/(20MHz/256) = 0.0128ms
1ms = 78.128 steps
2ms = 156.25 steps
*/
 
struct PWM_channel {
	uint8_t port : 6;
	uint8_t pin : 3;
	uint8_t min;
	uint8_t max;
	uint8_t start_time;
	uint8_t last_duration : 7;
	uint8_t last_state : 1;
};

struct PWM_channel ch[] = {	{78, 156, 0, 0, 0},
							{78, 156, 0, 0, 0}	};

int main(void) {
	// Output pins: PB0, PB1
	// Input pins: PB3, PB4
	DDRD = _BV(PD5) | _BV(PD6);
	
	// TIMER0: fast PWM, 0xFF top, no prescaler, TOV enabled, OC0A enabled, OC0B enabled
	TCCR0A = _BV(COM0A1) | _BV(COM0B1) | _BV(WGM00) | _BV(WGM01);
	TCCR0B = _BV(CS00);
	TIMSK0 = _BV(TOIE0);
	OCR0A =	OCR0B = 64;
	
	// PCINT0 and PCINT1 enabled
	PCICR = _BV(PCIE0);
	PCMSK0 = _BV(PCINT3) | _BV(PCINT4);
	
	sei();
}


ISR(TIMER0_OVF_vect) {
	clk++;
}


ISR(PCINT0_vect) {
	// Start timing on rising edge of PWM
	// Calculate duration on falling edge of PWM
	for (uint8_t i=0; sizeof(ch)/sizeof(ch[0]); i++) {
		if (!ch[i].last_state && bit_is_set_bool(PINB, PB4)) {
			clk++;
		} else if (ch[i].last_state && !bit_is_set_bool(PINB, PB4)) {
			clk--;
		}
	}
}
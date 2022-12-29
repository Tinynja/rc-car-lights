#ifndef CONFIG_H
#define CONFIG_H

// Microcontroller-specific pin and register configuration
#if defined(__AVR_ATtiny13__)
	#define F_CPU			16000000UL
	#define OSCCAL_VALUE	0x7D // OPTIONAL

	// LIGHTS_FRONT and LIGHTS_REAR must be the pins OCnA and OCnB associated
	// with the chosen TIMER_NUMBER (check MCU datasheet)
	#define TIMER_NUMBER	0 // MUST BE AN 8-bit TIMER
	#define LIGHTS_PORT		B
	#define LIGHTS_FRONT	0
	#define LIGHTS_REAR		1
	#define LIGHTS_LEFT		3
	#define LIGHTS_RIGHT	4

	// RX_CH1 and RX_CH2 must both be PCINT pins that are part of the same
	// PCMSK register (check MCU datasheet)
	#define RX_PORT		B
	#define RX_CH1		2
	#define RX_CH2		5
	#define PCINT_CH1	2
	#define PCINT_CH2	5

#elif defined(__AVR_ATtiny85__)
	#define F_CPU			16000000UL
	#define OSCCAL_VALUE	0x7D // OPTIONAL

	// LIGHTS_FRONT and LIGHTS_REAR must be the pins OCnA and OCnB associated
	// with the chosen TIMER_NUMBER (check MCU datasheet)
	#define TIMER_NUMBER	0 // MUST BE AN 8-bit TIMER
	#define LIGHTS_PORT		B
	#define LIGHTS_FRONT	0
	#define LIGHTS_REAR		1
	#define LIGHTS_LEFT		3
	#define LIGHTS_RIGHT	4

	// RX_CH1 and RX_CH2 must both be PCINT pins that are part of the same
	// PCMSK register (check MCU datasheet)
	#define RX_PORT		B
	#define RX_CH1		2
	#define RX_CH2		5
	#define PCINT_CH1	2
	#define PCINT_CH2	5

#elif defined(__AVR_ATmega328P__)
	#define F_CPU			16000000UL

	// LIGHTS_FRONT and LIGHTS_REAR must be the pins OCnA and OCnB associated
	// with the chosen TIMER_NUMBER (check MCU datasheet)
	#define TIMER_NUMBER	0 // MUST BE AN 8-bit TIMER
	#define LIGHTS_PORT		D
	#define LIGHTS_FRONT	6
	#define LIGHTS_REAR		5
	#define LIGHTS_LEFT		4
	#define LIGHTS_RIGHT	7
	
	// RX_CH1 and RX_CH2 must both be PCINT pins that are part of the same
	// PCMSK register (check MCU datasheet)
	#define RX_PORT		D
	#define RX_CH1		2
	#define RX_CH2		3
	#define PCINT_CH1	18
	#define PCINT_CH2	19

#else
	#error "Unsupported microcontroller! Please configure the pins/timers/registers/interrupts for it."
#endif

#endif //CONFIG_H
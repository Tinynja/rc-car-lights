#ifndef CONFIG_H
#define CONFIG_H

// Microcontroller-specific pin and register configuration
#if defined(__AVR_ATtiny13__)
	#define F_CPU			16000000UL
	#define OSCCAL_VALUE	0x7D // OPTIONAL

	#define TIMER_NUMBER	0 // MUST BE AN 8-bit TIMER

	#define RX_CH1			B,2
	#define RX_CH2			B,5
	#define LIGHTS_FRONT	B,0 // MUST BE OCnA PIN, WHERE n IS TIMER_NUMBER
	#define LIGHTS_REAR		B,1 // MUST BE OCnB PIN, WHERE n IS TIMER_NUMBER
	#define LIGHTS_LEFT		B,3
	#define LIGHTS_RIGHT	B,4

	// Check datasheet for PCINT# associated with pin for RX_CH1 & RX_CH2
	// Both pins must be part of the same PCMSK register.
	#define PCINT_CH1		2
	#define PCINT_CH2		5
#elif defined(__AVR_ATmega328P__)
	#define F_CPU			16000000UL

	#define TIMER_NUMBER	0 // MUST BE AN 8-bit TIMER

	#define RX_CH1			D,2
	#define RX_CH2			D,3
	#define LIGHTS_FRONT	D,6 // MUST BE OCnA PIN, WHERE n IS TIMER_NUMBER
	#define LIGHTS_REAR		D,5 // MUST BE OCnB PIN, WHERE n IS TIMER_NUMBER
	#define LIGHTS_LEFT		D,4
	#define LIGHTS_RIGHT	D,7

	// Check datasheet for PCINT# associated with pin for RX_CH1 & RX_CH2
	// Both pins must be part of the same PCMSK register.
	#define PCINT_CH1	18
	#define PCINT_CH2	19
#else
	#error "Unsupported microcontroller! Please configure the pins/timers/registers/interrupts for it."
#endif

#endif //CONFIG_H
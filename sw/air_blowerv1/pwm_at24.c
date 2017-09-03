
#define F_CPU 8000000UL
#include <util/delay.h>
#include <avr/io.h>

#include "pwm_at24.h"





void at24_pwmInit()
{
	
	TCCR0A |= (0x1 << COM0A1) | (0x1 << COM0B1) | (0x1 << WGM00);
	
	TCCR0B |= (0x1 << CS01); // 8 div
	
	
	DDRA |= (0x1 << PINA7);
	DDRB |= (0x1 << PINB2);
	
}

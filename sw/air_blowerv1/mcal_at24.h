#ifndef _MCAL_AT16_H
#define _MCAL_AT16_H 1

#include <avr/io.h>

/*LED defines*/
#define LED_PIN 	PINB1
#define LED_DRR		DDRB
#define LED_PORT	PORTB

/*TURBINE defines*/

#define TERMO_CS_CONF		DDRA |= 0x1
#define TERMO_SCK_CONF		DDRA |= 0x10
#define TERMO_SO_CONF		PORTA |= 0x20

#define TERMO_SCK_HIGH		PORTA |= 0x10
#define TERMO_SCK_LOW		PORTA &= 0xEF

#define TERMO_CS_HIGH		PORTA |= 0x1
#define TERMO_CS_LOW		PORTA &= 0xFE

#define TERMO_READ_SO		(uint8_t)(PINA >> 5) & 0x1

/*led macros*/
#define  SET_STATUS_LED			LED_DRR |= 0x1 << LED_PIN
#define  LED_STATUS_ON			LED_PORT &= ~(0x1 << LED_PIN)
#define  LED_STATUS_OFF			LED_PORT |= 0x1 << LED_PIN

#define  SET_ERROR_LED			DDRB |= 0x1
#define  LED_ERROR_ON			PORTB &= 0xFE
#define  LED_ERROR_OFF 			PORTB |= 0x1

#define SET_TERMO_LED			DDRA |= 0x8
#define	LED_TERMO_ON			PORTA &= 0xF7
#define LED_TERMO_OFF			PORTA |= 0x8


#define SET_BUTTON				PORTA |= 0x4
#define GET_BUTTON_INPUT		(uint8_t)((PINA & 0x4) >> 2)

/*Turbine macros*/

/*Senzor*/

#define GET_DEC_LVL (uint8_t)((PINA & 2u) >> 1)
#define SET_SENSOR_INPUT	PORTA |= 0x2

#endif /*_MCAL_AT16_H*/

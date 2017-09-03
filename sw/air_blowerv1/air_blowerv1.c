#include <avr/io.h>
#include "pwm_at24.h"

#define F_CPU 8000000UL
#define TEMP_START	5500

#include <util/delay.h>

#include "dbg_putchar.h"
#include "mcal_at24.h"

#define IDLE 		0x0
#define CHECK_SIG	0x3
#define START_RCV	0x1
#define DECODE_MSG	0x2

#define BUTTON_DOWN 0x4
#define BUTTON_UP_SHORT   0x5
#define BUTTON_UP_LONG	  0x6
#define TERMO_READING	  0x7


#define START_DEC_THRESHOLD 50
#define THOMSON_REPEAT_THR	600


#define CODE_NS_VERIFY	0x3
#define CODE_SH_VERIFY	0x6

#define CODE_NS_TURN_ON	0x1
#define CODE_SH_TURN_ON	0x2

#define CODE_NS_INCREASE	0x411
#define CODE_SH_INCREASE	0x822

#define CODE_NS_DECREASE	0x401
#define CODE_SH_DECREASE	0x802

#define CODE_NS_TERMO		0x511
#define CODE_SH_TERMO		0xA22


#define __DEBUG 0

void _sys_init(){

#if __DEBUG
	dbg_tx_init();
#endif

	SET_SENSOR_INPUT;
	SET_BUTTON;
	SET_STATUS_LED;
	SET_ERROR_LED;
	SET_TERMO_LED;

	TERMO_CS_CONF;
	TERMO_SO_CONF;
	TERMO_SCK_CONF;

	at24_pwmInit();

	SET_PWM_A(0);


	LED_ERROR_OFF;
	LED_TERMO_OFF;


	LED_STATUS_ON;
	_delay_ms(50);
	LED_STATUS_OFF;
	_delay_ms(50);
	LED_STATUS_ON;
	_delay_ms(100);
	LED_STATUS_OFF;

};




const uint8_t u8PwmVals[] = { 100, 175, 255 };
uint8_t signal_periods[64];
uint32_t signal_code = 0;
uint32_t signal_code2 = 0;
uint8_t  signal_shifter = 0;
uint8_t pwm_lvl = 0;
uint8_t pwm_isOn = 0;
uint8_t termocoupleOn = 0;
uint16_t termoCheckTimer = 0;
int8_t i = 0;
uint16_t temperatureRaw = 0;
uint8_t termoFail = 0;
uint16_t temperatureCel = 0;
#define MAX_TERMO_FAIL 3

uint8_t checkTermoTim = 0;

#define TERMO_CHECK_TIMEOUT (uint32_t)60000
#define TERMO_CHECK_TIMEOUT2	5

uint8_t button_lvl = 0;
uint8_t button_count = 0;
#define BUTTON_PRES_SHORT		3
#define BUTTON_PRES_LONG		15

#define PWM_MAX 3
#define PWM_INC 1

volatile uint16_t sig_idx = 0;
volatile uint8_t dec_state = IDLE;
volatile uint16_t hold_counter = 0;
//volatile uint8_t msg_counter = 0;
//volatile uint8_t msg_end_qalif = 0;

volatile uint8_t sig_lvl = 0;
volatile uint8_t sig_lvl_prev = 0;
volatile uint8_t idx = 0;









int main(){
	
	_sys_init();	

	
	while(1){


	
	
	sig_lvl = GET_DEC_LVL;
	button_lvl = GET_BUTTON_INPUT;

	termoCheckTimer++;
	

	switch(dec_state){
		
		case IDLE:
		{
			
			if(sig_lvl == 0){
				dec_state = CHECK_SIG;
				hold_counter = 0;
	
			
			}
			else if( button_lvl == 0 ){
				dec_state = BUTTON_DOWN;
				button_count = 0;
			}
			else if(termocoupleOn && (termoCheckTimer == TERMO_CHECK_TIMEOUT) ){

				termoCheckTimer = 0;


				if(checkTermoTim == TERMO_CHECK_TIMEOUT2){

					dec_state = TERMO_READING;
					checkTermoTim = 0;

				}
				checkTermoTim++;
			}
			else{
				dec_state = IDLE;
			}
		}
		break;

		case TERMO_READING:

			
			temperatureRaw = 0;
			temperatureCel = 0;
			TERMO_CS_LOW;

			_delay_ms(1);

			for(i = 15; i >= 0; i--){
				TERMO_SCK_LOW;
				_delay_ms(1);

				if( TERMO_READ_SO ){
					temperatureRaw |= 1 << i;
				}

				TERMO_SCK_HIGH;
				_delay_ms(1);

			}

			_delay_ms(1);

			TERMO_CS_HIGH;

			//no termocuple attached
			if(temperatureRaw & 0x4){
				termoFail++;
			}
			else{
				termoFail = 0;
			}

			if(termoFail == MAX_TERMO_FAIL){
								
				termocoupleOn = 0;
				LED_ERROR_ON;
				LED_TERMO_ON;
				_delay_ms(2000);
				LED_ERROR_OFF;
				LED_TERMO_OFF;
				SET_PWM_A(0);
				termoFail = 0;
			}
			
			temperatureRaw >>= 3;
			
#if __DEBUG
			dbg_putchar( (uint8_t)temperatureRaw);
			dbg_putchar( (uint8_t) temperatureRaw >> 8);
#endif		
			//maximum temperature for this scale
			if(temperatureRaw < 2600 ){
				temperatureCel = (uint16_t)temperatureRaw * (uint16_t)25;
				//temperature exceed 50 grades
				if(temperatureCel >= TEMP_START ){
					pwm_isOn = 1;
					SET_PWM_A(u8PwmVals[pwm_lvl]);
				}
				else{
					pwm_isOn = 0;
					SET_PWM_A(0);
				}
			}
			else{
					SET_PWM_A(0);
			}
			

			LED_TERMO_ON;
			_delay_ms(100);
			LED_TERMO_OFF;
			dec_state = IDLE;


		break;

		case BUTTON_DOWN:
		{

			if( button_lvl == 0 ){
				button_count++;
				_delay_ms(50);
			}
			else if( ( button_lvl == 1 ) && ( button_count >= BUTTON_PRES_LONG )){
				dec_state = BUTTON_UP_LONG;
			}
			else if(button_lvl == 1 && button_count >= BUTTON_PRES_SHORT){
				dec_state = BUTTON_UP_SHORT;
			}
			else{
				dec_state = IDLE;
			}



		}
		break;

		case BUTTON_UP_SHORT:
					if( ( ((uint8_t)pwm_lvl + PWM_INC) < (uint8_t)PWM_MAX)){
						pwm_lvl += PWM_INC;				
					}
					else{
						pwm_lvl = 0;				

					}

					if(pwm_isOn){
						SET_PWM_A(u8PwmVals[pwm_lvl]);
					}
					
					LED_STATUS_ON;
					_delay_ms(100);
					LED_STATUS_OFF;
					dec_state = IDLE;
			

		break;

		case BUTTON_UP_LONG:
					pwm_isOn ^= 0x1;
					termocoupleOn = 0;

					if(pwm_isOn){
							SET_PWM_A(u8PwmVals[0]);
							pwm_lvl = 0;

					}
					else{
						SET_PWM_A(0);
					}

					LED_STATUS_ON;
					_delay_ms(100);
					LED_STATUS_OFF;
					dec_state = IDLE;

		break;

		case CHECK_SIG:
		{
			
			if(sig_lvl == 0){
				hold_counter++;
			}
			else{
				dec_state = IDLE;
				hold_counter = 0;
			}
			
			if(hold_counter == START_DEC_THRESHOLD){
				dec_state = START_RCV;
				
				hold_counter = 0;
				sig_lvl = 0;
				idx = 0;
	//			msg_end_qalif = 0;


			}

		}	
		break;

		case START_RCV:
		{
		while(idx < 64){
		
			sig_lvl = GET_DEC_LVL;

			if((sig_lvl != sig_lvl_prev)  || (hold_counter == 5000)){

			
		
					if( hold_counter > 5000 ){
				
						signal_periods[idx] = 0;
					}
					else if( (hold_counter > 200) ){// && (hold_counter <= THOMSON_REPEAT_THR) ){
						signal_periods[idx] = 1;
					}
					else{
						signal_periods[idx] = 0;
					}
			
		

				idx++;
				hold_counter = 0;
			}
		
			hold_counter++;
			sig_lvl_prev = sig_lvl;


		}

		dec_state = DECODE_MSG;
		}
		break;

		case DECODE_MSG:
		{
		
		signal_code = 0;
		signal_code2 = 0;
		signal_shifter = 0;

		for(idx = 0; idx < 64; idx++){
			if(idx < 32){
				signal_code |= signal_periods[idx] << signal_shifter;
				signal_shifter++;

			}
			else if( idx == 32){
				signal_shifter = 0;
				signal_code2 |= signal_periods[idx] << signal_shifter;
			}
			else{
				signal_code2 |= signal_periods[idx] << signal_shifter;
				signal_shifter++;
			}
			
			signal_periods[idx] = 0;
			
		}

#if __DEBUG
		dbg_putchar( (uint8_t)signal_code);
		dbg_putchar( (uint8_t)(signal_code >> 8 ));
		dbg_putchar( (uint8_t)(signal_code >> 16 ));
		dbg_putchar( (uint8_t)(signal_code >> 24 ));

		dbg_putchar( (uint8_t)signal_code2);
		dbg_putchar( (uint8_t)(signal_code2 >> 8 ));
		dbg_putchar( (uint8_t)(signal_code2 >> 16 ));
		dbg_putchar( (uint8_t)(signal_code2 >> 24 ));
#endif
		
		
		if( ( signal_code == CODE_NS_VERIFY ) || ( signal_code == CODE_SH_VERIFY ) )
		{
			switch(signal_code2){
				case CODE_NS_TURN_ON:
				case CODE_SH_TURN_ON:

					pwm_isOn ^= 0x1;
					termocoupleOn = 0;

					if(pwm_isOn){
							SET_PWM_A(u8PwmVals[0]);
							pwm_lvl = 0;

					}
					else{
						SET_PWM_A(0);
					}

					LED_STATUS_ON;
					_delay_ms(100);
					LED_STATUS_OFF;

				break;

				case CODE_NS_INCREASE:
				case CODE_SH_INCREASE:



					if( ( ((uint8_t)pwm_lvl + PWM_INC) < (uint8_t)PWM_MAX) && pwm_isOn){
						pwm_lvl += PWM_INC;				
						SET_PWM_A(u8PwmVals[pwm_lvl]);

					}

					
					LED_STATUS_ON;
					_delay_ms(100);
					LED_STATUS_OFF;

					

				break;

				case CODE_NS_DECREASE:
				case CODE_SH_DECREASE:

					if( ( (int8_t)pwm_lvl - (int8_t)PWM_INC >= 0 ) && pwm_isOn){
						pwm_lvl -= PWM_INC;
						SET_PWM_A(u8PwmVals[pwm_lvl]);

					}

					LED_STATUS_ON;
					_delay_ms(100);
					LED_STATUS_OFF;

				break;

				case CODE_NS_TERMO:
				case CODE_SH_TERMO:

					termocoupleOn ^= 1;

					LED_STATUS_ON;
					_delay_ms(100);
					LED_STATUS_OFF;	
					
				break;	

				default:

					LED_ERROR_ON;
					_delay_ms(100);
					LED_ERROR_OFF;

				break;

			}


		}


		dec_state = IDLE;

		}
		break;
		default:
		{
		}
		break;
	}

	}

}

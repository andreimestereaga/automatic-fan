#ifndef _PWM_AT24_H
#define _PWM_AT24_H

#define CH_PWM0A 1
#define CH_PWM0B 2

#define SET_PWM_A(pwm) 	OCR0A = pwm;
#define SET_PWM_B(pwm) 	OCR0B = pwm;

extern void at24_pwmInit();


#endif //_PWM_AT24_H

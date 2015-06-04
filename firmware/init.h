#ifndef __INIT_H__
#define __INIT_H__

#include <avr/io.h>

#define O2_COEFF 209460000

#define SERVO1 0
#define SERVO2 1

void init_outputs();

void init_adc();

void init();

void set_servo(uint8_t servo, uint8_t value);

void set_brightness(uint8_t value);

void set_contrast(uint8_t value);

uint8_t is_emergency();



// int test_outputs()
// {
// 
	// PORTB |= (1 << 3) | (1 << 2); 
	// PORTC |= (1 << 0) | (1 << 1) | (1 << 2) | (1 << 3);
	// return 0;
// }


#define VALVE1_ON
#define VALVE1_OFF
#define VALVE2_ON
#define VALVE2_OFF
#define LED_VAVLE1_ON
#define LED_VAVLE1_OFF
#define LED_VAVLE2_ON
#define LED_VAVLE2_OFF
#define LED_ALERT_ON
#define LED_ALERT_OFF
#define BUZZER_ON
#define BUZZER_OFF
#define COMPRESSOR_IS_ON

#define MODE_SET_MIX 0
#define MODE_CALIBRATE 1
#define MODE_MINIXG 2
#define MODE_SETUP 3


#endif	// _INIT_H_

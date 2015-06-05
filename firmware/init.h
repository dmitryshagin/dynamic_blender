#ifndef __INIT_H__
#define __INIT_H__

#include <avr/io.h>
#include "pid.h"

#define O2_COEFF 209460000

#define SERVO1 0
#define SERVO2 1

typedef struct TARGET_MIX{
  uint8_t oxygen;
  uint8_t helium;
  uint16_t s1_target; //is calculated for desired mix. For Nitrox equals target_oxygen
  uint16_t s2_target;
} targetMix_t;

typedef struct SENSORS_DATA{	
	uint32_t s1_uV;
	uint32_t s2_uV; 
	uint32_t s1_O2;
	uint32_t s2_O2;
	int64_t	 s1_coeff;
	int64_t  s2_coeff;
} sensors_t;

typedef struct SYSTEM_CONFIG{
	uint16_t min_servo_1; 
	uint16_t min_servo_2;
	uint16_t max_servo_1;
	uint16_t max_servo_2;
	uint8_t max_servo1_percent;
	uint8_t max_servo2_percent;
	uint16_t servo_timer_period_icr_top;
	uint8_t brightness;
	uint8_t contrast;
} systemConfig_t;

typedef struct BUTTONS_STATUS{
	uint8_t buttonOK;
	uint8_t buttonCancel;
	uint8_t buttonPlus;
	uint8_t buttonMinus;
} buttonsStatus_t;

extern struct SYSTEM_CONFIG system_config;

void init_outputs();

void init_adc();

void init();

void set_servo(uint8_t servo, int16_t value);

void set_brightness(uint8_t value);

void set_contrast(uint8_t value);

uint8_t is_emergency();

void uart_init( void );

void save_eeprom_data();



// int test_outputs()
// {
// 
	// PORTB |= (1 << 3) | (1 << 2); 
	// PORTC |= (1 << 0) | (1 << 1) | (1 << 2) | (1 << 3);
	// return 0;
// }

// TODO - define everything below
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

//TODO - тут херня написана рандомная, надо реальный порт проверять
#define BUTTON_PLUS_PRESSED (PINB & (1 << 2))

#define MODE_SET_MIX 0
#define MODE_CALIBRATE 1
#define MODE_MINIXG 2
#define MODE_SETUP 3


#endif	// _INIT_H_

#ifndef __INIT_H__
#define __INIT_H__

#include <avr/io.h>
#include "pid.h"

#define O2_COEFF 209460000UL

#define SERVO1 0
#define SERVO2 1

typedef struct TARGET_MIX{
  uint16_t oxygen;
  uint32_t helium;
} targetMix_t;

typedef struct SENSORS_TARGET_MIX{
  uint32_t s1_target; //is calculated for desired mix. For Nitrox equals target_oxygen
  uint32_t s2_target;
} sensorsTargetMix_t;


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
	uint16_t servo_timer_period_icr_top;
	uint8_t brightness;
	uint8_t contrast;
	uint16_t oxygen_emergency_limit;
} systemConfig_t;

typedef struct BUTTONS_STATUS{
	uint8_t buttonPlus;
	uint8_t buttonMinus;
} buttonsStatus_t;

extern struct SYSTEM_CONFIG system_config;
extern struct TARGET_MIX target;

void init_outputs();

void init_adc();

void init();

void set_servo(uint8_t servo, int16_t value);

void set_brightness(uint8_t value);

void set_contrast(uint8_t value);

uint8_t check_emergency(uint16_t oxygen);

void uart_init( void );

void save_eeprom_data();

void save_target_to_eeprom();


#define VALVE1_ON 			(PORTB |=  (1<<PB2))
#define VALVE1_OFF 			(PORTB &= ~(1<<PB2))
#define VALVE2_ON 			(PORTB |=  (1<<PB3))
#define VALVE2_OFF 			(PORTB &= ~(1<<PB3))
#define LED_VAVLE1_ON 		(PORTC |=  (1<<PC2))
#define LED_VAVLE1_OFF 		(PORTC &= ~(1<<PC2))
#define LED_VAVLE2_ON	 	(PORTC |=  (1<<PC0))
#define LED_VAVLE2_OFF 		(PORTC &= ~(1<<PC0))
#define LED_ALERT_ON 		(PORTC |=  (1<<PC1))
#define LED_ALERT_OFF 		(PORTC &= ~(1<<PC1))
#define BUZZER_ON 			(PORTC |=  (1<<PC3))
#define BUZZER_OFF 			(PORTC &= ~(1<<PC3))
#define COMPRESSOR_IS_ON  	(!(PIND  & (1<<PD2))) 
#define FLOW_IS_ON  		(!(PIND  & (1<<PD3)))

#define BUTTON_PLUS_PRESSED 	(!(PINC & (1 << PC4)))
#define BUTTON_MINUS_PRESSED 	(!(PINC & (1 << PC5)))
#define BUTTON_ENTER_PRESSED 	(!(PINC & (1 << PC6)))
#define BUTTON_EXIT_PRESSED 	(!(PINC & (1 << PC7)))

#define ANY_BUTTON_PRESSED (BUTTON_PLUS_PRESSED||BUTTON_MINUS_PRESSED||BUTTON_ENTER_PRESSED||BUTTON_EXIT_PRESSED)
#define ALL_BUTTONS_RELEASED (!ANY_BUTTON_PRESSED)


#define MODE_EMERGENCY 1

#define MODE_CALIBRATE 10
#define MODE_MINIXG 11

#define MODE_SET_O2 20
#define MODE_SET_HE 21

#define MODE_SET_BRIGHTNESS 50
#define MODE_SET_CONTRAST 51
#define MODE_SET_EMERGENCY_LEVEL 52
#define MODE_SET_VALVE1 60
#define MODE_SET_VALVE2 61
#define MODE_RUN_TEST 70



// logic for menu:
// turn on, check & calibrate -> set mix -> mix
// calibrate -> set mix by "any key"
// mix -> emergency (if compressor turned off or O2 > 40%)
// mix -> set mix (if valves turned off)
// set mix -> mix (if valves turned on and only in main menu)
// set mix -> menu, on main switch turned off
// no tuning when mixing
// menu:
// -set O2 (default last, set on exit) - show 
// -set He (default 0, set only if selected (if selected - show last value, set on exit)
// -switch between O2 and He on short press of Enter/Exit
// -Longtap on Menu/Exit (if setting O2/He) - go to submenus, or go back to main menu
// -shorttap on menu/exit - navigate between menu items, += - set values, store on menu leave
// submenus: set brightness, contrast, rotate valve1, rotate valve2 (show O2 when rotating, valves turned on), run test

#endif	// _INIT_H_

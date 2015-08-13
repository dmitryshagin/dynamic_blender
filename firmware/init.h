#ifndef __INIT_H__
#define __INIT_H__

#include <avr/io.h>
#include "pid.h"

#define O2_COEFF 209460000UL

#define SERVO1 0
#define SERVO2 1

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
#define MODE_MIXING 11

#define MODE_SET_O2 20
#define MODE_SET_HE 21

#define MODE_SET_BRIGHTNESS 50
#define MODE_SET_CONTRAST 51
#define MODE_SET_EMERGENCY_LEVEL 52
#define MODE_SET_VALVE1 60
#define MODE_SET_VALVE2 61
#define MODE_RUN_TEST 70
#define MODE_START_CALIBRATE 71
#define MODE_SET_PID 81
#define MODE_SET_SERVO1_MIN 91
#define MODE_SET_SERVO1_MAX 92
#define MODE_SET_SERVO2_MIN 93
#define MODE_SET_SERVO2_MAX 94
#define MODE_SET_TIMER1 95

typedef struct TARGET_MIX{
  uint16_t oxygen;
  uint32_t helium;
  uint16_t real_oxygen;
  uint32_t real_helium;
} targetMix_t;

typedef struct SENSORS_TARGET_MIX{
  uint32_t s1_target; //is calculated for desired mix. For Nitrox equals target_oxygen
  uint32_t s2_target;
  uint8_t valve1_target;
  uint8_t valve2_target;
} sensorsTargetMix_t;


typedef struct SENSORS_DATA{	
	uint32_t s1_uV;
	uint32_t s2_uV; 
	uint32_t s1_O2;
	uint32_t s2_O2;
	int64_t	 s1_coeff;
	int64_t  s2_coeff;
} sensors_t;


typedef struct PID_FACTORS{	
	int16_t s1_p_factor;
	int16_t s1_i_factor;
	int16_t s1_d_factor;
	int16_t	s1_max_output;
	int16_t s2_p_factor;
	int16_t s2_i_factor;
	int16_t s2_d_factor;
	int16_t	s2_max_output;
} pid_factors_t;


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
extern struct SENSORS_TARGET_MIX sensors_target;
extern struct SENSORS_DATA s_data;
extern struct BUTTONS_STATUS buttons;
extern struct PID_FACTORS pid_factors;
extern uint32_t log_windows[2][10];
extern uint8_t  log_position[2];
extern struct PID_DATA pidData1; 
extern struct PID_DATA pidData2;



int8_t parseInt(char * input, uint8_t max_count,int *result,int min_allowed,int max_allowed);

uint8_t is_log_ok();

void init_outputs();

void init();

void set_servo(uint8_t servo, int16_t value);

void set_brightness(uint8_t value);

void set_contrast(uint8_t value);

uint8_t check_emergency();

void uart_init( void );

void test_outputs();

void save_eeprom_data();

void save_target_to_eeprom();

void save_pid_data_to_eeprom();

uint16_t get_uptime_seconds();

void reset_need_input();

uint8_t need_input();

uint8_t is_calibrated_values_ok();

uint8_t need_blink();

uint8_t need_buzz();

void reset_blink();

void reset_buzz();

void set_alert(uint8_t blink, uint8_t buzz);

void process_alert();

void check_alert();

uint32_t get_max_deviation();


#endif	// _INIT_H_

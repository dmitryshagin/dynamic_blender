#include <stdlib.h>
#include <avr/io.h>
#include <stdio.h>
#include <avr/interrupt.h>
#include "adc.h"
#include "init.h"
#include "uart.h"
#include "menu.h"



//COPYPASTED PID
// should store in EEPROM
#define K_P     1.00
#define K_I     0.00
#define K_D     0.00
struct PID_DATA pidData1; 
struct PID_DATA pidData2;
struct SENSORS_DATA sensors;

volatile uint8_t timer0_counter;
volatile uint16_t uptime_counter;


ISR(TIMER0_OVF_vect)
{
    timer0_counter++;
    if(timer0_counter>=9){
    	uptime_counter++;
    	timer0_counter=0;
    	check_adc_flags();
    }
    if(uptime_counter%50==0){
    	process_buttons();
    }
}

void process_adc_data()
{
	int64_t uV;
	char tmpstr[20];
	uV = (((int64_t)AD7793_ContinuousReadAvg(1) - 0x800000)*73125) / 0x800000;
    if(uV < 0){ uV = -uV; }
	
	if(adc_current_channel == AD7793_CH_AIN1P_AIN1M){
		s_data.s1_uV = uV;
		s_data.s1_O2 = s_data.s1_uV * s_data.s1_coeff / 10000;
		adc_current_channel = AD7793_CH_AIN2P_AIN2M;
		if(get_current_working_mode() == MODE_MIXING){
			// TODO - process PID 1
			//TODO - тут происходит какая-то жопа с размерностями и форматами
		      // int16_t inputValue = pid_Controller((uint16_t)oxygen1_target, (uint16_t)oxygen1_value, &pidData1);
		    // sprintf(tmpstr,"%8li",  inputValue);
			// LCDGotoXY(8,1);
			// LCDstring((uint8_t *)tmpstr,8);
		}
				
	      //should be limited to 0-100%
	      // set_servo(SERVO1, inputValue);

	}else{
		s_data.s2_uV = uV;
		s_data.s2_O2 = s_data.s2_uV * s_data.s2_coeff / 10000;
		adc_current_channel = AD7793_CH_AIN1P_AIN1M;
		// TODO - process PID 2
	}				
    adc_change_channel_and_trigger_delay(adc_current_channel);
}


int main(void)
{
	init();	
	set_current_working_mode(MODE_CALIBRATE);
	uart0_puts("Welcome to blender :)\r\n");

	pid_Init(K_P * SCALING_FACTOR, K_I * SCALING_FACTOR , K_D * SCALING_FACTOR , &pidData1);
	pid_Init(K_P * SCALING_FACTOR, K_I * SCALING_FACTOR , K_D * SCALING_FACTOR , &pidData2);

	sensors_target.s1_target=0;
	sensors_target.s2_target=0;

	for(;;){
		process_uart();
	    if(adc_ready>0){ process_adc_data(); }
	    process_menu_selection();

		if(uptime_counter%100==0){ // 10 times per cesond
			// putchar('F');//just for test
			// uart0_putc('F');
			process_menu_internal();
		}
			
    }
    return 0;
}
#include <stdlib.h>
#include <avr/io.h>
#include <stdio.h>
#include <avr/interrupt.h>
#include "adc.h"
#include "init.h"
#include "menu.h"
#include "uart_proto.h"


int16_t inputValue;
int16_t prev_input_s1;
int16_t prev_input_s2;
uint16_t corrected_target, corrected_current;
int64_t uV;


void process_adc_data()
{
	uV = (((int64_t)AD7793_ContinuousReadAvg(1) - 0x800000)*73125) / 0x800000;
    if(uV < 0){ uV = -uV; }
	
	if(adc_current_channel == AD7793_CH_AIN1P_AIN1M){
		s_data.s1_uV = uV;
		s_data.s1_O2 = s_data.s1_uV * s_data.s1_coeff / 10000;
        log_windows[0][log_position[0]] = s_data.s1_uV;
		adc_current_channel = AD7793_CH_AIN2P_AIN2M;
		if(get_current_working_mode() == MODE_MIXING){
			corrected_target = (uint16_t)(sensors_target.s1_target/100);
			corrected_current = (uint16_t)(s_data.s1_O2/100);
		    inputValue = pid_Controller(corrected_target, corrected_current, &pidData1);
		    if((prev_input_s1<inputValue) && (corrected_current >= corrected_target) ){
		    	inputValue = prev_input_s1;
		    }
		    prev_input_s1 = inputValue;
		    set_servo(SERVO1, inputValue);
		}
        if(++log_position[0]>9){log_position[0]=0;}
	}else{
		s_data.s2_uV = uV;
		s_data.s2_O2 = s_data.s2_uV * s_data.s2_coeff / 10000;
        log_windows[1][log_position[1]] = s_data.s2_uV;
		adc_current_channel = AD7793_CH_AIN1P_AIN1M;
		if(get_current_working_mode() == MODE_MIXING && (sensors_target.s1_target!=sensors_target.s2_target)){
		    uint16_t corrected_target = (uint16_t)(sensors_target.s2_target/100);
			uint16_t corrected_current = (uint16_t)(s_data.s2_O2/100);
			
		    inputValue = pid_Controller(corrected_target, corrected_current, &pidData1);
		    inputValue = -inputValue;
		    if((prev_input_s2<inputValue) && (corrected_current >= corrected_target) ){
		    	inputValue = prev_input_s2;
		    }
		    prev_input_s2 = inputValue;
		    set_servo(SERVO2, inputValue);
		}
        if(++log_position[1]>9){log_position[1]=0;}
	}				
    adc_change_channel_and_trigger_delay(adc_current_channel);
}


int main(void)
{
	init();	
	set_current_working_mode(MODE_CALIBRATE);
    set_countdown_timer(15);

	uart0_puts("Welcome to blender :)\r\n");
	for(;;){
		process_uart();
	    if(adc_ready>0){ 
            process_adc_data(); 
        }
	    process_menu_selection();

		if(need_input()){ // 10 times per second
			process_menu_internal();
			reset_need_input();
		}
		if(current_working_mode==MODE_MIXING||current_working_mode==MODE_SET_O2){
        	process_alert();
        }
			
    }
    return 0;
}
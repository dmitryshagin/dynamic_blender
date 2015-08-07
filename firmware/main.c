#include <stdlib.h>
#include <avr/io.h>
#include <stdio.h>
#include <avr/interrupt.h>
#include "adc.h"
#include "init.h"
#include "lcd.h"
#include "uart.h"
#include "menu.h"


//COPYPASTED PID
// should store in EEPROM
#define K_P     0.1
#define K_I     0.02
#define K_D     0.01
struct PID_DATA pidData1; 
struct PID_DATA pidData2;

int16_t inputValue;
int16_t prev_input_s1;
int16_t prev_input_s2;
uint16_t to_target = 0;

void process_adc_data()
{
	int64_t uV;
	
	uV = (((int64_t)AD7793_ContinuousReadAvg(1) - 0x800000)*73125) / 0x800000;
    if(uV < 0){ uV = -uV; }
	
	if(adc_current_channel == AD7793_CH_AIN1P_AIN1M){
		s_data.s1_uV = uV;
		s_data.s1_O2 = s_data.s1_uV * s_data.s1_coeff / 10000;
        log_windows[0][log_position[0]] = s_data.s1_uV;
		adc_current_channel = AD7793_CH_AIN2P_AIN2M;
		if(get_current_working_mode() == MODE_MIXING){
			uint16_t corrected_target = (uint16_t)(sensors_target.s1_target/100);
			uint16_t corrected_current = (uint16_t)(s_data.s1_O2/100);
			// if(corrected_current < corrected_target){
			// 	if( (corrected_target - corrected_current)>100 && to_target==2000){
			// 		corrected_target = corrected_current + 100;
			// 		to_target = 0;
			// 	}else{
			// 		to_target+=1;
			// 	}	
			// }
		    inputValue = pid_Controller(corrected_target, corrected_current, &pidData1);
		    if((prev_input_s1<inputValue) && (corrected_current >= corrected_target) ){
		    	inputValue = prev_input_s1;
		    }
		    // if((inputValue-prev_input_s1)>10){
		    // 	inputValue = prev_input_s1 + 2;
		    // }
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
		    inputValue = pid_Controller((uint16_t)(sensors_target.s2_target/100), (uint16_t)(s_data.s2_O2/100), &pidData2);
		    inputValue = -inputValue;
		    prev_input_s2 = inputValue;
		    set_servo(SERVO2, inputValue);
		}
		// if(get_current_working_mode() == MODE_MIXING){
		// 	inputValue = pid_Controller((int16_t)(target.oxygen/100), (int16_t)(s_data.s1_O2/100), &pidData2);
		// 	if(inputValue > 100){
		// 		inputValue = 100;
		// 	}
		// 	// inputValue/=10;

		// 	set_servo(SERVO1, inputValue);
		// 	set_servo(SERVO2, inputValue);
		// }	
        if(++log_position[1]>9){log_position[1]=0;}
	}				
    adc_change_channel_and_trigger_delay(adc_current_channel);
}


void process_uart(void){
	int c;
	c = uart0_getc();
    if ( c & UART_NO_DATA )
    {
        /* 
         * no data available from UART 
         */
    }
    else
    {
        /*
         * new data available from UART
         * check for Frame or Overrun error
         */
        if ( c & UART_FRAME_ERROR )
        {
            /* Framing Error detected, i.e no stop bit detected */
            uart0_puts("UART Frame Error: ");
        }
        if ( c & UART_OVERRUN_ERROR )
        {
            /* 
             * Overrun, a character already present in the UART UDR register was 
             * not read by the interrupt handler before the next character arrived,
             * one or more received characters have been dropped
             */
            uart0_puts("UART Overrun Error: ");
        }
        if ( c & UART_BUFFER_OVERFLOW )
        {
            /* 
             * We are not reading the receive buffer fast enough,
             * one or more received character have been dropped 
             */
            uart0_puts("Buffer overflow error: ");
        }
        /* 
         * send received character back
         */
         if ((uint8_t)c == '?'){
         	uart0_puts("TODO: Put some debug info");
         }
        uart0_putc( (unsigned char)c );
    }
}


int main(void)
{
	char tmpstr[25];
	init();	
	set_current_working_mode(MODE_CALIBRATE);
    set_countdown_timer(15);

	uart0_puts("Welcome to blender :)\r\n");

	pid_Init(K_P * SCALING_FACTOR, K_I * SCALING_FACTOR , K_D * SCALING_FACTOR , &pidData1);
	pid_Init(K_P * SCALING_FACTOR, K_I * SCALING_FACTOR , K_D * SCALING_FACTOR , &pidData2);

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
		if(need_output()){
		    uint8_t t_o2 = target.oxygen/1000UL;
		    // uint8_t curr_o2 = s_data.s1_O2/1000UL;
		    // uint8_t s_o2_target = sensors_target.s1_target/1000UL;
		    LCDGotoXY(0,0);
		    // sprintf(tmpstr,"S1: %02u, %02u, %02u, %03u\r\n",  t_o2, curr_o2, s_o2_target, sensors_target.valve1_target);
		    uint16_t curr_o2 = s_data.s1_O2/100UL;
		    
			sprintf(tmpstr,"%02u, %02u.%01u, %05d\r\n", t_o2, curr_o2/10, curr_o2%10, inputValue);
			uart0_puts(tmpstr);
			reset_need_output();	
		}	
		if(current_working_mode==MODE_MIXING||current_working_mode==MODE_SET_O2){
        	process_alert();
        }
			
    }
    return 0;
}
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
volatile uint8_t need_input;
volatile uint8_t need_output;


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
    if(uptime_counter%100==0){
		need_input = 1;		
	}	
    if(uptime_counter%250==0){
		need_output = 1;		
	}	
}

void process_adc_data()
{
	int64_t uV;
	int16_t inputValue;
	uV = (((int64_t)AD7793_ContinuousReadAvg(1) - 0x800000)*73125) / 0x800000;
    if(uV < 0){ uV = -uV; }
	
	if(adc_current_channel == AD7793_CH_AIN1P_AIN1M){
		s_data.s1_uV = uV;
		s_data.s1_O2 = s_data.s1_uV * s_data.s1_coeff / 10000;
		adc_current_channel = AD7793_CH_AIN2P_AIN2M;
		if(get_current_working_mode() == MODE_MIXING){
		    inputValue = pid_Controller((uint16_t)(sensors_target.s1_target/10), (uint16_t)(s_data.s1_O2/10), &pidData1);
		    set_servo(SERVO1, inputValue);
		}

	}else{
		s_data.s2_uV = uV;
		s_data.s2_O2 = s_data.s2_uV * s_data.s2_coeff / 10000;
		adc_current_channel = AD7793_CH_AIN1P_AIN1M;
		if(get_current_working_mode() == MODE_MIXING && (sensors_target.s1_target!=sensors_target.s2_target)){
		    inputValue = pid_Controller((uint16_t)(sensors_target.s2_target/10), (uint16_t)(s_data.s2_O2/10), &pidData2);
		    set_servo(SERVO2, inputValue);
		}
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
	uart0_puts("Welcome to blender :)\r\n");

	pid_Init(K_P * SCALING_FACTOR, K_I * SCALING_FACTOR , K_D * SCALING_FACTOR , &pidData1);
	pid_Init(K_P * SCALING_FACTOR, K_I * SCALING_FACTOR , K_D * SCALING_FACTOR , &pidData2);

	for(;;){
		process_uart();
	    if(adc_ready>0){ process_adc_data(); }
	    process_menu_selection();

		if(need_input){ // 10 times per cesond
			process_menu_internal();
			need_input = 0;
		}
		if(need_output){

		    uint8_t t_o2 = target.oxygen/1000UL;
		    uint8_t curr_o2 = s_data.s1_O2/1000UL;
		    uint8_t s_o2_target = sensors_target.s1_target/1000UL;
		    LCDGotoXY(0,0);
		    sprintf(tmpstr,"S1: %02u, %02u, %02u, %03u\r\n",  t_o2, curr_o2, s_o2_target, sensors_target.valve1_target);
			uart0_puts(tmpstr);
			need_output = 0;	
		}	
			
    }
    return 0;
}
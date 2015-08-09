#include <stdlib.h>
#include <string.h>
#include <avr/io.h>
#include <stdio.h>
#include <avr/interrupt.h>
#include "adc.h"
#include "init.h"
#include "lcd.h"
#include "uart.h"
#include "menu.h"
#define MAX_COMMAND_LENGTH 32


uint8_t buffer_pos=0;
int16_t inputValue;
int16_t prev_input_s1;
int16_t prev_input_s2;
uint16_t corrected_target, corrected_current;

uint8_t uart_reply_flag=0;
uint8_t got_command = 0;

char _tmpstr[100];


static char rxBuffer[MAX_COMMAND_LENGTH];


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

void reply_with_status(){
	uint8_t t_o2 = target.oxygen/1000UL;
    uint8_t t_he = target.helium/1000UL;
    uint16_t curr_s1 = s_data.s1_O2/100UL;
    uint16_t curr_s2 = s_data.s2_O2/100UL;
    uint8_t c_o2 = target.real_oxygen/1000UL;
    uint8_t c_he = target.real_helium/1000UL;

    uint8_t s_o2_target = sensors_target.s1_target/1000UL;
    uint8_t s_he_target = sensors_target.s2_target/1000UL;
    
	sprintf(_tmpstr,"< %02u/%02u, %02u/%02u, %02u, %02u, %02u.%01u, %02u.%01u, %03d, %03d\r\n", t_o2, t_he, c_o2, c_he, s_o2_target, s_he_target, curr_s1/10, curr_s1%10, curr_s2/10, curr_s2%10, sensors_target.valve1_target, sensors_target.valve2_target);
	uart0_puts(_tmpstr);
}

int parseInt(char * input, uint8_t max_count,int *result){
  *result=0;
  uint32_t buff;
  uint8_t negative=0;
  uint8_t i,j;
  if(input[0]=='-'){
    negative=1;
  }
  for(i=negative;i<max_count;i++){
    if(input[i]<0x30){
      return -1;
    }
    if(input[i]>0x39){
      return -1;
    }
    buff=(input[i]-0x30);
    for(j=max_count-1;j>i;j--){
      buff*=10;
    }
    *result+= buff;
  }
  if(negative){
    *result*=-1;
  }
  return 0;  
}

uint8_t process_set_brightness_uart(char * pch){
	pch = strtok (NULL, " ");
  	int res;
  	int brightness;
  	res=parseInt(pch,strlen(pch),&brightness);
  	if(res<0){
  		uart0_puts("<E! Brightness not set");
  	}else{
  		system_config.brightness=brightness;
  		set_brightness(brightness);
  		save_eeprom_data();
  		uart0_puts("<OK");
  	}
  	return 0;
}

uint8_t processCommand(uint16_t buffer_pos){
  if(buffer_pos>=MAX_COMMAND_LENGTH||rxBuffer[0]!='>'){
    return 0;
  }
  char * pch = strtok (rxBuffer," ");
  if(pch != NULL){
	if((strcmp(pch,">SB"))==0){
		process_set_brightness_uart(pch);
		return 1;
    }else if((strcmp(pch,">?"))==0){
    	reply_with_status();
    	return 1;
    }
  }
  return 0;
}


void process_uart(void){
	int c;
	uint8_t i;
	c = uart0_getc();
    if ( !(c & UART_NO_DATA) )
    {
        if ( c & UART_FRAME_ERROR ){uart0_puts("<!E1 UART Frame Error");}
        if ( c & UART_OVERRUN_ERROR ){uart0_puts("<!E2 UART Overrun Error");}
        if ( c & UART_BUFFER_OVERFLOW ){uart0_puts("<!E3 Buffer overflow error");}
        if ((uint8_t)c == '>'){
        	for(buffer_pos=MAX_COMMAND_LENGTH-1;buffer_pos>0;buffer_pos--){rxBuffer[buffer_pos]=0;}
        	rxBuffer[buffer_pos++]=c;
        }else if((c=='\0'||c=='\r')){
        	processCommand(buffer_pos);
	        for(i=0;i<MAX_COMMAND_LENGTH;i++){rxBuffer[i]=0;}
        }else{
        	rxBuffer[buffer_pos++]=c;
        }
        if(buffer_pos==MAX_COMMAND_LENGTH){ //reset buffer on erraty command
        	for(buffer_pos=MAX_COMMAND_LENGTH-1;buffer_pos>0;buffer_pos--){rxBuffer[buffer_pos]=0;}
        }
    }
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
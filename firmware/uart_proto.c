#include "uart_proto.h"
#include "init.h"
#include "uart.h"
#include "menu.h"
#include <string.h>
#include <stdio.h>
#include <avr/pgmspace.h>

#define MAX_COMMAND_LENGTH UART_RX0_BUFFER_SIZE


uint8_t buffer_pos=0;


static char rxBuffer[MAX_COMMAND_LENGTH];

static void reply_P(const char *addr){
    char c;
    while ((c = pgm_read_byte(addr++)))
    uart0_putc(c);
}

static void reply_OK(){
    reply_P(PSTR("<OK\n\r"));
}

static uint8_t process_set_brightness_uart(char * pch){
    int8_t res;
    int parsed_target;
	pch = strtok (NULL, " ");
  	res=parseInt(pch,strlen(pch),&parsed_target,0,0xFF);
  	if(res<0){
        reply_P(PSTR("<!E4 Brightness\n\r"));
        return 1;
  	}else{
  		system_config.brightness=(uint8_t)parsed_target;
  		set_brightness(system_config.brightness);
  		save_eeprom_data();
  		reply_OK();
  	}
  	return 0;
}

static uint8_t process_set_contrast_uart(char * pch){
    int8_t res;
    int parsed_target;
    pch = strtok (NULL, " ");
    res=parseInt(pch,strlen(pch),&parsed_target,0,0xFF);
    if(res<0){
        reply_P(PSTR("<!E5 Contrast\n\r"));
        return 1;
    }else{
        system_config.contrast=(uint8_t)parsed_target;
        set_contrast(system_config.contrast);
        save_eeprom_data();
        reply_OK();
    }
    return 0;
}

static uint8_t process_set_emergency_uart(char * pch){
    int8_t res;
    int parsed_target;
    pch = strtok (NULL, " ");
    res=parseInt(pch,strlen(pch),&parsed_target,21,65);
    if(res<0){
        reply_P(PSTR("<!E6 Emergency O2\n\r"));
        return 1;
    }else{
        system_config.oxygen_emergency_limit=(uint16_t)parsed_target*1000;
        save_eeprom_data();
        validate_o2_data();
        reply_OK();
    }
    return 0;
}

static uint8_t process_set_target_oxygen_uart(char * pch){
    int8_t res;
    int parsed_target;
    pch = strtok (NULL, " ");
    res=parseInt(pch,strlen(pch),&parsed_target,21,system_config.oxygen_emergency_limit/1000UL-5);
    if(res<0){
        reply_P(PSTR("<!E7 Oxygen\n\r"));
        return 1;
    }else{
        target.oxygen = (uint16_t)parsed_target*1000;
        validate_o2_data();
        save_target_to_eeprom();
        reply_OK();
    }
    return 0;
}

static uint8_t process_set_target_helium_uart(char * pch){
    int8_t res;
    int parsed_target;
    pch = strtok (NULL, " ");
    res=parseInt(pch,strlen(pch),&parsed_target,0,get_helium_limit()/1000UL);
    if(res<0){
        reply_P(PSTR("<!E8 Helium\n\r"));
        return 1;
    }else{
        target.helium = (uint32_t)parsed_target*1000;
        validate_o2_data();
        save_target_to_eeprom();
        reply_OK();
    }
    return 0;
}

static uint8_t process_set_valve1_uart(char * pch){
    int8_t res;
    int parsed_target;
    pch = strtok (NULL, " ");
    res=parseInt(pch,strlen(pch),&parsed_target,0,1);
    if(res<0){
        reply_P(PSTR("<!E9 Valve1\n\r"));
        return 1;
    }else{
        if(parsed_target==0){
            VALVE1_OFF;
            LED_VAVLE1_OFF;
        }else{
            VALVE1_ON;
            LED_VAVLE1_ON;
        }
        reply_OK();
    }
    return 0;
}

static uint8_t process_set_valve2_uart(char * pch){
    int8_t res;
    int parsed_target;
    pch = strtok (NULL, " ");
    res=parseInt(pch,strlen(pch),&parsed_target,0,1);
    if(res<0){
        reply_P(PSTR("<!E10 Valve2\n\r"));
        return 1;
    }else{
        if(parsed_target==0){
            VALVE2_OFF;
            LED_VAVLE2_OFF;
        }else{
            VALVE2_ON;
            LED_VAVLE2_ON;
        }
        reply_OK();
    }
    return 0;
}

static uint8_t process_set_servo1_uart(char * pch){
    int8_t res;
    int parsed_target;
    pch = strtok (NULL, " ");
    res=parseInt(pch,strlen(pch),&parsed_target,0,0xFF);
    if(res<0){
        reply_P(PSTR("<!E11 Servo1\n\r"));
        return 1;
    }else{
        set_servo(SERVO1,parsed_target);
        reply_OK();
    }
    return 0;
}

static uint8_t process_set_servo2_uart(char * pch){
    int8_t res;
    int parsed_target;
    pch = strtok (NULL, " ");
    res=parseInt(pch,strlen(pch),&parsed_target,0,0xFF);
    if(res<0){
        reply_P(PSTR("<!E12 Servo2\n\r"));
        return 1;
    }else{
        set_servo(SERVO2,parsed_target);
        reply_OK();
    }
    return 0;
}

static uint8_t process_set_pid1_p(char * pch){
    int8_t res;
    int parsed_target;
    pch = strtok (NULL, " ");
    res=parseInt(pch,strlen(pch),&parsed_target,-MAX_INT+1,MAX_INT);
    if(res<0){
        reply_P(PSTR("<!E13 PID1 P\n\r"));
        return 1;
    }else{
        pid_factors.s1_p_factor = parsed_target;
        save_pid_data_to_eeprom();
        reply_OK();
    }
    return 0;
}

static uint8_t process_set_pid1_i(char * pch){
    int8_t res;
    int parsed_target;
    pch = strtok (NULL, " ");
    res=parseInt(pch,strlen(pch),&parsed_target,-MAX_INT+1,MAX_INT);
    if(res<0){
        reply_P(PSTR("<!E14 PID1 I\n\r"));
        return 1;
    }else{
        pid_factors.s1_i_factor = parsed_target;
        save_pid_data_to_eeprom();
        reply_OK();
    }
    return 0;
}

static uint8_t process_set_pid1_d(char * pch){
    int8_t res;
    int parsed_target;
    pch = strtok (NULL, " ");
    res=parseInt(pch,strlen(pch),&parsed_target,-MAX_INT+1,MAX_INT);
    if(res<0){
        reply_P(PSTR("<!E15 PID1 D\n\r"));
        return 1;
    }else{
        pid_factors.s1_d_factor = parsed_target;
        save_pid_data_to_eeprom();
        reply_OK();
    }
    return 0;
}

static uint8_t process_set_pid2_p(char * pch){
    int8_t res;
    int parsed_target;
    pch = strtok (NULL, " ");
    res=parseInt(pch,strlen(pch),&parsed_target,-MAX_INT+1,MAX_INT);
    if(res<0){
        reply_P(PSTR("<!E16 PID2 P\n\r"));
        return 1;
    }else{
        pid_factors.s2_p_factor = parsed_target;
        save_pid_data_to_eeprom();
        reply_OK();
    }
    return 0;
}

static uint8_t process_set_pid2_i(char * pch){
    int8_t res;
    int parsed_target;
    pch = strtok (NULL, " ");
    res=parseInt(pch,strlen(pch),&parsed_target,-MAX_INT+1,MAX_INT);
    if(res<0){
        reply_P(PSTR("<!E17 PID2 I\n\r"));
        return 1;
    }else{
        pid_factors.s2_i_factor = parsed_target;
        save_pid_data_to_eeprom();
        reply_OK();
    }
    return 0;
}

static uint8_t process_set_pid2_d(char * pch){
    int8_t res;
    int parsed_target;
    pch = strtok (NULL, " ");
    res=parseInt(pch,strlen(pch),&parsed_target,-MAX_INT+1,MAX_INT);
    if(res<0){
        reply_P(PSTR("<!E18 PID2 D\n\r"));
        return 1;
    }else{
        pid_factors.s2_d_factor = parsed_target;
        save_pid_data_to_eeprom();
        reply_OK();
    }
    return 0;
}


static uint8_t process_set_pid1_m(char * pch){
    int8_t res;
    int parsed_target;
    pch = strtok (NULL, " ");
    res=parseInt(pch,strlen(pch),&parsed_target,0,255);
    if(res<0){
        reply_P(PSTR("<!E24 PID1 MAX\n\r"));
        return 1;
    }else{
        pid_factors.s1_max_output = parsed_target;
        save_pid_data_to_eeprom();
        reply_OK();
    }
    return 0;
}

static uint8_t process_set_pid2_m(char * pch){
    int8_t res;
    int parsed_target;
    pch = strtok (NULL, " ");
    res=parseInt(pch,strlen(pch),&parsed_target,0,255);
    if(res<0){
        reply_P(PSTR("<!E25 PID2 MAX\n\r"));
        return 1;
    }else{
        pid_factors.s2_max_output = parsed_target;
        save_pid_data_to_eeprom();
        reply_OK();
    }
    return 0;
}

static uint8_t process_set_servo1_min(char * pch){
    int8_t res;
    int parsed_target;
    pch = strtok (NULL, " ");
    res=parseInt(pch,strlen(pch),&parsed_target,0,16000);
    if(res<0){
        reply_P(PSTR("<!E19 Servo1 min\n\r"));
        return 1;
    }else{
        system_config.min_servo_1=(uint16_t)parsed_target;
        save_eeprom_data();
        init_outputs();
        set_servo(SERVO1,0);
        set_servo(SERVO2,0);
        reply_OK();
    }
    return 0;
}

static uint8_t process_set_servo2_min(char * pch){
    int8_t res;
    int parsed_target;
    pch = strtok (NULL, " ");
    res=parseInt(pch,strlen(pch),&parsed_target,0,16000);
    if(res<0){
        reply_P(PSTR("<!E20 Servo2 min\n\r"));
        return 1;
    }else{
       system_config.min_servo_2=(uint16_t)parsed_target;
        save_eeprom_data();
        init_outputs();
        set_servo(SERVO1,0);
        set_servo(SERVO2,0);

        reply_OK();
    }
    return 0;
}

static uint8_t process_set_servo1_max(char * pch){
    int8_t res;
    int parsed_target;
    pch = strtok (NULL, " ");
    res=parseInt(pch,strlen(pch),&parsed_target,0,16000);
    if(res<0){
        reply_P(PSTR("<!E21 Servo1 max\n\r"));
        return 1;
    }else{
        system_config.max_servo_1=(uint16_t)parsed_target;
        save_eeprom_data();
        init_outputs();
        set_servo(SERVO1,0xFF);
        set_servo(SERVO2,0);
        reply_OK();
    }
    return 0;
}

static uint8_t process_set_servo2_max(char * pch){
    int8_t res;
    int parsed_target;
    pch = strtok (NULL, " ");
    res=parseInt(pch,strlen(pch),&parsed_target,0,16000);
    if(res<0){
        reply_P(PSTR("<!E22 Servo2 max\n\r"));
        return 1;
    }else{
        system_config.max_servo_2=(uint16_t)parsed_target;
        save_eeprom_data();
        init_outputs();
        set_servo(SERVO1,0);
        set_servo(SERVO2,0xFF);
        reply_OK();
    }
    return 0;
}

static uint8_t process_set_timer1_period(char * pch){
    int8_t res;
    int parsed_target;
    pch = strtok (NULL, " ");
    res=parseInt(pch,strlen(pch),&parsed_target,0,16000);
    if(res<0){
        reply_P(PSTR("<!E23 Timer1 value\n\r"));
        return 1;
    }else{
        system_config.servo_timer_period_icr_top=(uint16_t)parsed_target;
        save_eeprom_data();
        init_outputs();
        set_servo(SERVO1,0);
        set_servo(SERVO2,0);
        reply_OK();
    }
    return 0;
}

static uint8_t process_get_config(char * pch){
    char _tmpstr[UART_TX0_BUFFER_SIZE];
    sprintf(_tmpstr,"<S1MIN:%04d\r\n<S2MIN:%04d\r\n<S1MAX:%04d\r\n<S2MAX:%04d\r\n<TIMER:%05d\r\n<BR:%03d\r\n<CT:%03d\r\n<O2EM:%02d\r\n", 
        system_config.min_servo_1,
        system_config.min_servo_2,
        system_config.max_servo_1,
        system_config.max_servo_2,
        system_config.servo_timer_period_icr_top,
        system_config.brightness,
        system_config.contrast,
        system_config.oxygen_emergency_limit/1000
        );
    uart0_puts(_tmpstr);
    return 0;
}

static uint8_t process_get_pid_config(char * pch){
    char _tmpstr[UART_TX0_BUFFER_SIZE];
    sprintf(_tmpstr,"<P1p:%03d\r\n<P1i:%03d\r\n<P1d:%03d\r\n<P2p:%03d\r\n<P2i:%03d\r\n<P2d:%03d\r\n", 
        pid_factors.s1_p_factor,pid_factors.s1_i_factor,pid_factors.s1_d_factor,
        pid_factors.s2_p_factor,pid_factors.s2_i_factor,pid_factors.s2_d_factor
        );
    uart0_puts(_tmpstr);
    return 0;
}


static uint8_t reply_with_status(){
    char _tmpstr[UART_TX0_BUFFER_SIZE];
    uint8_t t_o2 = target.oxygen/1000UL;
    uint8_t t_he = target.helium/1000UL;
    uint16_t curr_s1 = s_data.s1_O2/100UL;
    uint16_t curr_s2 = s_data.s2_O2/100UL;
    uint8_t c_o2 = target.real_oxygen/1000UL;
    uint8_t c_he = target.real_helium/1000UL;

    uint8_t s_o2_target = sensors_target.s1_target/1000UL;
    uint8_t s_he_target = sensors_target.s2_target/1000UL;
    
    sprintf(_tmpstr,"< %02u/%02u, %02u/%02u, %02u, %02u, %02u.%01u, %02u.%01u, %03d, %03d\r\n", 
            t_o2, t_he, c_o2, c_he, s_o2_target, s_he_target, 
            curr_s1/10, curr_s1%10, curr_s2/10, curr_s2%10, 
            sensors_target.valve1_target, sensors_target.valve2_target);
    uart0_puts(_tmpstr);
    return 0;
}

static uint8_t run_test_from_uart(){
    show_run_test();
    run_test();
    reply_OK();
    return 0;
}

static uint8_t run_calibration_from_uart(){
    set_current_working_mode(MODE_CALIBRATE);
    set_countdown_timer(30);
    reply_OK();
    return 0;
}



static uint8_t processCommand(uint16_t buffer_pos){
  if(buffer_pos>=MAX_COMMAND_LENGTH||rxBuffer[0]!='>'){
    return 0;
  }
  char * pch = strtok (rxBuffer," ");
  if(pch != NULL){
    if(buffer_pos<2){
        return 0;
    }
    if(rxBuffer[1]=='S'){ //SETTERS
        if((strcmp(pch,">SB"))==0){ //Brightness
            return process_set_brightness_uart(pch);
        }else if((strcmp(pch,">SC"))==0){ //Contrast
            return process_set_contrast_uart(pch);
        }else if((strcmp(pch,">SE"))==0){ //Emergency level
            return process_set_emergency_uart(pch);
        }else if((strcmp(pch,">SO2"))==0){ //Target Oxygen
            return process_set_target_oxygen_uart(pch);
        }else if((strcmp(pch,">SHe"))==0){ //Target Helium
            return process_set_target_helium_uart(pch);
        }else if((strcmp(pch,">SV1"))==0){ //Valve 1 position
            return process_set_valve1_uart(pch);
        }else if((strcmp(pch,">SV2"))==0){ //Valve 2 position
            return process_set_valve2_uart(pch);
        }else if((strcmp(pch,">SS1"))==0){ //Servo 1 position
            return process_set_servo1_uart(pch);
        }else if((strcmp(pch,">SS2"))==0){ //Servo 2 position
            return process_set_servo2_uart(pch);
        }else if((strcmp(pch,">SP1P"))==0){ //S1 PID P
            return process_set_pid1_p(pch);
        }else if((strcmp(pch,">SP1I"))==0){ //S1 PID I
            return process_set_pid1_i(pch);
        }else if((strcmp(pch,">SP1D"))==0){ //S1 PID D
            return process_set_pid1_d(pch);
        }else if((strcmp(pch,">SP1M"))==0){ //S1 PID D
            return process_set_pid1_m(pch);
        }else if((strcmp(pch,">SP2P"))==0){ //S2 PID P
            return process_set_pid2_p(pch);
        }else if((strcmp(pch,">SP2I"))==0){ //S2 PID I
            return process_set_pid2_i(pch);
        }else if((strcmp(pch,">SP2D"))==0){ //S2 PID D
            return process_set_pid2_d(pch);
        }else if((strcmp(pch,">SP2M"))==0){ //S1 PID D
            return process_set_pid2_m(pch);
        }else if((strcmp(pch,">SS1MIN"))==0){ //Servo 1 min
            return process_set_servo1_min(pch);
        }else if((strcmp(pch,">SS2MIN"))==0){ //Servo 2 min
            return process_set_servo2_min(pch);
        }else if((strcmp(pch,">SS1MAX"))==0){ //Servo 1 max
            return process_set_servo1_max(pch);
        }else if((strcmp(pch,">SS2MAX"))==0){ //Servo 2 max
            return process_set_servo2_max(pch);
        }else if((strcmp(pch,">STMR"))==0){ //Timer 1 period
            return process_set_timer1_period(pch);
        }
    }else if((strcmp(pch,">GCONF"))==0){ //Config
        return process_get_config(pch);
    }else if((strcmp(pch,">GPID"))==0){ //Pid config
        return process_get_pid_config(pch);    
    }else if((strcmp(pch,">?"))==0){ //Get current status
    	return reply_with_status();
    }else if((strcmp(pch,">RT"))==0){ //Run Test
        return run_test_from_uart();
    }else if((strcmp(pch,">RC"))==0){ //Run Calibration
        return run_calibration_from_uart();
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
        if ( c & UART_FRAME_ERROR ){uart0_puts("<!E1 Frame");}
        if ( c & UART_OVERRUN_ERROR ){uart0_puts("<!E2 Overrun");}
        if ( c & UART_BUFFER_OVERFLOW ){uart0_puts("<!E3 Overflow");}
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
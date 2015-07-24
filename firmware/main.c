#include <stdlib.h>
#include <avr/io.h>
#include <stdio.h>
#include <avr/interrupt.h>
#include <util/delay.h>
#include "lcd.h"
#include "adc.h"
#include "init.h"
#include "uart.h"



//COPYPASTED PID
// should store in EEPROM
#define K_P     1.00
#define K_I     0.00
#define K_D     0.00
struct PID_DATA pidData1; 
struct PID_DATA pidData2;
struct TARGET_MIX target; // should store in EEPROM
struct SENSORS_DATA sensors;
volatile struct BUTTONS_STATUS buttons;

/////////////

volatile uint8_t current_working_mode = MODE_CALIBRATE;

volatile uint8_t timer0_counter;
volatile uint16_t uptime_counter;

uint32_t oxygen1_uV, oxygen2_uV, oxygen1_value, oxygen2_value; //TODO - maybe extract that to structure?
int64_t coeff_sensor1, coeff_sensor2;
uint16_t oxygen1_target = 21000; //is calculated for desired mix. For Nitrox equals target_oxygen
uint16_t oxygen2_target;

// LCDprogressBar(i*3, 255, 16);

//TODO - pass buttons status by link
void process_buttons()
{
	if(BUTTON_PLUS_PRESSED){
		//0xFF - задержка перед повторным нажатием (надо вынести в EEPROM)
		//при анализе состояния смотрим, если 0 - не нажата, FF - зажата надолго, в промежутке - коротное нажатие
		//применять воздействие в диапазоне 1-FE можно только один раз (нужен наверное флаг дополнительный)
		if(buttons.buttonPlus < 0xFF){
			buttons.buttonPlus++;
		}
	}else{
		buttons.buttonPlus=0;
	}
	if(BUTTON_MINUS_PRESSED){
		if(buttons.buttonMinus < 0xFF){
			buttons.buttonMinus++;
		}
	}else{
		buttons.buttonMinus=0;
	}
}

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
	char tmpstr[8];
	uV = (((int64_t)AD7793_ContinuousReadAvg(1) - 0x800000)*73125) / 0x800000;
    if(uV < 0){ uV = -uV; }
	
	if(adc_current_channel == AD7793_CH_AIN1P_AIN1M){
		oxygen1_uV = uV;
		oxygen1_value = oxygen1_uV * coeff_sensor1 / 10000;
		adc_current_channel = AD7793_CH_AIN2P_AIN2M;
		if(current_working_mode == MODE_MINIXG){
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
		oxygen2_uV = uV;
		oxygen2_value = oxygen2_uV * coeff_sensor2 / 10000;
		adc_current_channel = AD7793_CH_AIN1P_AIN1M;
		// TODO - process PID 2
	}				
    adc_change_channel_and_trigger_delay(adc_current_channel);
}


	//TODO - limit O2 max for both channels (store in EEPROM)
    	// TODO - process uart input in some way
    	// int c = getchar();
	    // if( c != EOF ) {
	    //   putchar( c );
	    // }

void test_outputs(void)
{
	set_servo(SERVO1,0);
	set_servo(SERVO2,0);
	_delay_ms(500);
	set_servo(SERVO1,100);
	set_servo(SERVO2,100);
	_delay_ms(1000);
	set_servo(SERVO1,0);
	_delay_ms(1000);
	set_servo(SERVO2,0);
	_delay_ms(1000);
	VALVE1_ON;
	LED_VAVLE1_ON;
	_delay_ms(500);	
	VALVE2_ON;
	LED_VAVLE2_ON;
	_delay_ms(500);
	LED_ALERT_ON;
	BUZZER_ON;
	_delay_ms(500);
	VALVE1_OFF;
	VALVE2_OFF;	
	LED_VAVLE1_OFF;
	LED_VAVLE2_OFF;
	LED_ALERT_OFF;
	BUZZER_OFF; 
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

void show_run_test(){
	current_working_mode = MODE_RUN_TEST;
	VALVE1_OFF;
	VALVE2_OFF;
	LED_VAVLE1_OFF;
	LED_VAVLE2_OFF;
	LCDGotoXY(0,0);
	LCDstring("    Run Test   >",16);
	LCDGotoXY(0,1);
	LCDstring("               >",16);
}

void show_set_o2(){
	current_working_mode = MODE_SET_O2;
	VALVE1_OFF;
	VALVE2_OFF;
	LED_VAVLE1_OFF;
	LED_VAVLE2_OFF;
	LCDGotoXY(0,0);
	LCDstring("     Oxygen    +",16);
	LCDGotoXY(0,1);
	LCDstring("               -",16);
}

void show_set_he(){
	current_working_mode = MODE_SET_HE;
	VALVE1_OFF;
	VALVE2_OFF;
	LED_VAVLE1_OFF;
	LED_VAVLE2_OFF;
	LCDGotoXY(0,0);
	LCDstring("     Helium    +",16);
	LCDGotoXY(0,1);
	LCDstring("               -",16);
}

void show_set_brightness(){
	current_working_mode = MODE_SET_BRIGHTNESS;
	VALVE1_OFF;
	VALVE2_OFF;
	LED_VAVLE1_OFF;
	LED_VAVLE2_OFF;	
	LCDGotoXY(0,0);
	LCDstring("   Brightness  +",16);
	LCDGotoXY(0,1);
	LCDstring("               -",16);
}

void show_set_contrast(){
	current_working_mode = MODE_SET_CONTRAST;
	VALVE1_OFF;
	VALVE2_OFF;
	LED_VAVLE1_OFF;
	LED_VAVLE2_OFF;
	LCDGotoXY(0,0);
	LCDstring("    Contrast   +",16);
	LCDGotoXY(0,1);
	LCDstring("               -",16);
}

void show_set_warning_level(){
	current_working_mode = MODE_SET_WARNING_LEVEL;
	VALVE1_OFF;
	VALVE2_OFF;
	LED_VAVLE1_OFF;
	LED_VAVLE2_OFF;
	LCDGotoXY(0,0);
	LCDstring("   Warning O2  +",16);
	LCDGotoXY(0,1);
	LCDstring("               -",16);
}


void show_set_emergency_level(){
	current_working_mode = MODE_SET_EMERGENCY_LEVEL;
	VALVE1_OFF;
	VALVE2_OFF;
	LED_VAVLE1_OFF;
	LED_VAVLE2_OFF;
	LCDGotoXY(0,0);
	LCDstring("  Emergency O2 +",16);
	LCDGotoXY(0,1);
	LCDstring("               -",16);
}

void show_set_valve1(){
	current_working_mode = MODE_SET_VALVE1;
	VALVE1_ON;
	VALVE2_OFF;
	LED_VAVLE1_ON;
	LED_VAVLE2_OFF;
	LCDGotoXY(0,0);
	LCDstring("  Set O2 valve +",16);
	LCDGotoXY(0,1);
	LCDstring("               -",16);
}

void show_set_valve2(){
	current_working_mode = MODE_SET_VALVE2;
	VALVE1_OFF;
	VALVE2_ON;
	LED_VAVLE1_OFF;
	LED_VAVLE2_ON;
	LCDGotoXY(0,0);
	LCDstring("  Set He valve +",16);
	LCDGotoXY(0,1);
	LCDstring("               -",16);
}

void show_mixing(){
	current_working_mode = MODE_MINIXG;
	if(target.oxygen > 0){
		LED_VAVLE1_ON;
		VALVE1_ON;
	}
	if(target.helium > 0){
		LED_VAVLE2_ON;
		VALVE2_ON;
	}
	save_eeprom_data();
	LCDGotoXY(0,0);
	LCDstring("     Mixing     ",16);
}




int main(void)
{
	init();	
	// test_outputs();
	uart0_puts("Welcome to blender :)\r\n");


	pid_Init(K_P * SCALING_FACTOR, K_I * SCALING_FACTOR , K_D * SCALING_FACTOR , &pidData1);
	pid_Init(K_P * SCALING_FACTOR, K_I * SCALING_FACTOR , K_D * SCALING_FACTOR , &pidData2);

	char tmpstr[8];
	uint8_t mode_setup_iteration = 0;

	target.oxygen = 32000;

	for(;;){

		process_uart();
    
	    if(adc_ready>0){ process_adc_data(); }

	    if((current_working_mode == MODE_SET_O2 || current_working_mode == MODE_SET_HE) && COMPRESSOR_IS_ON && FLOW_IS_ON){
			show_mixing();
			mode_setup_iteration = 1;
		}else
		if(current_working_mode == MODE_MINIXG && (!COMPRESSOR_IS_ON || !FLOW_IS_ON) ){
			show_set_o2();
			mode_setup_iteration = 1;
		}

	    if(mode_setup_iteration==0 && ANY_BUTTON_PRESSED){
			if(current_working_mode == MODE_CALIBRATE && ANY_BUTTON_PRESSED && COMPRESSOR_IS_ON){
				show_set_o2();
				mode_setup_iteration = 1;
			}else
			if(current_working_mode == MODE_SET_O2 && (BUTTON_ENTER_PRESSED)){
				show_set_he();
				mode_setup_iteration = 1;
			}else
			if(current_working_mode == MODE_SET_HE && (BUTTON_EXIT_PRESSED)){
				show_set_o2();
				mode_setup_iteration = 1;
			}else
			if(current_working_mode == MODE_SET_BRIGHTNESS){
				if(BUTTON_ENTER_PRESSED){
					show_set_contrast();
					mode_setup_iteration = 1;
				}else 
				if(BUTTON_EXIT_PRESSED){
					show_run_test();
					mode_setup_iteration = 1;
				}
			}else
			if(current_working_mode == MODE_SET_CONTRAST){
				if(BUTTON_ENTER_PRESSED){
					show_set_warning_level();
					mode_setup_iteration = 1;
				}else 
				if(BUTTON_EXIT_PRESSED){
					show_set_brightness();
					mode_setup_iteration = 1;
				}
			}else
			if(current_working_mode == MODE_SET_WARNING_LEVEL){
				if(BUTTON_ENTER_PRESSED){
					show_set_emergency_level();
					mode_setup_iteration = 1;
				}else 
				if(BUTTON_EXIT_PRESSED){
					show_set_contrast();
					mode_setup_iteration = 1;
				}
			}else
			if(current_working_mode == MODE_SET_EMERGENCY_LEVEL){
				if(BUTTON_ENTER_PRESSED){
					show_set_valve1();
					mode_setup_iteration = 1;
				}else 
				if(BUTTON_EXIT_PRESSED){
					show_set_warning_level();
					mode_setup_iteration = 1;
				}
			}else
			if(current_working_mode == MODE_SET_VALVE1){
				if(BUTTON_ENTER_PRESSED){
					show_set_valve2();
					mode_setup_iteration = 1;
				}else 
				if(BUTTON_EXIT_PRESSED){
					show_set_emergency_level();
					mode_setup_iteration = 1;
				}
			}else
			if(current_working_mode == MODE_SET_VALVE2){
				if(BUTTON_ENTER_PRESSED){
					show_run_test();
					mode_setup_iteration = 1;
				}else 
				if(BUTTON_EXIT_PRESSED){
					show_set_valve1();
					mode_setup_iteration = 1;
				}
			}else
			if(current_working_mode == MODE_RUN_TEST){
				if(BUTTON_ENTER_PRESSED){
					show_set_brightness();
					mode_setup_iteration = 1;
				}else 
				if(BUTTON_EXIT_PRESSED){
					show_set_valve2();
					mode_setup_iteration = 1;
				}
			}
		}	

		if(!ANY_BUTTON_PRESSED){
			mode_setup_iteration = 0;
		}else{
			if(mode_setup_iteration == 1){
				if((current_working_mode >= 50 ) && (BUTTON_EXIT_PRESSED && BUTTON_ENTER_PRESSED)){
					show_set_o2();
					save_eeprom_data();
					mode_setup_iteration = 2;
				}	
			}	
			if(mode_setup_iteration == 1){
				if((current_working_mode == MODE_SET_HE || current_working_mode == MODE_SET_O2) && 
					(BUTTON_EXIT_PRESSED && BUTTON_ENTER_PRESSED)){
					show_set_brightness();
					mode_setup_iteration = 2;
				}
			}
			if(mode_setup_iteration==1){_delay_ms(50);} //just for some debounce	

		}



		

		if(uptime_counter%100==0){ // 10 times per cesond
			// putchar('F');//just for test
			// uart0_putc('F');
			if(current_working_mode==MODE_CALIBRATE){
		    	coeff_sensor1 = O2_COEFF/oxygen1_uV;
		    	coeff_sensor2 = O2_COEFF/oxygen2_uV;
		    	print_calibration_screen(oxygen1_uV, oxygen2_uV);
		    	//TODO - вынести всё это на указатели на структуру
		    }
		    if(current_working_mode==MODE_RUN_TEST){
		    	if(BUTTON_MINUS_PRESSED||BUTTON_PLUS_PRESSED){
		    		LCDGotoXY(15,0);
		    		LCDstring(" ",1);
		    		LCDGotoXY(0,1);
					LCDstring("   Checking...  ",16);
		    		test_outputs();
		    		LCDGotoXY(0,1);
					LCDstring("      DONE!     ",16);
					_delay_ms(1000);
					show_run_test();
		    	}
		    }
		    if(current_working_mode==MODE_SET_BRIGHTNESS){
		    	if(system_config.brightness>0 && buttons.buttonMinus>0){
		    		system_config.brightness-=1;
		    	}
		    	if(system_config.brightness<0xFF && buttons.buttonPlus>0){
		    		system_config.brightness+=1;
		    	}
		    	if(system_config.brightness>10 && buttons.buttonMinus==0xFF){
		    		system_config.brightness-=10;
		    	}
		    	if(system_config.brightness<0xF5 && buttons.buttonPlus==0xFF){
		    		system_config.brightness+=10;
		    	}
		    	sprintf(tmpstr,"%03u     ", system_config.brightness);
				LCDGotoXY(5,1);
				LCDstring((uint8_t *)tmpstr,8);
		    	set_brightness(system_config.brightness);
		    }
		    if(current_working_mode==MODE_SET_CONTRAST){
		    	if(system_config.contrast>0 && buttons.buttonMinus>0){
		    		system_config.contrast-=1;
		    	}
		    	if(system_config.contrast<0xFF && buttons.buttonPlus>0){
		    		system_config.contrast+=1;
		    	}
		    	if(system_config.contrast>10 && buttons.buttonMinus==0xFF){
		    		system_config.contrast-=10;
		    	}
		    	if(system_config.contrast<0xF5 && buttons.buttonPlus==0xFF){
		    		system_config.contrast+=10;
		    	}
		    	sprintf(tmpstr,"%03u     ", system_config.contrast);
				LCDGotoXY(5,1);
				LCDstring((uint8_t *)tmpstr,8);
		    	set_contrast(system_config.contrast);
		    }
		    if(current_working_mode==MODE_SET_O2){
				// target.s1_target = ((uint32_t)target.oxygen * 100UL) * 1000UL / (100 - target.helium);
				// target.s2_target = (uint16_t)target.oxygen * 1000;

				// LCDGotoXY(0,0);
				// LCDstring(" Oxygen ",8);
				// sprintf(tmpstr,"  %2li.%03li",  oxygen1_value/1000, oxygen1_value%1000);
				// LCDGotoXY(0,1);
				// LCDstring((uint8_t *)tmpstr,8);
				// LCDGotoXY(8,0);
				// LCDstring(" Helium ",8);
				// sprintf(tmpstr,"  %2li.%03li",  oxygen2_value/1000, oxygen2_value%1000);
				// LCDGotoXY(8,1);
				// LCDstring((uint8_t *)tmpstr,8);
		    }

		    if(current_working_mode==MODE_MINIXG){
		    	if(check_emergency((uint16_t)oxygen2_value)){
		    		current_working_mode = MODE_EMERGENCY;
		    		LED_ALERT_ON;
		    		BUZZER_ON;
			        VALVE1_OFF;
			        VALVE2_OFF;
			        LED_VAVLE1_OFF;
			        LED_VAVLE2_OFF;
			        LCDGotoXY(0,0);
				    LCDstring("   Emergency!   ",16);
				    LCDGotoXY(0,1);
				    if(COMPRESSOR_IS_ON){
				        LCDstring("  O2 too high!  ",16);
				    }else{
				        LCDstring("Compressor:  OFF",16);
				    }
		    	}else{
			    	// TODO - display some data, check valves and emergency states
					// sprintf(tmpstr,"%6liuV", oxygen1_uV);
					// LCDGotoXY(0,0);
					// LCDstring((uint8_t *)tmpstr,8);
					sprintf(tmpstr,"  %2li.%03li",  oxygen1_value/1000, oxygen1_value%1000);
					LCDGotoXY(0,1);
					LCDstring((uint8_t *)tmpstr,8);
				}
		    }
		}
			
    }
    return 0;
}
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
uint16_t oxygen1_target; //is calculated for desired mix. For Nitrox equals target_oxygen
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
}

ISR(TIMER0_OVF_vect)
{
    timer0_counter++;
    if(timer0_counter>=9){
    	uptime_counter++;
    	timer0_counter=0;
    	check_adc_flags();
    }

}

void process_adc_data()
{
	int64_t uV;
	uV = (((int64_t)AD7793_ContinuousReadAvg(1) - 0x800000)*73125) / 0x800000;
    if(uV < 0){ uV = -uV; }
	
	if(adc_current_channel == AD7793_CH_AIN1P_AIN1M){
		oxygen1_uV = uV;
		oxygen1_value = oxygen1_uV * coeff_sensor1 / 10000;
		adc_current_channel = AD7793_CH_AIN2P_AIN2M;
		// TODO - process PID 1
	      int16_t inputValue = pid_Controller(oxygen1_target, oxygen1_value, &pidData1);
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

int main(void)
{
	init();	
	puts( "Hello world\r\n" );

	//TODO - limit O2 max for both channels (store in EEPROM)

	pid_Init(K_P * SCALING_FACTOR, K_I * SCALING_FACTOR , K_D * SCALING_FACTOR , &pidData1);
	pid_Init(K_P * SCALING_FACTOR, K_I * SCALING_FACTOR , K_D * SCALING_FACTOR , &pidData2);

	char tmpstr[8];


	for(;;){

    	// TODO - process uart input in some way
    	// int c = getchar();
	    // if( c != EOF ) {
	    //   putchar( c );
	    // }
		    
		if(uptime_counter%50==0){
    		process_buttons();
    		//TODO - process inputs and buttons - 20 times per second are definitely enough
    		//we also need some mode for repetative presses - e.g. delay for 1 second before repeat
    	}

	    if(adc_ready>0){ process_adc_data(); }

		if(current_working_mode == MODE_CALIBRATE && uptime_counter>10000){
			current_working_mode = MODE_MINIXG;
		}

		if(uptime_counter%100==0){ // 10 times per cesond
			putchar('F');//just for test
			if(current_working_mode==MODE_CALIBRATE){
		    	coeff_sensor1 = O2_COEFF/oxygen1_uV;
		    	coeff_sensor2 = O2_COEFF/oxygen2_uV;
		    	print_calibration_screen(oxygen1_uV, oxygen2_uV);
		    	//TODO - вынести всё это на указатели на структуру
		    	//TODO - надо подождать, пока юзер включит компрессор (как показать?) и подождать после этого несколько секунд (как показать?)
		    	//без включения компрессора калибровку поидее надо запрещать
		    }
		    if(current_working_mode==MODE_SET_MIX){
				target.s1_target = ((uint32_t)target.oxygen * 100UL) * 1000UL / (100 - target.helium);
				target.s2_target = (uint16_t)target.oxygen * 1000;
		    }

		    if(current_working_mode==MODE_MINIXG){
		    	// TODO - display some data, check valves and emergency states
				sprintf(tmpstr,"%6liuV", oxygen1_uV);
				LCDGotoXY(0,0);
					LCDstring((uint8_t *)tmpstr,8);
					sprintf(tmpstr,"  %2li.%03li",  oxygen1_value/1000, oxygen1_value%1000);
					LCDGotoXY(0,1);
					LCDstring((uint8_t *)tmpstr,8);
				
		    }
		}
			
    }
    return 0;
}
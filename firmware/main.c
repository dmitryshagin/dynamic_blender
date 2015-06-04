#include <stdlib.h>
#include <avr/io.h>
#include <stdio.h>
#include <avr/pgmspace.h>
#include <avr/interrupt.h>
#include <util/delay.h>
#include "lcd.h"
#include "adc.h"
#include "init.h"



//Strings stored in AVR Flash memory
const uint8_t LCDprogress[] PROGMEM="Loading...\0";
const uint8_t LCDDiveTeam[] PROGMEM="DiveTeam\0";
const uint8_t LCDTrimix[] PROGMEM="Trimix 10/70\0";

volatile uint8_t current_working_mode = 0;

volatile uint8_t timer0_counter = 0;
volatile uint32_t uptime_counter = 0;

volatile uint8_t adc_ready, adc_prepare;
uint8_t adc_current_channel = AD7793_CH_AIN1P_AIN1M;

uint32_t oxygen1_uV, oxygen2_uV, oxygen1_value, oxygen2_value;
int64_t coeff_sensor1 = 0; //will store in EEPROM
int64_t coeff_sensor2 = 0; //will store in EEPROM
uint32_t oxygen1_target = 0; //will store in EEPROM
uint32_t oxygen2_target = 0;//will store in EEPROM

// LCDprogressBar(i*3, 255, 16);

void adc_change_channel_and_trigger_delay(uint8_t channel)
{
	adc_change_channel(channel, 0);
	adc_prepare = 1;
	adc_ready = 0;
}

ISR(TIMER0_OVF_vect)
{
    // keep a track of number of overflows
    timer0_counter++;
    if(timer0_counter>=9){
    	//happens every 1ms - to process buttons or other events
    	uptime_counter++;
    	timer0_counter=0;
    	if(adc_prepare>0){
    		adc_prepare++;
    		if(adc_prepare>=240){
	    		adc_ready = 1;
	    		adc_prepare = 0;
	    	}
    	}
    }

}

int main(void)
{
	init();	
	sei();

	char tmpstr[8];
	uint32_t total = 0; 
	int64_t uV, val;

	adc_current_channel = AD7793_CH_AIN1P_AIN1M;
    adc_change_channel_and_trigger_delay(adc_current_channel);
    current_working_mode = MODE_CALIBRATE;
    for(;;){
		    
	    if(adc_ready>0){
		    uV = (((int64_t)AD7793_ContinuousReadAvg(1) - 0x800000)*73125) / 0x800000;
		    if(uV < 0){ uV = -uV; }
			
			if(adc_current_channel == AD7793_CH_AIN1P_AIN1M){
				oxygen1_uV = uV;
				oxygen1_value = oxygen1_uV * coeff_sensor1 / 10000;
				adc_current_channel = AD7793_CH_AIN2P_AIN2M;
				// TODO - process PID 1
			}else{
				oxygen2_uV = uV;
				oxygen2_value = oxygen2_uV * coeff_sensor2 / 10000;
				adc_current_channel = AD7793_CH_AIN1P_AIN1M;
				// TODO - process PID 2
			}				
		    adc_change_channel_and_trigger_delay(adc_current_channel);
		}

			if(current_working_mode == MODE_CALIBRATE && uptime_counter>10000){
				current_working_mode = MODE_MINIXG;
			}

			if(uptime_counter%25==0){
				if(current_working_mode==MODE_CALIBRATE){
			    	coeff_sensor1 = O2_COEFF/oxygen1_uV;
			    	coeff_sensor2 = O2_COEFF/oxygen2_uV;
			    	print_calibration_screen(oxygen1_uV, oxygen2_uV);
			    	//TODO - надо подождать, пока юзер включит компрессор (как показать?) и подождать после этого несколько секунд (как показать?)
			    	//без включения компрессора калибровку поидее надо запрещать
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
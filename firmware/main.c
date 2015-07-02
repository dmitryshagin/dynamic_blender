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
		      int16_t inputValue = pid_Controller((uint16_t)oxygen1_target, (uint16_t)oxygen1_value, &pidData1);
		    sprintf(tmpstr,"%8li",  inputValue);
			LCDGotoXY(8,1);
			LCDstring((uint8_t *)tmpstr,8);
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

	_delay_ms(1000);
	set_servo(SERVO1,100);
	_delay_ms(1000);
	set_servo(SERVO2,100);

	_delay_ms(1000);
	set_servo(SERVO1,0);
	_delay_ms(1000);
	set_servo(SERVO2,0);
	_delay_ms(1000);
	VALVE1_ON;
	_delay_ms(1000);	
	VALVE2_ON;
	_delay_ms(1000);
	LED_VAVLE1_ON;
	_delay_ms(500);
	LED_VAVLE2_ON;
	_delay_ms(500);
	LED_ALERT_ON;
	_delay_ms(500);
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

int main(void)
{
	init();	
	test_outputs();
	uart0_puts("Welcome to blender :)\r\n");


	pid_Init(K_P * SCALING_FACTOR, K_I * SCALING_FACTOR , K_D * SCALING_FACTOR , &pidData1);
	pid_Init(K_P * SCALING_FACTOR, K_I * SCALING_FACTOR , K_D * SCALING_FACTOR , &pidData2);

	char tmpstr[8];


	for(;;){

		process_uart();

		if(buttons.buttonPlus>0){
			if(buttons.buttonPlus==0xFF){
				LED_VAVLE1_ON;
			}
			LED_ALERT_ON;
		}else{
			LED_ALERT_OFF;
			LED_VAVLE1_OFF;
		}
    
	    if(adc_ready>0){ process_adc_data(); }

		if(current_working_mode == MODE_CALIBRATE && uptime_counter>10000){
			// current_working_mode = MODE_MINIXG;
		}

		if(uptime_counter%100==0){ // 10 times per cesond
			// putchar('F');//just for test
			uart0_putc('F');
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
				// sprintf(tmpstr,"%6liuV", oxygen1_uV);
				// LCDGotoXY(0,0);
					// LCDstring((uint8_t *)tmpstr,8);
					// sprintf(tmpstr,"  %2li.%03li",  oxygen1_value/1000, oxygen1_value%1000);
					// LCDGotoXY(0,1);
					// LCDstring((uint8_t *)tmpstr,8);
				
		    }
		}
			
    }
    return 0;
}
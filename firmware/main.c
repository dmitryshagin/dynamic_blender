#include <stdlib.h>
#include <avr/io.h>
#include <stdio.h>
#include <avr/pgmspace.h>
#include <avr/interrupt.h>
#include <util/delay.h>
#include "lcd.h"
#include "adc.h"





//Strings stored in AVR Flash memory
const uint8_t LCDprogress[] PROGMEM="Loading...\0";
const uint8_t LCDDiveTeam[] PROGMEM="DiveTeam\0";
const uint8_t LCDTrimix[] PROGMEM="Trimix 10/70\0";


volatile uint8_t timer0_counter = 0;
volatile uint32_t uptime_counter = 0;
volatile uint8_t adc_ready=0;
uint8_t adc_current_channel = AD7793_CH_AIN1P_AIN1M;
volatile uint8_t adc_prepare=0;

//demonstration of progress bar
void progress(void)
{
	uint8_t i=0;
	LCDclr();
	CopyStringtoLCD(LCDprogress, 3, 0);	
	for(i=0;i<86;i++)
		{
			LCDGotoXY(0, 1);
			LCDprogressBar(i*3, 255, 16);
		}
}

int Wait()
{
	uint8_t i=0;
	for(i=0;i<50;i++){
		_delay_ms(20);
	}

	return 0;
}

int init_servo()
{
	TCCR1A|=(1<<COM1A1)|(1<<COM1B1)|(1<<WGM11);        //NON Inverted PWM
    TCCR1B|=(1<<WGM13)|(1<<WGM12)|(1<<CS11)|(1<<CS10); //PRESCALER=64 MODE 14(FAST PWM)

    ICR1=5759;  //fPWM=50Hz (Period = 20ms Standard).
    // Timer period - 3.4722uS

    DDRD|=(1<<PD4)|(1<<PD5);   //PWM Pins as Out

    OCR1A=95;   //0 degree (0.388ms)
    OCR1B=95;
    return 0;
}

int init_outputs()
{
	DDRB |= (1 << 3) | (1 << 2); 
	DDRC |= (1 << 0) | (1 << 1) | (1 << 2) | (1 << 3);	
	DDRD |= (1 << 6) | (1 << 7); //Contrast & Brightness

	TCCR2A |= (1 << WGM20 | 1 << WGM21) | (1 << COM2A1) | (1 << COM2B1);
	TCCR2B |= (1 << CS21) | (1 << CS22);


	OCR2A = 0x80; //BRIGHTNESS
	OCR2B = 165; //CONTRAST (set xFF by default)
    
	return 0;
}

int test_servo()
{
	init_servo();
	OCR1A=95;   //0 degree (0.388ms)
	OCR1B=660;
    _delay_ms(500);

    OCR1A=378;  //90 degree (1.264ms)
    _delay_ms(500);
      
    OCR1A=660;  //180 degree (2.140ms) 
    OCR1B=95;
    _delay_ms(500);
    OCR1A=95;
    OCR1B=95;
      
	return 0;
}

int adc_init_channel(uint8_t channel, uint8_t recalibrate)
{
	unsigned long command = 0;
    
    
    // command = AD7793_GetRegisterValue(AD7793_REG_CONF,
                                      // 2,
                                      // 1); // CS is modified by SPI read/write functions.
    // command |= AD7793_CONF_UNIPOLAR;
    command |= AD7793_CONF_REFSEL(AD7793_REFSEL_INT);
    // command |= AD7793_CONF_VBIAS(1);
    command |= AD7793_CONF_BOOST;
    command |= AD7793_CONF_BUF;
    command |= AD7793_CONF_CHAN(channel);
    command |= AD7793_CONF_GAIN(AD7793_GAIN_16);
    AD7793_SetRegisterValue(
            AD7793_REG_CONF,
            command,
            2,
            1);
	
	
	if(recalibrate==1)
	{
		AD7793_WaitRdyGoLow();
		// _delay_ms(500);
	
	    AD7793_Calibrate(AD7793_MODE_CAL_INT_ZERO,
	                     channel);      // Internal Zero-Scale Calibration
	    AD7793_Calibrate(AD7793_MODE_CAL_INT_FULL,
	                     channel);      // Internal Full-Scale Calibration
	    AD7793_SetRegisterValue(AD7793_REG_MODE,
                            0x9, 
                            2,
                            1);
	}
	else
	{
		adc_prepare = 1;
		adc_ready = 0;
	}
	
}	

int init_adc()
{
	LCDclr();
	LCDGotoXY(3,0);	
	LCDstring("Initing...",10);
	AD7793_Reset();

	if(!AD7793_Init())
	{
		LCDclr();
		LCDGotoXY(2,0);
		LCDstring("ADC Error!",10);		
		return -1;
	}
	
	LCDGotoXY(0,1);	

	adc_init_channel(AD7793_CH_AIN1P_AIN1M, 1);
	adc_init_channel(AD7793_CH_AIN2P_AIN2M, 1);	
	LCDclr();

   	return 0;	
}

int init_timers()
{
	// set up timer with prescaler = 1024
    TCCR0B |= (1 << CS01);
    TCNT0 = 0;
    TIMSK0 |= 0x01;
	return 0;
}

int init()
{
	init_outputs();
	LCDinit();//init LCD bit, dual line, cursor right
	LCDclr();//clears LCD

	init_servo();
	init_adc();
	init_timers();

	return 0;
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

int test_outputs()
{

	PORTB |= (1 << 3) | (1 << 2); 
	PORTC |= (1 << 0) | (1 << 1) | (1 << 2) | (1 << 3);

	// PORTD |= (1 << 6) | (1 << 7);

	return 0;
}

int main(void)
{
	init();	
	sei();
	int8_t direction = 1;

	// AD7793_ContinuousReadAvg(1); 
	// progress();

	test_outputs();

	test_servo();


	char tmpstr[8];
	uint32_t total = 0; 
	adc_current_channel = AD7793_CH_AIN1P_AIN1M;
    adc_init_channel(adc_current_channel,0); //to launch interrupt-based process
    for(;;){
	  
		    LCDGotoXY(0,0);
		    int64_t value = 0;
		    if(adc_ready>0){
		    	// _delay_ms(50);
		    	//TODO - сюда же запихивать PID будем
		    	value = AD7793_ContinuousReadAvg(1);
			    // value = AD7793_SingleConversion();
			    sprintf(tmpstr,"%.8lu",value);
				
				if(adc_current_channel == AD7793_CH_AIN1P_AIN1M){
					LCDGotoXY(0,0);
					adc_current_channel = AD7793_CH_AIN2P_AIN2M;
				}else{
					LCDGotoXY(0,1);
					adc_current_channel = AD7793_CH_AIN1P_AIN1M;
				}
				
				adc_init_channel(adc_current_channel,0);

				// sprintf(tmpstr,"%6liuV",value * (11700000 / 8) / (0xFFFFFF/2));

				// sprintf(tmpstr,"%6liuV", ((value - 0x800000)*(146250/2) / 0x800000 ));

				sprintf(tmpstr,"%6liuV", (((value - 0x800000)*(146250/2) / 0x800000 )) * 1000 / 488 );
				// 0.48803

				// Code = 2(N – 1) × [(AIN × GAIN /VREF) + 1] 
				
				// Code = (2N × AIN × GAIN)/VREF
				// Code * VREF = 2^24 * AIN * GAIN
				// AIN = Code * VREF/ (2^24 * GAIN)
			
				LCDstring((uint8_t *)tmpstr,8);

				
				if(adc_current_channel == AD7793_CH_AIN2P_AIN2M){
					LCDGotoXY(10,0);
					sprintf(tmpstr,"%6li ",total++);
					
					LCDstring((uint8_t *)tmpstr,8);
				}
		    }
    
			if(uptime_counter%10==0){

				if(OCR1A<660||OCR1A>95){
					OCR1A+=(direction*2);
					if(OCR1A>660||OCR1A<95){
						direction=-direction;
					}
				}
				LCDGotoXY(10,1);
				sprintf(tmpstr,"%6li ",uptime_counter/100);
				
				LCDstring((uint8_t *)tmpstr,8);
			}
			

		    // LCDsendChar(0x30+(uint8_t)value); 
        // char i;
        // for(i = 0; i < 30; i++){
        //     _delay_ms(30);  /* max is 262.14 ms / F_CPU in MHz */
        // }
        // PORTB ^= 1 << 3;    /* toggle the LED */
    }
    return 0;               /* never reached */
}
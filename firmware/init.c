#include <avr/interrupt.h>
#include <avr/eeprom.h>
#include <stdio.h>
#include "init.h"
#include "lcd.h"
#include "adc.h"
#include "uart.h"

struct SYSTEM_CONFIG nv_system_config EEMEM = {95,95,660,660,100,100,5759,0x80,0xFF};
struct SYSTEM_CONFIG system_config;


//it shuld be stored in EEPROM
void load_eeprom_data()
{
    eeprom_read_block((void*)&system_config, (const void*)&nv_system_config, sizeof(system_config));
}

void save_eeprom_data()
{
    eeprom_write_block(&system_config, &nv_system_config, sizeof(system_config));
}

void init_outputs()
{
	DDRB |= (1 << 3) | (1 << 2); 
	DDRC |= (1 << 0) | (1 << 1) | (1 << 2) | (1 << 3);	
	DDRD |= (1 << 6) | (1 << 7); //Contrast & Brightness

	TCCR2A |= (1 << WGM20 | 1 << WGM21) | (1 << COM2A1) | (1 << COM2B1);
	TCCR2B |= (1 << CS21) | (1 << CS22);


	OCR2A = system_config.brightness; //BRIGHTNESS
	OCR2B = system_config.contrast; //CONTRAST (set xFF by default)

	// set up timer with prescaler = 1024
    TCCR0B |= (1 << CS01);
    TCNT0 = 0;
    TIMSK0 |= 0x01;
	
	//servo
	TCCR1A|=(1<<COM1A1)|(1<<COM1B1)|(1<<WGM11);        //NON Inverted PWM
    TCCR1B|=(1<<WGM13)|(1<<WGM12)|(1<<CS11)|(1<<CS10); //PRESCALER=64 MODE 14(FAST PWM)

    ICR1=system_config.servo_timer_period_icr_top;  //fPWM=50Hz (Period = 20ms Standard).
    // Timer period - 3.4722uS

    DDRD|=(1<<PD4)|(1<<PD5);   //PWM Pins as Out

    OCR1A=system_config.min_servo_1;   //0 degree (0.388ms)
    OCR1B=system_config.min_servo_2;
}

void set_brightness(uint8_t value)
{
    OCR2A = value;
}

void set_contrast(uint8_t value)
{
    OCR2B = value;
}

void init_adc()
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
        return;
    }
    
    LCDGotoXY(0,1); 
    LCDstring("C1...",5);
    adc_init_channel(AD7793_CH_AIN1P_AIN1M);
    LCDstring("OK  C2...",9);
    adc_init_channel(AD7793_CH_AIN2P_AIN2M);    
    LCDstring("OK",2);
    adc_current_channel = AD7793_CH_AIN1P_AIN1M;
    adc_change_channel_and_trigger_delay(adc_current_channel);
    LCDclr();
}


void init()
{
    load_eeprom_data();
    init_outputs();
    LCDinit();//init LCD bit, dual line, cursor right
    LCDclr();//clears LCD
    init_adc();
    uart0_init(UART_BAUD_SELECT(F_CPU, 115200UL));
    FILE uart_stream = FDEV_SETUP_STREAM(uart0_putc, uart0_getc, _FDEV_SETUP_RW);
    stdout = stdin = &uart_stream;
    sei();
}


void set_servo(uint8_t servo, int16_t value)
{
    if(value>100)
    {
        value = 100;
    }
    if(value<0)
    {
        value = 0;
    }
    if(servo==SERVO1)
    {
        OCR1A = system_config.min_servo_1 + (system_config.max_servo_1 - system_config.min_servo_1)*100/value;
    }
    else
    {
        OCR1B = system_config.min_servo_2 + (system_config.max_servo_2 - system_config.min_servo_2)*100/value;
    }
}

uint8_t is_emergency()
{
    //надо проверить ничего ли мы не профтыкали, если профтыкали - обрубить клапана и начать орать
    return 0;
}

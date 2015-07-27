#include <avr/interrupt.h>
#include <avr/eeprom.h>
#include <util/delay.h>
#include <stdio.h>
#include "init.h"
#include "lcd.h"
#include "adc.h"
#include "uart.h"


// struct SYSTEM_CONFIG nv_system_config EEMEM = {95,95,660,660,5759,0x80,0xFF,45000};
struct SYSTEM_CONFIG nv_system_config EEMEM = {95,95,660,660,5759,0x80,0xA0,45000};
struct SYSTEM_CONFIG system_config;
struct SYSTEM_CONFIG stored_system_config;

struct TARGET_MIX nv_target EEMEM = {32000,0};
struct TARGET_MIX target;
struct TARGET_MIX stored_target;

struct SENSORS_DATA s_data;
struct SENSORS_TARGET_MIX sensors_target;
struct BUTTONS_STATUS buttons;


//it shuld be stored in EEPROM
void load_eeprom_data()
{
    eeprom_read_block((void*)&system_config, (const void*)&nv_system_config, sizeof(system_config));
    eeprom_read_block((void*)&target, (const void*)&nv_target, sizeof(target));
}


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

void save_eeprom_data()
{
    eeprom_read_block((void*)&stored_system_config, (const void*)&nv_system_config, sizeof(stored_system_config));

    if (system_config.min_servo_1 != stored_system_config.min_servo_1 ||
        system_config.min_servo_2 != stored_system_config.min_servo_2 ||
        system_config.max_servo_1 != stored_system_config.max_servo_1 ||
        system_config.max_servo_2 != stored_system_config.max_servo_2 ||
        system_config.servo_timer_period_icr_top != stored_system_config.servo_timer_period_icr_top ||
        system_config.brightness != stored_system_config.brightness ||
        system_config.contrast != stored_system_config.contrast||
        system_config.oxygen_emergency_limit != stored_system_config.oxygen_emergency_limit){
        eeprom_write_block(&system_config, &nv_system_config, sizeof(system_config));
    }
}

void save_target_to_eeprom()
{
    eeprom_read_block((void*)&stored_target, (const void*)&nv_target, sizeof(stored_target));

    if (stored_target.oxygen != target.oxygen ||
        stored_target.helium != target.helium){
        eeprom_write_block(&target, &nv_target, sizeof(target));
    }

}


void init_outputs()
{
	DDRB |= (1 << 3) | (1 << 2); 
	DDRC |= (1 << 0) | (1 << 1) | (1 << 2) | (1 << 3);	
	DDRD |= (1 << 6) | (1 << 7); //Contrast & Brightness
    DDRD &= ~(1 << PD2);
    DDRD &= ~(1 << PD3);
    PORTD |= (1<<PD2);
    PORTD |= (1<<PD3);

    DDRC &= ~(1 << PC4);
    DDRC &= ~(1 << PC5);
    DDRC &= ~(1 << PC6);
    DDRC &= ~(1 << PC7);
    PORTC |= (1<<PC4);
    PORTC |= (1<<PC5);
    PORTC |= (1<<PC6);
    PORTC |= (1<<PC7);


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
    uart0_init(UART_BAUD_SELECT(115200UL, F_CPU));
    if(target.oxygen > 40000){
        target.oxygen = 40000;
    }
    // FILE uart_stream = FDEV_SETUP_STREAM(uart0_putc, uart0_getc, _FDEV_SETUP_RW);
    // stdout = stdin = &uart_stream;
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

uint8_t check_emergency(uint16_t oxygen)
{
    if(!COMPRESSOR_IS_ON || (oxygen > system_config.oxygen_emergency_limit*100) ){
        return 1;
    }    
    return 0;
}

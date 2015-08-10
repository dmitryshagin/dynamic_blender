#include <avr/interrupt.h>
#include <avr/eeprom.h>
#include <util/delay.h>
#include <stdio.h>
#include "init.h"
#include "lcd.h"
#include "adc.h"
#include "uart.h"
#include "menu.h"


// struct SYSTEM_CONFIG nv_system_config EEMEM = {95,95,660,660,5759,0x80,0xFF,45000};
struct SYSTEM_CONFIG nv_system_config EEMEM = {95,95,660,660,5759,0x80,0xA0,45000};
struct SYSTEM_CONFIG system_config;
struct SYSTEM_CONFIG stored_system_config;

struct TARGET_MIX nv_target EEMEM = {32000,0};
struct TARGET_MIX target;
struct TARGET_MIX stored_target;

struct PID_FACTORS nv_pid_factors EEMEM = {12,2,1,12,2,1};
struct PID_FACTORS pid_factors;
struct PID_FACTORS stored_pid_factors;

struct SENSORS_DATA s_data;
struct SENSORS_TARGET_MIX sensors_target;
struct BUTTONS_STATUS buttons;

struct PID_DATA pidData1; 
struct PID_DATA pidData2;

uint32_t log_windows[2][10];
uint8_t  log_position[2];

volatile uint8_t timer0_counter;
volatile uint16_t uptime_counter;
volatile uint16_t uptime_seconds;
volatile uint8_t fl_need_input;
volatile uint8_t fl_blink_5Hz = 0;
volatile uint8_t fl_need_blink = 0;
volatile uint8_t fl_need_buzz = 0;
uint32_t min_s1, min_s2, max_s1, max_s2;


ISR(TIMER0_OVF_vect)
{
    if(timer0_counter>=9){
        uptime_counter++;
        timer0_counter=0;
        check_adc_flags();
        if(uptime_counter%50==0){
            process_buttons();
        }
    }else
    if(timer0_counter==1){
        if(uptime_counter%100==0){
            if(uptime_counter%500==0){
                if(uptime_counter%1000==0){
                    uptime_seconds+=1;
                }
               fl_blink_5Hz = 1-fl_blink_5Hz;
            }
            fl_need_input = 1;     
        } 
    }   
    timer0_counter++;
}


int8_t parseInt(char * input, uint8_t max_count,int *result,int min_allowed,int max_allowed){
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
  if(*result>max_allowed ||*result<min_allowed){
    return -1;
  }
  return 0;  
}

uint8_t blink_5Hz(){
    return fl_blink_5Hz;
}

void process_alert(){
    if(blink_5Hz()){
        if(need_blink()){
            LED_ALERT_ON;
        }
        if(need_buzz()){
            BUZZER_ON;
        }
    }else{
        LED_ALERT_OFF;
        BUZZER_OFF;
    }
}

uint8_t need_blink(){
    return fl_need_blink;
}

uint8_t need_buzz(){
    return fl_need_buzz;
}

void reset_blink(){
    fl_need_blink=0;
}

void reset_buzz(){
    fl_need_buzz=0;
}

void set_alert(uint8_t blink, uint8_t buzz){
    fl_need_blink=blink;
    fl_need_buzz=buzz;
}

uint8_t need_input()
{
    return fl_need_input;
}

void reset_need_input(){
    fl_need_input=0;
}


uint16_t get_uptime_seconds()
{
    return uptime_seconds;
}



//it shuld be stored in EEPROM
void load_eeprom_data()
{
    eeprom_read_block((void*)&system_config, (const void*)&nv_system_config, sizeof(system_config));
    eeprom_read_block((void*)&target, (const void*)&nv_target, sizeof(target));
    eeprom_read_block((void*)&pid_factors, (const void*)&nv_pid_factors, sizeof(pid_factors));
    sensors_target.s1_target = target.oxygen;
    sensors_target.s2_target = target.oxygen;
}



void test_outputs(void)
{
    set_servo(SERVO1,0);
    set_servo(SERVO2,0);
    _delay_ms(500);
    set_servo(SERVO1,0xFF);
    set_servo(SERVO2,0xFF);
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

void save_pid_data_to_eeprom(){
    eeprom_read_block((void*)&stored_pid_factors, (const void*)&nv_pid_factors, sizeof(pid_factors));

    if (stored_pid_factors.s1_p_factor != pid_factors.s1_p_factor ||
        stored_pid_factors.s1_i_factor != pid_factors.s1_i_factor ||
        stored_pid_factors.s1_d_factor != pid_factors.s1_d_factor ||
        stored_pid_factors.s2_p_factor != pid_factors.s2_p_factor ||
        stored_pid_factors.s2_i_factor != pid_factors.s2_i_factor ||
        stored_pid_factors.s2_d_factor != pid_factors.s2_d_factor){
        eeprom_write_block(&pid_factors, &nv_pid_factors, sizeof(pid_factors));
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

static void init_adc()
{
    LCDclr();
    LCDGotoXY(3,0); 
    LCDstring((uint8_t *)"Initing...",10);
    AD7793_Reset();

    if(!AD7793_Init())
    {
        LCDclr();
        LCDGotoXY(2,0);
        LCDstring((uint8_t *)"ADC Error!",10);     
        return;
    }
    
    LCDGotoXY(0,1); 
    LCDstring((uint8_t *)"S1...",5);
    adc_init_channel(AD7793_CH_AIN1P_AIN1M);
    LCDstring((uint8_t *)"OK  S2...",9);
    adc_init_channel(AD7793_CH_AIN2P_AIN2M);    
    LCDstring((uint8_t *)"OK",2);
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
    while(ANY_BUTTON_PRESSED){;}
    _delay_ms(150);
    sei();
    set_current_working_mode(MODE_CALIBRATE);
    set_countdown_timer(15);

}


void set_servo(uint8_t servo, int16_t value)
{
    if(value>0xFF)
    {
        value = 0xFF;
    }
    if(value<0)
    {
        value = 0;
    }
    uint32_t val;
    if(servo==SERVO1)
    {
        sensors_target.valve1_target = value;
        val = system_config.min_servo_1 + (uint32_t)(system_config.max_servo_1 - system_config.min_servo_1)*(uint32_t)value/0xFF;
        OCR1B = val;
    }
    else
    {
        sensors_target.valve2_target = value;
        val = system_config.min_servo_2 + (uint32_t)(system_config.max_servo_2 - system_config.min_servo_2)*(uint32_t)value/0xFF;
        OCR1A = val;
    }
}

void check_alert(){
    uint8_t bl=0;
    uint8_t bz=0;
    if(target.real_oxygen > (target.oxygen+1000)){
        bl=1;
        if(target.real_oxygen > (target.oxygen+2000)){bz=1;} 
    }
    if(target.real_oxygen < (target.oxygen-1000)){
        bl=1;
        if(target.real_oxygen < (target.oxygen-2000)){bz=1;}
    }
    if(sensors_target.s1_target!=sensors_target.s2_target){
        if(target.real_helium > (target.helium+1000)){
            bl=1;
            if(target.real_helium > (target.helium+2000)){bz=1;}
        }
        if(target.real_helium < (target.helium-1000)){
            bl=1;
            if(target.real_helium < (target.helium-2000)){bz=1;}
        }
    }
    set_alert(bl,bz);
}

uint8_t check_emergency()
{
    if(!COMPRESSOR_IS_ON 
            || (s_data.s1_O2 > system_config.oxygen_emergency_limit)
            || (s_data.s2_O2 > system_config.oxygen_emergency_limit) ){
        return 1;
    }    
    return 0;
}

uint8_t is_calibrated_values_ok(){
    if( get_max_deviation() > 500 ){ return 0;}
    if( (min_s1 < 6000) || (min_s2 < 6000) ){ return 0; }
    if( (max_s2 > 29000) || (max_s2 > 29000) ){ return 0; }
    return 1;

}

uint32_t get_max_deviation(){
    uint8_t i;
    min_s1=log_windows[0][0];
    min_s2=log_windows[1][0];
    max_s1=min_s1;
    max_s2=min_s2;
    for (i = 0; i < 10; ++i)
    {
        if( log_windows[0][i] < min_s1 ){ min_s1 = log_windows[0][i]; }
        if( log_windows[0][i] > max_s1 ){ max_s1 = log_windows[0][i]; }
        if( log_windows[1][i] < min_s2 ){ min_s2 = log_windows[1][i]; }
        if( log_windows[1][i] > max_s2 ){ max_s2 = log_windows[1][i]; }
    }
    if( (max_s1-min_s1) >  (max_s2-min_s2) ){
        return (max_s1-min_s1);
    }else{
        return (max_s2-min_s2);
    }

}
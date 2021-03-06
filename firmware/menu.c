#include "menu.h"
#include <stdio.h>
#include <string.h>
#include <avr/io.h>
#include <util/delay.h>
#include "init.h"
#include "lcd.h"


uint8_t mixing_submenu, mode_setup_iteration, valve1_test, valve2_test;
int8_t submenu_position = -1;
int8_t last_submenu_position = -1;

volatile uint16_t current_timer_mark = 0;
volatile uint8_t countdown_timer = 0;


void set_countdown_timer(uint8_t seconds){
    current_timer_mark = get_uptime_seconds()+seconds;
}

uint8_t get_seconds_left(){
    if( current_timer_mark > get_uptime_seconds()){
        return current_timer_mark - get_uptime_seconds();
    }else{
        return 0;
    }
}


void process_buttons()
{
    if(BUTTON_PLUS_PRESSED){
        if(buttons.buttonPlus < 0xFF){
            buttons.buttonPlus+=0xF;
        }
    }else{
        buttons.buttonPlus=0;
    }
    if(BUTTON_MINUS_PRESSED){
        if(buttons.buttonMinus < 0xFF){
            buttons.buttonMinus+=0xF;
        }
    }else{
        buttons.buttonMinus=0;
    }
}

void show_run_test(){
    submenu_position = 5; //to work with UART correctly
    last_submenu_position = 5;
    current_working_mode = MODE_RUN_TEST;
    VALVE1_OFF;
    VALVE2_OFF;
    LED_VAVLE1_OFF;
    LED_VAVLE2_OFF;
    LCDGotoXY(0,0);
    LCDstring((uint8_t *)"    Run Test   >",16);
    LCDGotoXY(0,1);
    LCDstring((uint8_t *)"                ",16);
}

void show_start_calibrate(){
    current_working_mode = MODE_START_CALIBRATE;
    VALVE1_OFF;
    VALVE2_OFF;
    LED_VAVLE1_OFF;
    LED_VAVLE2_OFF;
    LCDGotoXY(0,0);
    LCDstring((uint8_t *)"   Calibrate   >",16);
    LCDGotoXY(0,1);
    LCDstring((uint8_t *)"                ",16);    
}

void show_set_o2(){
    current_working_mode = MODE_SET_O2;
    VALVE1_OFF;
    VALVE2_OFF;
    LED_VAVLE1_OFF;
    LED_VAVLE2_OFF;
    set_alert(0,0);
    set_servo(SERVO1, 0);
    set_servo(SERVO2, 0);
    LCDGotoXY(0,0);
    LCDstring((uint8_t *)"O2<  Oxygen    +",16);
    LCDGotoXY(0,1);
    LCDstring((uint8_t *)"He             -",16);
}

void show_set_he(){
    current_working_mode = MODE_SET_HE;
    VALVE1_OFF;
    VALVE2_OFF;
    LED_VAVLE1_OFF;
    LED_VAVLE2_OFF;
    LCDGotoXY(0,0);
    LCDstring((uint8_t *)"O2   Helium    +",16);
    LCDGotoXY(0,1);
    LCDstring((uint8_t *)"He<            -",16);
}

void show_set_brightness(){
    current_working_mode = MODE_SET_BRIGHTNESS;
    VALVE1_OFF;
    VALVE2_OFF;
    LED_VAVLE1_OFF;
    LED_VAVLE2_OFF; 
    LCDGotoXY(0,0);
    LCDstring((uint8_t *)"  Brightness   +",16);
    LCDGotoXY(0,1);
    LCDstring((uint8_t *)"               -",16);
}

void show_set_contrast(){
    current_working_mode = MODE_SET_CONTRAST;
    VALVE1_OFF;
    VALVE2_OFF;
    LED_VAVLE1_OFF;
    LED_VAVLE2_OFF;
    LCDGotoXY(0,0);
    LCDstring((uint8_t *)"    Contrast   +",16);
    LCDGotoXY(0,1);
    LCDstring((uint8_t *)"               -",16);
}

void show_set_emergency_level(){
    current_working_mode = MODE_SET_EMERGENCY_LEVEL;
    VALVE1_OFF;
    VALVE2_OFF;
    LED_VAVLE1_OFF;
    LED_VAVLE2_OFF;
    LCDGotoXY(0,0);
    LCDstring((uint8_t *)"  Emergency O2 +",16);
    LCDGotoXY(0,1);
    LCDstring((uint8_t *)"               -",16);
}

void show_set_valve1(){
    current_working_mode = MODE_SET_VALVE1;
    VALVE1_ON;
    VALVE2_OFF;
    LED_VAVLE1_ON;
    LED_VAVLE2_OFF;
    LCDGotoXY(0,0);
    LCDstring((uint8_t *)"  Set O2 valve +",16);
    LCDGotoXY(0,1);
    LCDstring((uint8_t *)"               -",16);
}

void show_set_valve2(){
    current_working_mode = MODE_SET_VALVE2;
    VALVE1_OFF;
    VALVE2_ON;
    LED_VAVLE1_OFF;
    LED_VAVLE2_ON;
    LCDGotoXY(0,0);
    LCDstring((uint8_t *)"  Set He valve +",16);
    LCDGotoXY(0,1);
    LCDstring((uint8_t *)"               -",16);
}

void show_set_pid(uint8_t show_max_value){
    current_working_mode = MODE_SET_PID;
    VALVE1_OFF;
    VALVE2_OFF;
    LED_VAVLE1_OFF;
    LED_VAVLE2_OFF; 
    LCDGotoXY(0,0);
    if(show_max_value){
        LCDstring((uint8_t *)"PID setup MAX  +",16);
    }else{
        LCDstring((uint8_t *)"PID setup x128 +",16);    
    }
    LCDGotoXY(0,1);
    LCDstring((uint8_t *)"               -",16);
}

void show_set_servo1_min(){
    current_working_mode = MODE_SET_SERVO1_MIN;
    VALVE1_ON;
    VALVE2_OFF;
    LED_VAVLE1_ON;
    LED_VAVLE2_OFF; 
    LCDGotoXY(0,0);
    LCDstring((uint8_t *)"  Servo1 MIN   +",16);
    LCDGotoXY(0,1);
    LCDstring((uint8_t *)"               -",16);
}

void show_set_servo1_max(){
    current_working_mode = MODE_SET_SERVO1_MAX;
    VALVE1_ON;
    VALVE2_OFF;
    LED_VAVLE1_ON;
    LED_VAVLE2_OFF; 
    LCDGotoXY(0,0);
    LCDstring((uint8_t *)"  Servo1 MAX   +",16);
    LCDGotoXY(0,1);
    LCDstring((uint8_t *)"               -",16);
}

void show_set_servo2_min(){
    current_working_mode = MODE_SET_SERVO2_MIN;
    VALVE1_OFF;
    VALVE2_ON;
    LED_VAVLE1_OFF;
    LED_VAVLE2_ON; 
    LCDGotoXY(0,0);
    LCDstring((uint8_t *)"  Servo2 MIN   +",16);
    LCDGotoXY(0,1);
    LCDstring((uint8_t *)"               -",16);
}

void show_set_servo2_max(){
    current_working_mode = MODE_SET_SERVO2_MAX;
    VALVE1_OFF;
    VALVE2_ON;
    LED_VAVLE1_OFF;
    LED_VAVLE2_ON; 
    LCDGotoXY(0,0);
    LCDstring((uint8_t *)"  Servo2 MAX   +",16);
    LCDGotoXY(0,1);
    LCDstring((uint8_t *)"               -",16);
}

void show_set_timer1(){
    current_working_mode = MODE_SET_TIMER1;
    VALVE1_OFF;
    VALVE2_OFF;
    LED_VAVLE1_OFF;
    LED_VAVLE2_OFF; 
    LCDGotoXY(0,0);
    LCDstring((uint8_t *)"    Timer1     +",16);
    LCDGotoXY(0,1);
    LCDstring((uint8_t *)"               -",16);
}

void show_submenu(){
    switch(submenu_position){
        case 0:
            show_start_calibrate();
            break;
        case 1:
            show_set_emergency_level();
            break;
        case 2:
            show_run_test();
            break;
        case 3:
            show_set_brightness();
            break;
        case 4:
            show_set_contrast();
            break;
        case 5:
            show_set_valve1();
            break;
        case 6:
            show_set_valve2();
            break;
        case 7:
            show_set_pid(0);
            break;
        case 8:
            show_set_pid(0);
            break;        
        case 9:
            show_set_pid(0);
            break;
        case 10:
            show_set_pid(1);
            break;
        case 11:
            show_set_pid(0);
            break;        
        case 12:
            show_set_pid(0);
            break;
        case 13:
            show_set_pid(0);
            break; 
        case 14:
            show_set_pid(1);
            break; 
        case 15:
            show_set_servo1_min();
            break;        
        case 16:
            show_set_servo1_max();
            break;
        case 17:
            show_set_servo2_min();
            break;        
        case 18:
            show_set_servo2_max();
            break;
        case 19:
            show_set_timer1();
            break;           
        default:
            break;    
    }
}


void show_mixing(uint8_t store_to_eeprom){
    current_working_mode = MODE_MIXING;
    LED_VAVLE1_ON;
    VALVE1_ON;

    if(store_to_eeprom){
        save_eeprom_data();
        save_target_to_eeprom();
        save_pid_data_to_eeprom();
        pid_Init(pid_factors.s1_p_factor, pid_factors.s1_i_factor , pid_factors.s1_d_factor , pid_factors.s1_max_output, &pidData1);
        pid_Init(pid_factors.s2_p_factor, pid_factors.s2_i_factor , pid_factors.s2_d_factor , pid_factors.s2_max_output, &pidData2);
    }
    show_mixing_headline();
}

void calclucate_real_gas_values(){
    target.real_oxygen = s_data.s2_O2;
    if(sensors_target.s1_target==sensors_target.s2_target){
        target.real_helium = 0;
    }else{
        uint32_t res = ((((10000UL*(uint32_t)target.real_oxygen))/(s_data.s1_O2)))*10UL;
        if(res > 100000UL){
            target.real_helium = 0;
        }else{
            target.real_helium = 100000UL-res;
        }
    }    
}

void validate_o2_data(){
    if(target.oxygen+5000>system_config.oxygen_emergency_limit){
        target.oxygen=system_config.oxygen_emergency_limit-5000;
    }
    if(target.helium<get_min_helium_limit()){
        target.helium=get_min_helium_limit();
    }
    if(target.helium>get_helium_limit()){
        target.helium=get_helium_limit();
    }
    if(sensors_target.s1_target==sensors_target.s2_target){ //nitrox or trimix?
        sensors_target.s1_target = target.oxygen;
        sensors_target.s2_target = target.oxygen;       
    }else{
        sensors_target.s1_target = ((uint32_t)target.oxygen * 100UL) / (100 - (target.helium/1000UL));
        sensors_target.s2_target = target.oxygen;
    }
}

uint32_t get_helium_limit(){
    uint32_t res = ((((10000UL*(uint32_t)target.oxygen))/(system_config.oxygen_emergency_limit-2000)))*10UL;
    if(res > 100000UL){
        return 0;
    }else{
        return 100000UL-res;
    }
}    

uint32_t get_min_helium_limit(){
    if(target.oxygen<21000){
        uint32_t res = 100000UL*(21000UL-(uint32_t)target.oxygen)/21000UL;
        return ((res/1000UL)+1)*1000UL; //we'll round to next value, a little bit higher
    }else{
        return 0;
    }
}    


void show_mixing_headline(){
    char tmpstr[20];
    // sprintf(tmpstr,"S1:%2li.%01li S2:%2li.%01li  ",  s_data.s1_O2/1000, (s_data.s1_O2%1000)/100, s_data.s2_O2/1000,(s_data.s2_O2%1000)/100);
    calclucate_real_gas_values();
    uint8_t t_o2 = target.oxygen/1000UL;
    uint8_t t_he = target.helium/1000UL;
    uint16_t c_o2 = target.real_oxygen/100UL;
    uint16_t c_he = target.real_helium/100UL;
    if(target.helium > 0 && (sensors_target.s1_target!=sensors_target.s2_target)){
        LED_VAVLE2_ON;
        VALVE2_ON;
        sprintf(tmpstr,"%2u/%2u  %2u.%01u/%2u.%01u",  t_o2, t_he, c_o2/10, (c_o2%10), c_he/10, (c_he%10));
    }else{
        sprintf(tmpstr,"EAN%2u  >EAN%02u.%01u<",  t_o2, c_o2/10, (c_o2%10));
    }
    LCDGotoXY(0,0);
    LCDstring((uint8_t *)tmpstr,16);
}

void show_mixing_submenu(){
    char tmpstr[20];
    uint8_t t_o2 = target.oxygen/1000UL;
    uint8_t curr_o2 = s_data.s1_O2/1000UL;
    uint8_t s_o2_target = sensors_target.s1_target/1000UL;
    uint8_t t_he = target.helium/1000UL;
    uint8_t curr_he = s_data.s2_O2/1000UL;
    uint8_t s_he_target = sensors_target.s2_target/1000UL;

    LCDGotoXY(0,0);
    sprintf(tmpstr,"t%02uc%02u st%02u v%03u",  t_o2, curr_o2, s_o2_target, sensors_target.valve1_target);
    LCDstring((uint8_t *)tmpstr,16);

    LCDGotoXY(0,1);
    sprintf(tmpstr,"t%02uc%02u st%02u v%03u",  t_he, curr_he, s_he_target, sensors_target.valve2_target);
    LCDstring((uint8_t *)tmpstr,16);
}

void show_calibration_error(){
    LED_ALERT_ON;
    LCDGotoXY(0,0);
    LCDstring((uint8_t *)"Can't calibrate!",16);
    LCDGotoXY(0,1);
    LCDstring((uint8_t *)"Sensors failure!",16);
}

void process_menu_selection(){
    char tmpstr[20];
    if((current_working_mode == MODE_SET_O2 || current_working_mode == MODE_SET_HE) && COMPRESSOR_IS_ON && FLOW_IS_ON){
        show_mixing(1);
        set_countdown_timer(30);
        mode_setup_iteration = 1;
    }else
    if(current_working_mode == MODE_MIXING && (!COMPRESSOR_IS_ON || !FLOW_IS_ON) ){
        show_set_o2();
        mode_setup_iteration = 1;
    }

    if(mode_setup_iteration==0){
        if(current_working_mode == MODE_CALIBRATE && COMPRESSOR_IS_ON &&
            (ANY_BUTTON_PRESSED || get_seconds_left()==0) ){
            LCDGotoXY(0,0);
            LCDstring((uint8_t *)" Max deviation: ",16);
            LCDGotoXY(0,1);
            sprintf(tmpstr,"    %5u uV     ",  (uint16_t)get_max_deviation());
            LCDstring((uint8_t *)tmpstr,16);
            _delay_ms(2000);
            if(is_calibrated_values_ok()){
                show_set_o2();
                while(ANY_BUTTON_PRESSED){;}
                _delay_ms(100);
                mode_setup_iteration = 1;
            }else{
                show_calibration_error();
                _delay_ms(2000);
                BUZZER_ON;
                _delay_ms(3000);
                while(ANY_BUTTON_PRESSED){;}
                LED_ALERT_OFF;
                BUZZER_OFF;
                set_countdown_timer(20);
            }
        }
    }

    if(mode_setup_iteration==0 && ANY_BUTTON_PRESSED){
        if(current_working_mode == MODE_SET_O2 && (BUTTON_ENTER_PRESSED)){
            show_set_he();
            mode_setup_iteration = 1;
        }else
        if(current_working_mode == MODE_SET_HE && (BUTTON_EXIT_PRESSED)){
            show_set_o2();
            mode_setup_iteration = 1;
        }else
        if(submenu_position>=0){
            if(BUTTON_ENTER_PRESSED){
                submenu_position+=1;
                if(submenu_position>19){submenu_position=0;}
                mode_setup_iteration = 1;
            }else 
            if(BUTTON_EXIT_PRESSED){
                submenu_position-=1;
                if(submenu_position<0){submenu_position=19;}
                mode_setup_iteration = 1;
            }
        }
        if(last_submenu_position!=submenu_position){
            show_submenu();
            last_submenu_position=submenu_position;
        }
    }   

    if(!ANY_BUTTON_PRESSED){
        mode_setup_iteration = 0;
    }else{
        if(mode_setup_iteration == 1){
            if((current_working_mode >= 50 ) && (BUTTON_EXIT_PRESSED && BUTTON_ENTER_PRESSED)){
                submenu_position=-1;
                last_submenu_position=-1;
                show_set_o2();
                validate_o2_data();
                save_eeprom_data();
                mode_setup_iteration = 2;
            }   
        }   
        if(mode_setup_iteration == 1){
            if((current_working_mode == MODE_SET_HE || current_working_mode == MODE_SET_O2) && 
                (BUTTON_EXIT_PRESSED && BUTTON_ENTER_PRESSED)){
                submenu_position = 0;
                show_submenu();
                mode_setup_iteration = 2;
            }
        }
        if(mode_setup_iteration>0){_delay_ms(50);} //just for some debounce 

    }
}

void run_test()
{
    LCDGotoXY(15,0);
    LCDstring((uint8_t *)" ",1);
    LCDGotoXY(0,1);
    LCDstring((uint8_t *)"   Checking...  ",16);
    test_outputs();
    LCDGotoXY(0,1);
    LCDstring((uint8_t *)"      DONE!     ",16);
    _delay_ms(1000);
    show_run_test();
}

void print_calibration_screen(int32_t oxygen1_uV, int32_t oxygen2_uV, uint8_t time_left)
{
    if(COMPRESSOR_IS_ON){
      char tmpstr[20];
      LCDGotoXY(0,0);
      if(is_calibrated_values_ok()){
        sprintf(tmpstr,"S1: %5liuV    >", oxygen1_uV);
      }else{
        sprintf(tmpstr,"S1: %5liuV     ", oxygen1_uV);
      }
      LCDstring((uint8_t *)tmpstr,16);

      sprintf(tmpstr,"S2: %5liuV  %2ds", oxygen2_uV, time_left);
      LCDGotoXY(0,1);
      LCDstring((uint8_t *)tmpstr,16);
    }else{
      LCDGotoXY(0,0);
      LCDstring((uint8_t *)"Please, turn on ",16);
      LCDGotoXY(0,1);
      LCDstring((uint8_t *)"your compressor!",16);
      set_countdown_timer(20);
    }
}

void screen_set_brightness()
{
    char tmpstr[10];
    uint8_t diff=0;
    if(buttons.buttonMinus>0){
        if(buttons.buttonMinus==0xFF){
            diff = 10;
        }else{
            diff = 1;
        }
        if(system_config.brightness>diff){
            system_config.brightness-=diff;
        }else{
            if(system_config.brightness>0){
                system_config.brightness=0;
            }   
        }
    }


    if(buttons.buttonPlus>0){
        if(buttons.buttonPlus==0xFF){
            diff = 10;
        }else{
            diff = 1;
        }
        if(system_config.brightness<(0xFF-diff)){
            system_config.brightness+=diff;
        }else{
            if(system_config.brightness<0xFF){
                system_config.brightness=0xFF;
            }
        }
    }
    sprintf(tmpstr,"%03u ", system_config.brightness);
    LCDGotoXY(0,1);
    LCDstring((uint8_t *)tmpstr,4);
    LCDGotoXY(4,1);
    LCDprogressBar(system_config.brightness, 255, 10);
    LCDGotoXY(14,1);
    LCDstring((uint8_t *)" -",2);
    set_brightness(system_config.brightness); 
}

void screen_set_contrast()
{
    char tmpstr[10];
    uint8_t diff=0;
    if(buttons.buttonMinus>0){
        if(buttons.buttonMinus==0xFF){
            diff = 10;
        }else{
            diff = 1;
        }
        if(system_config.contrast>diff){
            system_config.contrast-=diff;
        }else{
            if(system_config.contrast>0){
                system_config.contrast=0;
            }   
        }
    }


    if(buttons.buttonPlus>0){
        if(buttons.buttonPlus==0xFF){
            diff = 10;
        }else{
            diff = 1;
        }
        if(system_config.contrast<(0xFF-diff)){
            system_config.contrast+=diff;
        }else{
            if(system_config.contrast<0xFF){
                system_config.contrast=0xFF;
            }
        }
    }
    sprintf(tmpstr,"%03u ", system_config.contrast);
    LCDGotoXY(0,1);
    LCDstring((uint8_t *)tmpstr,4);
    LCDGotoXY(4,1);
    LCDprogressBar(system_config.contrast, 255, 10);
    LCDGotoXY(14,1);
    LCDstring((uint8_t *)" -",2);
    set_contrast(system_config.contrast);
}

void screen_set_emergency_level()
{
    char tmpstr[10];
    uint16_t diff=0;
    if(buttons.buttonMinus>0){
        if(buttons.buttonMinus==0xFF){
            diff = 2000;
        }else{
            diff = 1000;
        }
        if(system_config.oxygen_emergency_limit>(26000+diff)){
            system_config.oxygen_emergency_limit-=diff;
        }else{
            system_config.oxygen_emergency_limit=26000;
        }
    }


    if(buttons.buttonPlus>0){
        if(buttons.buttonPlus==0xFF){
            diff = 2000;
        }else{
            diff = 1000;
        }
        if(system_config.oxygen_emergency_limit<(55000-diff)){
            system_config.oxygen_emergency_limit+=diff;
        }else{
            system_config.oxygen_emergency_limit=55000;
        }
    }
    uint8_t t_print = system_config.oxygen_emergency_limit/1000UL;
    sprintf(tmpstr,"%02u%%", t_print);
    LCDGotoXY(7,1);
    LCDstring((uint8_t *)tmpstr,3);
}

void screen_set_valve1()
{
    char tmpstr[10];
    uint8_t diff=0;
    if(buttons.buttonMinus>0){
        if(buttons.buttonMinus==0xFF){
            diff = 10;
        }else{
            diff = 1;
        }
        if(valve1_test>diff){
            valve1_test-=diff;
        }else{
            valve1_test=0;
        }
    }


    if(buttons.buttonPlus>0){
        if(buttons.buttonPlus==0xFF){
            diff = 10;
        }else{
            diff = 1;
        }
        if(valve1_test<(0xFF-diff)){
            valve1_test+=diff;
        }else{
            valve1_test=0xFF;
        }
    }
    sprintf(tmpstr,"%3u ", valve1_test);
    LCDGotoXY(0,1);
    LCDstring((uint8_t *)tmpstr,4);
    LCDGotoXY(4,1);
    LCDprogressBar(valve1_test, 0xFF, 10);
    LCDGotoXY(14,1);
    LCDstring((uint8_t *)" -",2);
    set_servo(SERVO1, valve1_test);
}

void screen_set_valve2()
{
    char tmpstr[10];
    uint8_t diff=0;
    if(buttons.buttonMinus>0){
        if(buttons.buttonMinus==0xFF){
            diff = 10;
        }else{
            diff = 1;
        }
        if(valve2_test>diff){
            valve2_test-=diff;
        }else{
            valve2_test=0;
        }
    }


    if(buttons.buttonPlus>0){
        if(buttons.buttonPlus==0xFF){
            diff = 10;
        }else{
            diff = 1;
        }
        if(valve2_test<(0xFF-diff)){
            valve2_test+=diff;
        }else{
            valve2_test=0xFF;
        }
    }
    sprintf(tmpstr,"%3u ", valve2_test);
    LCDGotoXY(0,1);
    LCDstring((uint8_t *)tmpstr,4);
    LCDGotoXY(4,1);
    LCDprogressBar(valve2_test, 0xFF, 10);
    LCDGotoXY(14,1);
    LCDstring((uint8_t *)" -",2);
    set_servo(SERVO2, valve2_test);
}

void screen_set_o2()
{
    char tmpstr[10];
    uint16_t diff=0;
    if(buttons.buttonMinus>0){
        if(buttons.buttonMinus==0xFF){
            diff = 2000;
        }else{
            diff = 1000;
        }
        if(target.oxygen>(2000+diff)){
            target.oxygen-=diff;
        }else{
            target.oxygen=2000;
        }
    }


    if(buttons.buttonPlus>0){
        if(buttons.buttonPlus==0xFF){
            diff = 2000;
        }else{
            diff = 1000;
        }
        if(target.oxygen<(system_config.oxygen_emergency_limit-diff-5000)){
            target.oxygen+=diff;
        }else{
            target.oxygen=system_config.oxygen_emergency_limit-5000;
        }
    }
    uint8_t t_print = target.oxygen/1000;
    sprintf(tmpstr,"%02u%%", t_print);
    LCDGotoXY(7,1);
    LCDstring((uint8_t *)tmpstr,3);
    validate_o2_data();
}

void scrren_set_o2_while_mixing()
{
    uint16_t diff=0;
    if(buttons.buttonMinus>0){
        if(buttons.buttonMinus==0xFF){
            diff = 2000;
        }else{
            diff = 1000;
        }
        if(target.oxygen>(21000+diff)){
            target.oxygen-=diff;
        }else{
            target.oxygen=21000;
        }
    }

    if(buttons.buttonPlus>0){
        if(buttons.buttonPlus==0xFF){
            diff = 2000;
        }else{
            diff = 1000;
        }
        if(target.oxygen<(system_config.oxygen_emergency_limit-diff-5000)){
            target.oxygen+=diff;
        }else{
            target.oxygen=system_config.oxygen_emergency_limit-5000;
        }
    }
    sensors_target.s1_target = target.oxygen;
    sensors_target.s2_target = target.oxygen;
    if(diff>0){
        show_mixing(0);
        set_alert(0,0);
        set_countdown_timer(10);
    }
}

void screen_set_helium()
{
    char tmpstr[10];
    uint16_t diff=0;

    if(buttons.buttonMinus>0){
        if(buttons.buttonMinus==0xFF){
            diff = 2000;
        }else{
            diff = 1000;
        }
        if(target.helium>(get_min_helium_limit()+diff)){
            target.helium-=diff;
        }else{
            target.helium=get_min_helium_limit();
        }
    }


    if(buttons.buttonPlus>0){
        if(buttons.buttonPlus==0xFF){
            diff = 2000;
        }else{
            diff = 1000;
        }
        
        if(target.helium<(get_helium_limit()-diff)){
            target.helium+=diff;
        }else{
            target.helium=get_helium_limit();
        }
    }


    uint8_t t_print = target.helium/1000;
    sprintf(tmpstr,"%02u%%", t_print);
    LCDGotoXY(7,1);
    LCDstring((uint8_t *)tmpstr,3);

    sensors_target.s1_target = ((uint32_t)target.oxygen * 100UL) / (100 - (target.helium/1000UL));
    sensors_target.s2_target = target.oxygen;

}

void screen_main_mixing()
{
    char tmpstr[20];
    if(check_emergency()){
        current_working_mode = MODE_EMERGENCY;
        LED_ALERT_ON;
        BUZZER_ON;
        VALVE1_OFF;
        VALVE2_OFF;
        LED_VAVLE1_OFF;
        LED_VAVLE2_OFF;
        LCDGotoXY(0,0);
        LCDstring((uint8_t *)"   Emergency!   ",16);
        LCDGotoXY(0,1);
        LCDstring((uint8_t *)"Reboot required!",16);
    }else{
        if(mixing_submenu==0){
            show_mixing_headline();
            sprintf(tmpstr,"S1:%2li.%01li S2:%2li.%01li  ",  s_data.s1_O2/1000, (s_data.s1_O2%1000)/100, s_data.s2_O2/1000,(s_data.s2_O2%1000)/100);
            LCDGotoXY(0,1);
            LCDstring((uint8_t *)tmpstr,16);
        }else{
            show_mixing_submenu();
        }   
        if(get_seconds_left()==0){
            check_alert();
        }
    }
}

static void screen_set_pid(uint8_t pid_index)
{
    char tmpstr[20];
    int16_t diff=0;
    int16_t target_value=0;
    switch(pid_index){
        case 0:
            target_value = pid_factors.s1_p_factor;
            break;
        case 1:
            target_value = pid_factors.s1_i_factor;
            break;
        case 2:
            target_value = pid_factors.s1_d_factor;
            break;
        case 3:
            target_value = pid_factors.s1_max_output;
            break;    
        case 4:
            target_value = pid_factors.s2_p_factor;
            break;
        case 5:
            target_value = pid_factors.s2_i_factor;
            break;
        case 6:
            target_value = pid_factors.s2_d_factor;
            break;
        case 7:
            target_value = pid_factors.s2_max_output;
            break;    
    }

    if(buttons.buttonMinus>0){
        if(buttons.buttonMinus==0xFF){
            diff = 10;
        }else{
            diff = 1;
        }
        if(target_value>(diff-MAX_INT)){
            target_value-=diff;
        }else{
            target_value=-MAX_INT;
        }
    }

    if(buttons.buttonPlus>0){
        if(buttons.buttonPlus==0xFF){
            diff = 10;
        }else{
            diff = 1;
        }
        if(target_value<(MAX_INT-diff)){
            target_value+=diff;
        }else{
            target_value=MAX_INT;
        }
    }
    char target_name[7];

    switch(pid_index){
        case 0:
            pid_factors.s1_p_factor = target_value;
            strncpy(target_name, "S1_P", sizeof("S1_P"));
            break;
        case 1:
            pid_factors.s1_i_factor = target_value;
            strncpy(target_name, "S1_I", sizeof("S1_I"));
            break;
        case 2:
            pid_factors.s1_d_factor = target_value;
            strncpy(target_name, "S1_D", sizeof("S1_D"));
            break;
        case 3:
            pid_factors.s1_max_output = target_value;
            strncpy(target_name, "S1_MAX", sizeof("S1_MAX"));
            break;    
        case 4:
            pid_factors.s2_p_factor = target_value;
            strncpy(target_name, "S2_P", sizeof("S2_P"));
            break;
        case 5:
            pid_factors.s2_i_factor = target_value;
            strncpy(target_name, "S2_I", sizeof("S2_I"));
            break;
        case 6:
            pid_factors.s2_d_factor = target_value;
            strncpy(target_name, "S2_D", sizeof("S2_D"));
            break;
        case 7:
            pid_factors.s2_max_output = target_value;
            strncpy(target_name, "S2_MAX", sizeof("S2_MAX"));
            break;    
    }


    sprintf(tmpstr,"%s   %05d   -",target_name, target_value);

    LCDGotoXY(0,1);
    LCDstring((uint8_t *)tmpstr,16);
}

void screen_set_servo1_min(){
    char tmpstr[10];
    uint16_t diff=0;
    if(buttons.buttonMinus>0){
        if(buttons.buttonMinus==0xFF){
            diff = 10;
        }else{
            diff = 1;
        }
        if(system_config.min_servo_1>diff){
            system_config.min_servo_1-=diff;
        }else{
            system_config.min_servo_1=0;
        }
    }


    if(buttons.buttonPlus>0){
        if(buttons.buttonPlus==0xFF){
            diff = 10;
        }else{
            diff = 1;
        }
        if(system_config.min_servo_1<(system_config.max_servo_1-diff)){
            system_config.min_servo_1+=diff;
        }else{
            system_config.min_servo_1=system_config.max_servo_1;
        }
    }
    sprintf(tmpstr,"%04u", system_config.min_servo_1);
    LCDGotoXY(6,1);
    LCDstring((uint8_t *)tmpstr,4);
    init_outputs();
    set_servo(SERVO1,0);
    set_servo(SERVO2,0);
}

void screen_set_servo1_max(){
    char tmpstr[10];
    uint16_t diff=0;
    if(buttons.buttonMinus>0){
        if(buttons.buttonMinus==0xFF){
            diff = 10;
        }else{
            diff = 1;
        }
        if(system_config.max_servo_1>(system_config.min_servo_1-diff)){
            system_config.max_servo_1-=diff;
        }else{
            system_config.max_servo_1=system_config.min_servo_1;
        }
    }


    if(buttons.buttonPlus>0){
        if(buttons.buttonPlus==0xFF){
            diff = 10;
        }else{
            diff = 1;
        }
        if(system_config.max_servo_1<(10000-diff)){
            system_config.max_servo_1+=diff;
        }else{
            system_config.max_servo_1=10000;
        }
    }
    sprintf(tmpstr,"%04u", system_config.max_servo_1);
    LCDGotoXY(6,1);
    LCDstring((uint8_t *)tmpstr,4);
    init_outputs();
    set_servo(SERVO1,0xFF);
    set_servo(SERVO2,0);
}

void screen_set_servo2_min(){
    char tmpstr[10];
    uint16_t diff=0;
    if(buttons.buttonMinus>0){
        if(buttons.buttonMinus==0xFF){
            diff = 10;
        }else{
            diff = 1;
        }
        if(system_config.min_servo_2>diff){
            system_config.min_servo_2-=diff;
        }else{
            system_config.min_servo_2=0;
        }
    }


    if(buttons.buttonPlus>0){
        if(buttons.buttonPlus==0xFF){
            diff = 10;
        }else{
            diff = 1;
        }
        if(system_config.min_servo_2<(system_config.max_servo_2-diff)){
            system_config.min_servo_2+=diff;
        }else{
            system_config.min_servo_2=system_config.max_servo_2;
        }
    }
    sprintf(tmpstr,"%04u", system_config.min_servo_2);
    LCDGotoXY(6,1);
    LCDstring((uint8_t *)tmpstr,4);
    init_outputs();
    set_servo(SERVO1,0);
    set_servo(SERVO2,0);
}

void screen_set_servo2_max(){
    char tmpstr[10];
    uint16_t diff=0;
    if(buttons.buttonMinus>0){
        if(buttons.buttonMinus==0xFF){
            diff = 10;
        }else{
            diff = 1;
        }
        if(system_config.max_servo_2>(system_config.min_servo_2-diff)){
            system_config.max_servo_2-=diff;
        }else{
            system_config.max_servo_2=system_config.min_servo_2;
        }
    }


    if(buttons.buttonPlus>0){
        if(buttons.buttonPlus==0xFF){
            diff = 10;
        }else{
            diff = 1;
        }
        if(system_config.max_servo_2<(10000-diff)){
            system_config.max_servo_2+=diff;
        }else{
            system_config.max_servo_2=10000;
        }
    }
    sprintf(tmpstr,"%04u", system_config.max_servo_2);
    LCDGotoXY(6,1);
    LCDstring((uint8_t *)tmpstr,4);
    init_outputs();
    set_servo(SERVO1,0);
    set_servo(SERVO2,0xFF);
}

void screen_set_timer1(){
    char tmpstr[10];
    uint16_t diff=0;
    if(buttons.buttonMinus>0){
        if(buttons.buttonMinus==0xFF){
            diff = 10;
        }else{
            diff = 1;
        }
        if(system_config.servo_timer_period_icr_top>diff){
            system_config.servo_timer_period_icr_top-=diff;
        }else{
            system_config.servo_timer_period_icr_top=0;
        }
    }


    if(buttons.buttonPlus>0){
        if(buttons.buttonPlus==0xFF){
            diff = 10;
        }else{
            diff = 1;
        }
        if(system_config.servo_timer_period_icr_top<(16000-diff)){
            system_config.servo_timer_period_icr_top+=diff;
        }else{
            system_config.servo_timer_period_icr_top=16000;
        }
    }
    sprintf(tmpstr,"%04u", system_config.servo_timer_period_icr_top);
    LCDGotoXY(6,1);
    LCDstring((uint8_t *)tmpstr,4);
    init_outputs();
    set_servo(SERVO1,0);
    set_servo(SERVO2,0);
}


void process_menu_internal(){
    if(current_working_mode==MODE_CALIBRATE){
        s_data.s1_coeff = O2_COEFF/s_data.s1_uV;
        s_data.s2_coeff = O2_COEFF/s_data.s2_uV;
        print_calibration_screen(s_data.s1_uV, s_data.s2_uV, get_seconds_left());
    }else
    if(current_working_mode==MODE_RUN_TEST){
        if(BUTTON_PLUS_PRESSED){
            run_test();
        }
    }else
    if(current_working_mode==MODE_START_CALIBRATE){
        if(BUTTON_PLUS_PRESSED){
            current_working_mode=MODE_CALIBRATE;
            while(ANY_BUTTON_PRESSED){;}
            set_countdown_timer(30);
        }
    }else
    if(current_working_mode==MODE_SET_BRIGHTNESS){
        screen_set_brightness();              
    }else
    if(current_working_mode==MODE_SET_CONTRAST){
        screen_set_contrast();
    }else
    if(current_working_mode==MODE_SET_EMERGENCY_LEVEL){
        screen_set_emergency_level();
    }else
    if(current_working_mode==MODE_SET_VALVE1){
        screen_set_valve1();
    }else
    if(current_working_mode==MODE_SET_VALVE2){
        screen_set_valve2();
    }else
    if(current_working_mode==MODE_SET_O2){
        screen_set_o2(); 
    }else
    if(current_working_mode==MODE_MIXING && 
        (BUTTON_PLUS_PRESSED || BUTTON_MINUS_PRESSED) &&
        (sensors_target.s1_target == sensors_target.s2_target)){
        scrren_set_o2_while_mixing();
    }else
    if(current_working_mode==MODE_MIXING && 
        BUTTON_ENTER_PRESSED && BUTTON_EXIT_PRESSED ){
        mixing_submenu = 1;
    }else
    if(current_working_mode==MODE_MIXING && 
        BUTTON_EXIT_PRESSED && !BUTTON_ENTER_PRESSED){
        mixing_submenu = 0;
        show_mixing(0);
    }else
    if(current_working_mode==MODE_SET_HE){
        screen_set_helium();
    }else
    if(current_working_mode==MODE_START_CALIBRATE){
        show_start_calibrate();
    }else
    if(current_working_mode==MODE_SET_PID){
        screen_set_pid(submenu_position-7);
    }else
    if(current_working_mode==MODE_SET_SERVO1_MIN){
        screen_set_servo1_min();
    }else
    if(current_working_mode==MODE_SET_SERVO1_MAX){
        screen_set_servo1_max();
    }else
    if(current_working_mode==MODE_SET_SERVO2_MIN){
        screen_set_servo2_min();
    }else
    if(current_working_mode==MODE_SET_SERVO2_MAX){
        screen_set_servo2_max();
    }else
    if(current_working_mode==MODE_SET_TIMER1){
        screen_set_timer1();
    }

    if(current_working_mode==MODE_MIXING){
        screen_main_mixing();
    }
}

uint8_t get_current_working_mode(){
    return current_working_mode;
}

void set_current_working_mode(uint8_t mode){
    current_working_mode = mode;
}


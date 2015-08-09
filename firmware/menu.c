#include "menu.h"
#include <stdio.h>
#include <avr/io.h>
#include <util/delay.h>
#include "init.h"
#include "lcd.h"


uint8_t mixing_submenu = 0;
uint8_t mode_setup_iteration = 0;
uint8_t valve1_test = 0;
uint8_t valve2_test = 0;
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

void show_submenu(){
    switch(submenu_position){
        case 0:
            show_set_brightness();
            break;
        case 1:
            show_set_contrast();
            break;
        case 2:
            show_set_emergency_level();
            break;
        case 3:
            show_set_valve1();
            break;
        case 4:
            show_set_valve2();
            break;
        case 5:
            show_run_test();
            break;
        case 6:
            show_start_calibrate();
            break;
    }
}


void show_mixing(uint8_t store_to_eeprom){
    current_working_mode = MODE_MIXING;
    LED_VAVLE1_ON;
    VALVE1_ON;

    //TODO - remove IRL
    // LED_VAVLE2_ON;
    // VALVE2_ON;
    if(store_to_eeprom){
        save_eeprom_data();
        save_target_to_eeprom();
        save_pid_data_to_eeprom();
        pid_Init(pid_factors.s1_p_factor, pid_factors.s1_i_factor , pid_factors.s1_d_factor , &pidData1);
        pid_Init(pid_factors.s2_p_factor, pid_factors.s2_i_factor , pid_factors.s2_d_factor , &pidData2);
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
    if(target.helium>get_helium_limit()){
        target.helium=get_helium_limit();
    }
    if(sensors_target.s1_target==sensors_target.s2_target){ //nitrox or trimix?
        sensors_target.s1_target = target.oxygen;
        sensors_target.s2_target = target.oxygen;       
    }else{
        sensors_target.s1_target = ((uint32_t)target.oxygen * 100UL) / (100 - (target.helium/1000UL));
        sensors_target.s2_target = (uint16_t)target.oxygen;
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

void show_mixing_headline(){
    char tmpstr[20];
    sprintf(tmpstr,"S1:%2li.%01li S2:%2li.%01li  ",  s_data.s1_O2/1000, (s_data.s1_O2%1000)/100, s_data.s2_O2/1000,(s_data.s2_O2%1000)/100);
    calclucate_real_gas_values();
    uint8_t t_o2 = target.oxygen/1000UL;
    uint8_t t_he = target.helium/1000UL;
    uint8_t c_o2 = target.real_oxygen/1000UL;
    uint8_t c_he = target.real_helium/1000UL;
    if(target.helium > 0 && (sensors_target.s1_target!=sensors_target.s2_target)){
        LED_VAVLE2_ON;
        VALVE2_ON;
        sprintf(tmpstr,"TmX %2u/%2u  %2u/%2u",  t_o2, t_he, c_o2, c_he);
    }else{
        sprintf(tmpstr,"EANx%2u  Real:%2u ",  t_o2, c_o2);
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
    // if(sensors_target.s1_target!=sensors_target.s2_target){
        sprintf(tmpstr,"t%02uc%02u st%02u v%03u",  t_he, curr_he, s_he_target, sensors_target.valve2_target);
        LCDstring((uint8_t *)tmpstr,16);
    // }else{
        // LCDstring((uint8_t *)"                ",16);
    // }   
}

void show_calibration_error(){
    LED_ALERT_ON;
    LCDGotoXY(0,0);
    LCDstring((uint8_t *)"Can't calibrate!",16);
    LCDGotoXY(0,1);
    LCDstring((uint8_t *)"Sensors failure!",16);
}

void process_menu_selection(){
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
                set_countdown_timer(15);
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
                if(submenu_position>6){submenu_position=0;}
                mode_setup_iteration = 1;
            }else 
            if(BUTTON_EXIT_PRESSED){
                submenu_position-=1;
                if(submenu_position<0){submenu_position=6;}
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

void screen_set_brightness()
{
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
    uint16_t diff=0;
    if(buttons.buttonMinus>0){
        if(buttons.buttonMinus==0xFF){
            diff = 2000;
        }else{
            diff = 1000;
        }
        if(system_config.oxygen_emergency_limit>(21000+diff)){
            system_config.oxygen_emergency_limit-=diff;
        }else{
            system_config.oxygen_emergency_limit=21000;
        }
    }


    if(buttons.buttonPlus>0){
        if(buttons.buttonPlus==0xFF){
            diff = 2000;
        }else{
            diff = 1000;
        }
        if(system_config.oxygen_emergency_limit<(65000-diff)){
            system_config.oxygen_emergency_limit+=diff;
        }else{
            system_config.oxygen_emergency_limit=65000;
        }
    }
    uint8_t t_print = system_config.oxygen_emergency_limit/1000UL;
    sprintf(tmpstr,"%02u%%", t_print);
    LCDGotoXY(7,1);
    LCDstring((uint8_t *)tmpstr,3);
}

void screen_set_valve1()
{
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
    if(target.helium>get_helium_limit()){
        target.helium=get_helium_limit();
    }
    if(sensors_target.s1_target==sensors_target.s2_target){ //nitrox or trimix?
        sensors_target.s1_target = target.oxygen;
        sensors_target.s2_target = target.oxygen;       
    }else{
        sensors_target.s1_target = ((uint32_t)target.oxygen * 100UL) / (100 - (target.helium/1000UL));
        sensors_target.s2_target = (uint16_t)target.oxygen;
    }  
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
    uint16_t diff=0;

    if(buttons.buttonMinus>0){
        if(buttons.buttonMinus==0xFF){
            diff = 2000;
        }else{
            diff = 1000;
        }
        if(target.helium>diff){
            target.helium-=diff;
        }else{
            target.helium=0;
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
    sensors_target.s2_target = (uint16_t)target.oxygen;

}

void screen_main_mixing()
{
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
        if(COMPRESSOR_IS_ON){
            LCDstring((uint8_t *)"  O2 too high!  ",16);
        }else{
            LCDstring((uint8_t *)"Compressor:  OFF",16);
        }
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
            set_countdown_timer(15);
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


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


void process_buttons()
{
    if(BUTTON_PLUS_PRESSED){
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

void show_start_calibrate(){
    current_working_mode = MODE_START_CALIBRATE;
    VALVE1_OFF;
    VALVE2_OFF;
    LED_VAVLE1_OFF;
    LED_VAVLE2_OFF;
    LCDGotoXY(0,0);
    LCDstring("   Calibrate   >",16);
    LCDGotoXY(0,1);
    LCDstring("               >",16);    
}

void show_set_o2(){
    current_working_mode = MODE_SET_O2;
    VALVE1_OFF;
    VALVE2_OFF;
    LED_VAVLE1_OFF;
    LED_VAVLE2_OFF;
    set_servo(SERVO1, 0);
    set_servo(SERVO2, 0);
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
    current_working_mode = MODE_MIXING;
    LED_VAVLE1_ON;
    VALVE1_ON;

    char tmpstr[20];
    uint8_t t_o2 = target.oxygen/1000UL;
    uint8_t t_he = target.helium/1000UL;
    if(target.helium > 0 && (sensors_target.s1_target!=sensors_target.s2_target)){
        LED_VAVLE2_ON;
        VALVE2_ON;
        sprintf(tmpstr,"Mixing TmX %2u/%2u  ",  t_o2, t_he);
    }else{
        sprintf(tmpstr,"Mixing EANx%2u   ",  t_o2);
    }
    save_eeprom_data();
    save_target_to_eeprom();
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
    sprintf(tmpstr,"t%02uc%02u st%02u %03u%%",  t_o2, curr_o2, s_o2_target, sensors_target.valve1_target);
    LCDstring((uint8_t *)tmpstr,16);

    LCDGotoXY(0,1);
    if(sensors_target.s1_target!=sensors_target.s2_target){
        sprintf(tmpstr,"t%02uc%02u st%02u %03u%%",  t_he, curr_he, s_he_target, sensors_target.valve2_target);
        LCDstring((uint8_t *)tmpstr,16);
    }else{
        LCDstring("                ",16);
    }   
}

void process_menu_selection(){
    if((current_working_mode == MODE_SET_O2 || current_working_mode == MODE_SET_HE) && COMPRESSOR_IS_ON && FLOW_IS_ON){
        show_mixing();
        mode_setup_iteration = 1;
    }else
    if(current_working_mode == MODE_MIXING && (!COMPRESSOR_IS_ON || !FLOW_IS_ON) ){
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
                show_start_calibrate();
                mode_setup_iteration = 1;
            }
        }else
        if(current_working_mode == MODE_SET_CONTRAST){
            if(BUTTON_ENTER_PRESSED){
                show_set_emergency_level();
                mode_setup_iteration = 1;
            }else 
            if(BUTTON_EXIT_PRESSED){
                show_set_brightness();
                mode_setup_iteration = 1;
            }
        }else
        if(current_working_mode == MODE_SET_EMERGENCY_LEVEL){
            if(BUTTON_ENTER_PRESSED){
                show_set_valve1();
                mode_setup_iteration = 1;
            }else 
            if(BUTTON_EXIT_PRESSED){
                show_set_contrast();
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
                show_start_calibrate();
                mode_setup_iteration = 1;
            }else 
            if(BUTTON_EXIT_PRESSED){
                show_set_valve2();
                mode_setup_iteration = 1;
            }
        }else
        if(current_working_mode == MODE_START_CALIBRATE){
            if(BUTTON_ENTER_PRESSED){
                show_set_brightness();
                mode_setup_iteration = 1;
            }else 
            if(BUTTON_EXIT_PRESSED){
                show_run_test();
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
        if(mode_setup_iteration>0){_delay_ms(50);} //just for some debounce 

    }
}

void run_test()
{
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
    sprintf(tmpstr,"%03u     ", system_config.brightness);
    LCDGotoXY(0,1);
    LCDstring((uint8_t *)tmpstr,4);
    LCDGotoXY(4,1);
    LCDprogressBar(system_config.brightness, 255, 10);
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
    sprintf(tmpstr,"%03u     ", system_config.contrast);
    LCDGotoXY(0,1);
    LCDstring((uint8_t *)tmpstr,4);
    LCDGotoXY(4,1);
    LCDprogressBar(system_config.contrast, 255, 10);
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
        if(valve1_test<(100-diff)){
            valve1_test+=diff;
        }else{
            valve1_test=100;
        }
    }
    sprintf(tmpstr,"%3u%%", valve1_test);
    LCDGotoXY(0,1);
    LCDstring((uint8_t *)tmpstr,4);
    LCDGotoXY(4,1);
    LCDprogressBar(valve1_test, 100, 10);
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
        if(valve2_test<(100-diff)){
            valve2_test+=diff;
        }else{
            valve2_test=100;
        }
    }
    sprintf(tmpstr,"%3u%%", valve2_test);
    LCDGotoXY(0,1);
    LCDstring((uint8_t *)tmpstr,4);
    LCDGotoXY(4,1);
    LCDprogressBar(valve2_test, 100, 10);
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
    if((target.oxygen+target.helium)>100000){
        target.helium=100000-target.oxygen;
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
        show_mixing();
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
        if(target.helium<(100000UL-target.oxygen-diff)){
            target.helium+=diff;
        }else{
            target.helium=100000UL-target.oxygen;
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
    if(check_emergency((uint16_t)s_data.s2_O2)){
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
        if(mixing_submenu==0){
            sprintf(tmpstr,"S1:%2li.%01li S2:%2li.%01li  ",  s_data.s1_O2/1000, (s_data.s1_O2%1000)/100, s_data.s2_O2/1000,(s_data.s2_O2%1000)/100);
            LCDGotoXY(0,1);
            LCDstring((uint8_t *)tmpstr,16);
        }else{
            show_mixing_submenu();
        }   
    }
}

void process_menu_internal(){
    if(current_working_mode==MODE_CALIBRATE){
        s_data.s1_coeff = O2_COEFF/s_data.s1_uV;
        s_data.s2_coeff = O2_COEFF/s_data.s2_uV;
        print_calibration_screen(s_data.s1_uV, s_data.s2_uV);
    }else
    if(current_working_mode==MODE_RUN_TEST){
        if(BUTTON_MINUS_PRESSED||BUTTON_PLUS_PRESSED){
            run_test();
        }
    }else
    if(current_working_mode==MODE_START_CALIBRATE){
        if(BUTTON_MINUS_PRESSED||BUTTON_PLUS_PRESSED){
            current_working_mode=MODE_CALIBRATE;
            while(ANY_BUTTON_PRESSED){;}
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
        show_mixing();
    }else
    if(current_working_mode==MODE_SET_HE){
        screen_set_helium();
    }else
    if(current_working_mode==MODE_START_CALIBRATE){
        screen_start_calibrate();
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


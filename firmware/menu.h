#ifndef __MENU_H__
#define __MENU_H__

#include <stdio.h>
#include <avr/io.h>


uint8_t current_working_mode;


void show_set_o2();

void show_set_brightness();

void show_mixing(uint8_t store_to_eeprom);
void show_mixing_headline();

uint32_t get_helium_limit();
uint32_t get_min_helium_limit();

void show_mixing_submenu();

void process_menu_selection();

void process_menu_internal();

void process_buttons();

uint8_t get_current_working_mode();

void set_current_working_mode(uint8_t mode);

void set_countdown_timer(uint8_t seconds);
void print_calibration_screen(int32_t oxygen1_uV, int32_t oxygen2_uV,uint8_t time_left);

void show_run_test();
void run_test();
void validate_o2_data();
void show_start_calibrate();

#endif	// _MENU_H_

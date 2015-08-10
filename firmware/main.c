#include "adc.h"
#include "init.h"
#include "menu.h"
#include "uart_proto.h"

int main(void){
	init();	
    
	for(;;){
		process_uart();
        process_adc_data(); 
	    process_menu_selection();

		if(need_input()){
			process_menu_internal();
			reset_need_input();
		}

		if( current_working_mode != MODE_EMERGENCY ){
        	process_alert();
        }
    }
    return 0;
}
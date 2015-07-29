#include <stdlib.h>
#include <stdio.h>
#include "pid.h"


//COPYPASTED PID
// should store in EEPROM
#define K_P     1.00
#define K_I     0.00
#define K_D     0.00
struct PID_DATA pidData1; 
struct PID_DATA pidData2;


int main(void)
{
	

	pid_Init(K_P * SCALING_FACTOR, K_I * SCALING_FACTOR , K_D * SCALING_FACTOR , &pidData1);
	pid_Init(K_P * SCALING_FACTOR, K_I * SCALING_FACTOR , K_D * SCALING_FACTOR , &pidData2);


	int16_t inputValue = pid_Controller((uint16_t)400, (uint16_t)210, &pidData1);
		    // sprintf(tmpstr,"%8li",  inputValue);
	printf("VAL: %i   \n", inputValue);
	// for(;;){
		//inputValue = pid_Controller((uint16_t)oxygen1_target, (uint16_t)oxygen1_value, &pidData1);
		    // sprintf(tmpstr,"%8li",  inputValue);
			
    // }
    return 0;
}
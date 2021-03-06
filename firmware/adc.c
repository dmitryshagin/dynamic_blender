/***************************************************************************//**
 *   @file   AD7793.c
 *   @brief  Implementation of AD7793 Driver.
 *   @author Bancisor MIhai
********************************************************************************
 * Copyright 2012(c) Analog Devices, Inc.
 *
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *  - Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 *  - Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in
 *    the documentation and/or other materials provided with the
 *    distribution.
 *  - Neither the name of Analog Devices, Inc. nor the names of its
 *    contributors may be used to endorse or promote products derived
 *    from this software without specific prior written permission.
 *  - The use of this software may or may not infringe the patent rights
 *    of one or more patent holders.  This license does not release you
 *    from the requirement that you obtain separate licenses from these
 *    patent holders to use this software.
 *  - Use of the software either in source or binary form, must be run
 *    on or directly connected to an Analog Devices Inc. component.
 *
 * THIS SOFTWARE IS PROVIDED BY ANALOG DEVICES "AS IS" AND ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, NON-INFRINGEMENT,
 * MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.
 * IN NO EVENT SHALL ANALOG DEVICES BE LIABLE FOR ANY DIRECT, INDIRECT,
 * INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
 * LIMITED TO, INTELLECTUAL PROPERTY RIGHTS, PROCUREMENT OF SUBSTITUTE GOODS OR
 * SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
 * CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
 * OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *
********************************************************************************
 *   SVN Revision: 500
*******************************************************************************/

/******************************************************************************/
/* Include Files                                                              */
/******************************************************************************/
#include "adc.h"	   // AD7793 definitions.
#include "spi.h"	   // Communication definitions.
#include "init.h"
#include "menu.h"
#include <util/delay.h>


volatile uint8_t adc_current_channel = AD7793_CH_AIN1P_AIN1M;
volatile uint8_t adc_ready = 0, adc_prepare = 0;
int16_t prev_input_s1, prev_input_s2;

/***************************************************************************//**
 * @brief Initializes the AD7793 and checks if the device is present.
 *
 * @return status - Result of the initialization procedure.
 *                  Example: 1 - if initialization was successful (ID is 0x0B).
 *                           0 - if initialization was unsuccessful.
*******************************************************************************/
void check_adc_flags()
{
    if(adc_prepare>0){
        adc_prepare++;
        if(adc_prepare>=50){
            adc_ready = 1;
            adc_prepare = 0;
        }
    }
}


void process_adc_data()
{
    int64_t uV;
    int16_t inputValue;
    uint16_t corrected_target, corrected_current;
    if(adc_ready>0){ 
        uV = (((int64_t)AD7793_ContinuousReadAvg(1) - 0x800000)*73125) / 0x800000;
        if(uV < 0){ uV = -uV; }
        
        if(adc_current_channel == AD7793_CH_AIN1P_AIN1M){
            s_data.s1_uV = uV;
            s_data.s1_O2 = s_data.s1_uV * s_data.s1_coeff / 10000;
            log_windows[0][log_position[0]] = s_data.s1_uV;
            adc_current_channel = AD7793_CH_AIN2P_AIN2M;
            if(get_current_working_mode() == MODE_MIXING){
                corrected_target = (uint16_t)((sensors_target.s1_target+50)/100);
                corrected_current = (uint16_t)((s_data.s1_O2+50)/100);
                inputValue = pid_Controller(corrected_target, corrected_current, &pidData1);
                if((prev_input_s1<inputValue) && (corrected_current >= corrected_target) ){
                    inputValue = prev_input_s1;
                }
                prev_input_s1 = inputValue;
                set_servo(SERVO1, inputValue);
            }
            if(++log_position[0]>9){log_position[0]=0;}
        }else{
            s_data.s2_uV = uV;
            s_data.s2_O2 = s_data.s2_uV * s_data.s2_coeff / 10000;
            log_windows[1][log_position[1]] = s_data.s2_uV;
            adc_current_channel = AD7793_CH_AIN1P_AIN1M;
            if(get_current_working_mode() == MODE_MIXING && (sensors_target.s1_target!=sensors_target.s2_target)){
                corrected_target = (uint16_t)((sensors_target.s2_target+50)/100);
                corrected_current = (uint16_t)((s_data.s2_O2+50)/100);
                inputValue = pid_Controller(corrected_target, corrected_current, &pidData2);
                inputValue = -inputValue;
                if((prev_input_s2<inputValue) && (corrected_current <= corrected_target) ){
                   inputValue = prev_input_s2;
                }
                prev_input_s2 = inputValue;
                //TODO - do not open second valve if first one is far away from target? Just an idea
                set_servo(SERVO2, inputValue);
            }
            if(++log_position[1]>9){log_position[1]=0;}
        }               
        adc_change_channel_and_trigger_delay(adc_current_channel);
    }    
}

unsigned char AD7793_Init(void)
{ 
	unsigned char status = 0x1;
    unsigned char ad_id = AD7793_GetRegisterValue(AD7793_REG_ID, 1, 1);
    
    adc_prepare = 0;
    adc_ready = 0;
    
    if((ad_id & 0x0F) != AD7793_ID)
	{
	   status = 0x0;
    }
    return(status);
}

/***************************************************************************//**
 * @brief Sends 32 consecutive 1's on SPI in order to reset the part.
 *
 * @return  None.    
*******************************************************************************/
void AD7793_Reset(void)
{
	unsigned char dataToSend[5] = {0x01, 0xff, 0xff, 0xff, 0xff};
    SPI_Init(0, 1000000, 1, 1);    	    
	SPI_Write(dataToSend,4);
}
/***************************************************************************//**
 * @brief Reads the value of the selected register
 *
 * @param regAddress - The address of the register to read.
 * @param size - The size of the register to read.
 *
 * @return data - The value of the selected register register.
*******************************************************************************/
unsigned long AD7793_GetRegisterValue(unsigned char regAddress, 
                                      unsigned char size,
                                      unsigned char modifyCS)
{
	unsigned char data[5]      = {0x00, 0x00, 0x00, 0x00, 0x00};
	unsigned long receivedData = 0x00;
    unsigned char i            = 0x00; 
    
	data[0] = 0x01 * modifyCS;
	data[1] = AD7793_COMM_READ |  AD7793_COMM_ADDR(regAddress); 
	SPI_Read(data,(1 + size));
	for(i = 1;i < size + 1;i ++)
    {
        receivedData = (receivedData << 8) + data[i];
    }
    
    return (receivedData);
}
/***************************************************************************//**
 * @brief Writes the value to the register
 *
 * @param -  regAddress - The address of the register to write to.
 * @param -  regValue - The value to write to the register.
 * @param -  size - The size of the register to write.
 *
 * @return  None.    
*******************************************************************************/
void AD7793_SetRegisterValue(unsigned char regAddress,
                             unsigned long regValue, 
                             unsigned char size,
                             unsigned char modifyCS)
{
	unsigned char data[5]      = {0x00, 0x00, 0x00, 0x00, 0x00};	
	unsigned char* dataPointer = (unsigned char*)&regValue;
    unsigned char bytesNr      = size + 1;
    
    data[0] = 0x01 * modifyCS;
    data[1] = AD7793_COMM_WRITE |  AD7793_COMM_ADDR(regAddress);
    while(bytesNr > 1)
    {
        data[bytesNr] = *dataPointer;
        dataPointer ++;
        bytesNr --;
    }	    
	SPI_Write(data,(1 + size));
}
/***************************************************************************//**
 * @brief  Waits for RDY pin to go low.
 *
 * @return None.
*******************************************************************************/
void AD7793_WaitRdyGoLow(void)
{
    while( AD7793_RDY_STATE )
    {
        ;
    }
}

/***************************************************************************//**
 * @brief Sets the operating mode of AD7793.
 *
 * @param mode - Mode of operation.
 *
 * @return  None.    
*******************************************************************************/
void AD7793_SetMode(unsigned long mode)
{
    unsigned long command;
    
    command = AD7793_GetRegisterValue(AD7793_REG_MODE,
                                      2,
                                      1); // CS is modified by SPI read/write functions.
    command &= ~AD7793_MODE_SEL(0xFF);
    command |= AD7793_MODE_SEL(mode);
    AD7793_SetRegisterValue(
            AD7793_REG_MODE,
            command,
            2, 
            1); // CS is modified by SPI read/write functions.
}
/***************************************************************************//**
 * @brief Selects the channel of AD7793.
 *
 * @param  channel - ADC channel selection.
 *
 * @return  None.    
*******************************************************************************/
void AD7793_SetChannel(unsigned long channel)
{
    unsigned long command;
    
    command = AD7793_GetRegisterValue(AD7793_REG_CONF,
                                      2,
                                      1); // CS is modified by SPI read/write functions.
    command &= ~AD7793_CONF_CHAN(0xF);
    command |= AD7793_CONF_CHAN(channel);
    AD7793_SetRegisterValue(
            AD7793_REG_CONF,
            command,
            2,
            1); // CS is modified by SPI read/write functions.
}

/***************************************************************************//**
 * @brief  Sets the gain of the In-Amp.
 *
 * @param  gain - Gain.
 *
 * @return  None.    
*******************************************************************************/
void AD7793_SetGain(unsigned long gain)
{
    unsigned long command;
    
    command = AD7793_GetRegisterValue(AD7793_REG_CONF,
                                      2,
                                      1); // CS is modified by SPI read/write functions.
    command &= ~AD7793_CONF_GAIN(0xFF);
    command |= AD7793_CONF_GAIN(gain);
    AD7793_SetRegisterValue(
            AD7793_REG_CONF,
            command,
            2,
            1); // CS is modified by SPI read/write functions.
}
/***************************************************************************//**
 * @brief Sets the reference source for the ADC.
 *
 * @param type - Type of the reference.
 *               Example: AD7793_REFSEL_EXT	- External Reference Selected
 *                        AD7793_REFSEL_INT	- Internal Reference Selected.
 *
 * @return None.    
*******************************************************************************/
void AD7793_SetIntReference(unsigned char type)
{
    unsigned long command = 0;
    
    command = AD7793_GetRegisterValue(AD7793_REG_CONF,
                                      2,
                                      1); // CS is modified by SPI read/write functions.
    command &= ~AD7793_CONF_REFSEL(AD7793_REFSEL_INT);
    command |= AD7793_CONF_REFSEL(type);
    AD7793_SetRegisterValue(AD7793_REG_CONF,
							command,
							2,
                            1); // CS is modified by SPI read/write functions.
}

/***************************************************************************//**
 * @brief Performs the given calibration to the specified channel.
 *
 * @param mode - Calibration type.
 * @param channel - Channel to be calibrated.
 *
 * @return none.
*******************************************************************************/
void AD7793_Calibrate(unsigned char mode, unsigned char channel)
{
    unsigned short oldRegValue = 0x0;
    unsigned short newRegValue = 0x0;
    
    AD7793_SetChannel(channel);
    oldRegValue &= AD7793_GetRegisterValue(AD7793_REG_MODE, 2, 1); // CS is modified by SPI read/write functions.
    oldRegValue &= ~AD7793_MODE_SEL(0x7);
    newRegValue = oldRegValue | AD7793_MODE_SEL(mode);
    // ADI_PART_CS_LOW; 
    AD7793_SetRegisterValue(AD7793_REG_MODE, newRegValue, 2, 1); // CS is not modified by SPI read/write functions.
    AD7793_WaitRdyGoLow();
    // ADI_PART_CS_HIGH;
    
}

/***************************************************************************//**
 * @brief Returns the result of a single conversion.
 *
 * @return regData - Result of a single analog-to-digital conversion.
*******************************************************************************/
unsigned long AD7793_SingleConversion(void)
{
    unsigned long command = 0x0;
    unsigned long regData = 0x0;
    
    command  = AD7793_MODE_SEL(AD7793_MODE_SINGLE);
    ADI_PART_CS_LOW;
    AD7793_SetRegisterValue(AD7793_REG_MODE, 
                            command,
                            2,
                            0);// CS is not modified by SPI read/write functions.
    AD7793_WaitRdyGoLow();
    regData = AD7793_GetRegisterValue(AD7793_REG_DATA, 3, 0); // CS is not modified by SPI read/write functions.
    ADI_PART_CS_HIGH;

    return(regData);
}

/***************************************************************************//**
 * @brief Returns the average of several conversion results.
 *
 * @return samplesAverage - The average of the conversion results.
*******************************************************************************/
unsigned long AD7793_ContinuousReadAvg(unsigned char sampleNumber)
{
    unsigned long samplesAverage = 0x0;
    unsigned long command        = 0x0;
    unsigned char count          = 0x0;
    
    command = AD7793_MODE_SEL(AD7793_MODE_CONT);
    // ADI_PART_CS_LOW;
    AD7793_SetRegisterValue(AD7793_REG_MODE,
                            command, 
                            2,
                            1);// CS is not modified by SPI read/write functions.
    for(count = 0;count < sampleNumber;count ++)
    {
        // AD7793_WaitRdyGoLow();
        samplesAverage += AD7793_GetRegisterValue(AD7793_REG_DATA, 3, 1);  // CS is not modified by SPI read/write functions.
    }
    // ADI_PART_CS_HIGH;
    samplesAverage = samplesAverage / sampleNumber;
    
    return(samplesAverage);
}


void adc_change_channel(uint8_t channel, uint8_t use_bias)
{
    unsigned long command = 0;
    // command |= AD7793_CONF_UNIPOLAR;
    command |= AD7793_CONF_REFSEL(AD7793_REFSEL_INT);
    if(use_bias == 1){
        command |= AD7793_CONF_VBIAS(1);
    }
    command |= AD7793_CONF_BOOST;
    command |= AD7793_CONF_BUF;
    command |= AD7793_CONF_CHAN(channel);
    command |= AD7793_CONF_GAIN(AD7793_GAIN_16);
    AD7793_SetRegisterValue(
            AD7793_REG_CONF,
            command,
            2,
            1);
}

void adc_init_channel(uint8_t channel)
{
    adc_change_channel(channel, 1);
    if(!BUTTON_ENTER_PRESSED){
        AD7793_WaitRdyGoLow();
     
        AD7793_Calibrate(AD7793_MODE_CAL_INT_ZERO,
                         channel);      // Internal Zero-Scale Calibration
        AD7793_Calibrate(AD7793_MODE_CAL_INT_FULL,
                         channel);      // Internal Full-Scale Calibration
        AD7793_SetRegisterValue(AD7793_REG_MODE,
                            0x3, 
                            2,
                            1);
    }    
}   

void adc_change_channel_and_trigger_delay(uint8_t channel)
{
    adc_change_channel(channel, 0);
    adc_prepare = 1;
    adc_ready = 0;
}


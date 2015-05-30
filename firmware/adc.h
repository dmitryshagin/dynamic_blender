/***************************************************************************//**
 *   @file   AD7793.h
 *   @brief  Header file of AD7793 Driver.
 *   @author Bancisor Mihai
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
#ifndef __AD7793_H__
#define __AD7793_H__

#include <avr/io.h>

/******************************************************************************/
/* AD7793                                                                   */
/******************************************************************************/

/* AD7793 GPIO */
#define AD7793_RDY_STATE       GPIO1_STATE

/*AD7793 Registers*/
#define AD7793_REG_COMM		0 /* Communications Register(WO, 8-bit) */
#define AD7793_REG_STAT	    0 /* Status Register	    (RO, 8-bit) */
#define AD7793_REG_MODE	    1 /* Mode Register	     	(RW, 16-bit */
#define AD7793_REG_CONF	    2 /* Configuration Register (RW, 16-bit)*/
#define AD7793_REG_DATA	    3 /* Data Register	     	(RO, 16-/24-bit) */
#define AD7793_REG_ID	    4 /* ID Register	     	(RO, 8-bit) */
#define AD7793_REG_IO	    5 /* IO Register	     	(RO, 8-bit) */
#define AD7793_REG_OFFSET   6 /* Offset Register	    (RW, 24-bit */
#define AD7793_REG_FULLSALE	7 /* Full-Scale Register	(RW, 24-bit */

/* Communications Register Bit Designations (AD7793_REG_COMM) */
#define AD7793_COMM_WEN		(1 << 7) 			/* Write Enable */
#define AD7793_COMM_WRITE	(0 << 6) 			/* Write Operation */
#define AD7793_COMM_READ    (1 << 6) 			/* Read Operation */
#define AD7793_COMM_ADDR(x)	(((x) & 0x7) << 3)	/* Register Address */
#define AD7793_COMM_CREAD	(1 << 2) 			/* Continuous Read of Data Register */

/* Status Register Bit Designations (AD7793_REG_STAT) */
#define AD7793_STAT_RDY		(1 << 7) /* Ready */
#define AD7793_STAT_ERR		(1 << 6) /* Error (Overrange, Underrange) */
#define AD7793_STAT_CH3		(1 << 2) /* Channel 3 */
#define AD7793_STAT_CH2		(1 << 1) /* Channel 2 */
#define AD7793_STAT_CH1		(1 << 0) /* Channel 1 */

/* Mode Register Bit Designations (AD7793_REG_MODE) */
#define AD7793_MODE_SEL(x)		(((x) & 0x7) << 13)	/* Operation Mode Select */
#define AD7793_MODE_CLKSRC(x)	(((x) & 0x3) << 6) 	/* ADC Clock Source Select */
#define AD7793_MODE_RATE(x)		((x) & 0xF) 		/* Filter Update Rate Select */

/* AD7793_MODE_SEL(x) options */
#define AD7793_MODE_CONT		 0 /* Continuous Conversion Mode */
#define AD7793_MODE_SINGLE		 1 /* Single Conversion Mode */
#define AD7793_MODE_IDLE		 2 /* Idle Mode */
#define AD7793_MODE_PWRDN		 3 /* Power-Down Mode */
#define AD7793_MODE_CAL_INT_ZERO 4 /* Internal Zero-Scale Calibration */
#define AD7793_MODE_CAL_INT_FULL 5 /* Internal Full-Scale Calibration */
#define AD7793_MODE_CAL_SYS_ZERO 6 /* System Zero-Scale Calibration */
#define AD7793_MODE_CAL_SYS_FULL 7 /* System Full-Scale Calibration */

/* AD7793_MODE_CLKSRC(x) options */
#define AD7793_CLK_INT		0 /* Internal 64 kHz Clk not available at the CLK pin */
#define AD7793_CLK_INT_CO	1 /* Internal 64 kHz Clk available at the CLK pin */
#define AD7793_CLK_EXT		2 /* External 64 kHz Clock */
#define AD7793_CLK_EXT_DIV2	3 /* External Clock divided by 2 */

/* Configuration Register Bit Designations (AD7793_REG_CONF) */
#define AD7793_CONF_VBIAS(x)  (((x) & 0x3) << 14) 	/* Bias Voltage Generator Enable */
#define AD7793_CONF_BO_EN	  (1 << 13) 			/* Burnout Current Enable */
#define AD7793_CONF_UNIPOLAR  (1 << 12) 			/* Unipolar/Bipolar Enable */
#define AD7793_CONF_BOOST	  (1 << 11) 			/* Boost Enable */
#define AD7793_CONF_GAIN(x)	  (((x) & 0x7) << 8) 	/* Gain Select */
#define AD7793_CONF_REFSEL(x) (((x) & 0x1) << 7) 	/* INT/EXT Reference Select */
#define AD7793_CONF_BUF		  (1 << 4) 				/* Buffered Mode Enable */
#define AD7793_CONF_CHAN(x)	  ((x) & 0x7) 			/* Channel select */

/* AD7793_CONF_GAIN(x) options */
#define AD7793_GAIN_1       0
#define AD7793_GAIN_2       1
#define AD7793_GAIN_4       2
#define AD7793_GAIN_8       3
#define AD7793_GAIN_16      4
#define AD7793_GAIN_32      5
#define AD7793_GAIN_64      6
#define AD7793_GAIN_128     7

/* AD7793_CONF_REFSEL(x) options */
#define AD7793_REFSEL_INT   1	/* Internal Reference Selected. */
#define AD7793_REFSEL_EXT   0	/* External Reference Applied between REFIN(+) and REFIN(ñ). */

/* AD7793_CONF_CHAN(x) options */
#define AD7793_CH_AIN1P_AIN1M	0 /* AIN1(+) - AIN1(-) */
#define AD7793_CH_AIN2P_AIN2M	1 /* AIN2(+) - AIN2(-) */
#define AD7793_CH_AIN3P_AIN3M	2 /* AIN3(+) - AIN3(-) */
#define AD7793_CH_AIN1M_AIN1M	3 /* AIN1(-) - AIN1(-) */
#define AD7793_CH_TEMP			6 /* Temp Sensor */
#define AD7793_CH_AVDD_MONITOR	7 /* AVDD Monitor */

/* ID Register Bit Designations (AD7793_REG_ID) */
#define AD7793_ID			0xB
#define AD7793_ID_MASK		0xF

/* IO (Excitation Current Sources) Register Bit Designations (AD7793_REG_IO) */
#define AD7793_IEXCDIR(x)	(((x) & 0x3) << 2)
#define AD7793_IEXCEN(x)	(((x) & 0x3) << 0)

/* AD7793_IEXCDIR(x) options*/
#define AD7793_DIR_IEXC1_IOUT1_IEXC2_IOUT2	0  /* IEXC1 connect to IOUT1, IEXC2 connect to IOUT2 */
#define AD7793_DIR_IEXC1_IOUT2_IEXC2_IOUT1	1  /* IEXC1 connect to IOUT2, IEXC2 connect to IOUT1 */
#define AD7793_DIR_IEXC1_IEXC2_IOUT1		2  /* Both current sources IEXC1,2 connect to IOUT1  */
#define AD7793_DIR_IEXC1_IEXC2_IOUT2		3  /* Both current sources IEXC1,2 connect to IOUT2 */

/* AD7793_IEXCEN(x) options*/
#define AD7793_EN_IXCEN_10uA				1  /* Excitation Current 10uA */
#define AD7793_EN_IXCEN_210uA				2  /* Excitation Current 210uA */
#define AD7793_EN_IXCEN_1mA					3  /* Excitation Current 1mA */

/******************************************************************************/
/* Functions Prototypes                                                       */
/******************************************************************************/

/* Initialize AD7793 and check if the device is present*/
unsigned char AD7793_Init(void);

/* Sends 32 consecutive 1's on SPI in order to reset the part. */
void AD7793_Reset(void);

/* Reads the value of the selected register. */
unsigned long AD7793_GetRegisterValue(unsigned char regAddress, 
									  unsigned char size,
                                      unsigned char modifyCS);

/* Writes a value to the register. */
void AD7793_SetRegisterValue(unsigned char regAddress,
							 unsigned long regValue, 
							 unsigned char size,
                             unsigned char modifyCS);

/* Waits for RDY pin to go low. */
void AD7793_WaitRdyGoLow(void);

/* Sets the operating mode of AD7793. */
void AD7793_SetMode(unsigned long mode);     

/* Selects the channel of AD7793. */
void AD7793_SetChannel(unsigned long channel);

/* Sets the gain of the In-Amp. */
void AD7793_SetGain(unsigned long gain);

/* Sets the reference source for the ADC. */
void AD7793_SetIntReference(unsigned char type);

/* Performs the given calibration to the specified channel. */
void AD7793_Calibrate(unsigned char mode, unsigned char channel);

/* Returns the result of a single conversion. */
unsigned long AD7793_SingleConversion(void);

/* Returns the average of several conversion results. */
unsigned long AD7793_ContinuousReadAvg(unsigned char sampleNumber);

#endif	// _AD7793_H_

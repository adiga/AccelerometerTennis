/*
 * accerelometerDriver.c
 * Functions definition of accelerometer driver
 *  Created on: Nov 20, 2010
 *      Author: Vikram Adiga/Sudersan Sampath
 */
#include "accelerometerDriver.h"
#include "support_common.h"
#include <stdio.h>

/**
 *  initializes accelerometer and pot. 
 *  Requires mode to set acceleration
 */

void init_accelerometer_and_pot (int mode){

	//set port T (GPT)
	MCF_GPIO_PTAPAR = MCF_GPIO_PTAPAR_GPT0_GPIO | MCF_GPIO_PTAPAR_GPT1_GPIO | MCF_GPIO_PTAPAR_GPT2_GPIO;
	MCF_GPIO_DDRTA = MCF_GPIO_DDRTA_DDRTA0 | MCF_GPIO_DDRTA_DDRTA1 | MCF_GPIO_DDRTA_DDRTA2;
	//The g-select are selected based on bit0 and bit1 of mode and SLEEP is pulled high
	MCF_GPIO_PORTTA = (unsigned char)(mode & 0x1) | (unsigned char)(mode & 0x2) | MCF_GPIO_PORTTA_PORTTA2;

	//set PORT AN
	MCF_GPIO_PORTAN = 0;
	MCF_GPIO_PANPAR = MCF_GPIO_PANPAR_AN0_AN0 | MCF_GPIO_PANPAR_AN4_AN4 | MCF_GPIO_PANPAR_AN5_AN5 | MCF_GPIO_PANPAR_AN6_AN6;
	MCF_GPIO_DDRAN =  0;

	//set LED
	MCF_GPIO_PTCPAR = MCF_GPIO_PTCPAR_DTIN3_GPIO | MCF_GPIO_PTCPAR_DTIN2_GPIO | MCF_GPIO_PTCPAR_DTIN1_GPIO | MCF_GPIO_PTCPAR_DTIN0_GPIO;
	MCF_GPIO_DDRTC = MCF_GPIO_DDRTC_DDRTC3 | MCF_GPIO_DDRTC_DDRTC2 | MCF_GPIO_DDRTC_DDRTC1 | MCF_GPIO_DDRTC_DDRTC0;

	//setting supervisory mode for changing adc
	asm{
		move.w        #0x2700,sr
	}
	//set ADC
	//single ended and once parallel
	MCF_ADC_CTRL1 = MCF_ADC_CTRL1_CHNCFG(0) | MCF_ADC_CTRL1_SMODE(001);
	//simultaneous scan A & B and ROSC Normal - 2MHZ
	MCF_ADC_CTRL2 = MCF_ADC_CTRL2_SIMULT | MCF_ADC_CTRL2_DIV(0x0);
	//Disable Zero crossing
	MCF_ADC_ADZCC = 0;
	//Sample 0 
	MCF_ADC_ADLST1 = MCF_ADC_ADLST1_SAMPLE0(0x0);
	//Sample 4 , 5 ,6 
	MCF_ADC_ADLST2 = MCF_ADC_ADLST2_SAMPLE4(0x4) | MCF_ADC_ADLST2_SAMPLE5(0x5) | MCF_ADC_ADLST2_SAMPLE6(0x6); 
	//disable last sample	
	MCF_ADC_ADSDIS = MCF_ADC_ADSDIS_DS7;

	//For positive results
	MCF_ADC_ADOFS0 = MCF_ADC_ADOFS_OFFSET(0x0000);
	MCF_ADC_ADOFS4 = MCF_ADC_ADOFS_OFFSET(0x0000);
	MCF_ADC_ADOFS5 = MCF_ADC_ADOFS_OFFSET(0x0000);
	MCF_ADC_ADOFS6 = MCF_ADC_ADOFS_OFFSET(0x0000);

	//disabling Lower and Higher limits
	MCF_ADC_ADLLMT0 = 0;
	MCF_ADC_ADHLMT0 = MCF_ADC_ADHLMT_HLMT(0x7ff8);
	MCF_ADC_ADLLMT1 = 0;
	MCF_ADC_ADHLMT1 = MCF_ADC_ADHLMT_HLMT(0x7ff8);
	MCF_ADC_ADLLMT2 = 0;
	MCF_ADC_ADHLMT2 = MCF_ADC_ADHLMT_HLMT(0x7ff8);
	MCF_ADC_ADLLMT3 = 0;
	MCF_ADC_ADHLMT3 = MCF_ADC_ADHLMT_HLMT(0x7ff8);
	MCF_ADC_ADLLMT4 = 0;
	MCF_ADC_ADHLMT4 = MCF_ADC_ADHLMT_HLMT(0x7ff8);
	MCF_ADC_ADLLMT5 = 0;
	MCF_ADC_ADHLMT5 = MCF_ADC_ADHLMT_HLMT(0x7ff8);
	MCF_ADC_ADLLMT6 = 0;
	MCF_ADC_ADHLMT6 = MCF_ADC_ADHLMT_HLMT(0x7ff8);
	MCF_ADC_ADLLMT7 = 0;
	MCF_ADC_ADHLMT7 = MCF_ADC_ADHLMT_HLMT(0x7ff8);


	//Selecting voltage reference 
	MCF_ADC_CAL = 0;

	// Power up ADC converters
	MCF_ADC_POWER = MCF_ADC_POWER_PUDELAY(0xd) | MCF_ADC_POWER_PD2;
	MCF_SCM_PPMRH &= ~MCF_SCM_PPMRH_CDADC;

	// Wait for converter A power up to complete 
	while (MCF_ADC_POWER & MCF_ADC_POWER_PSTS0){ asm{TPF};}


	// Wait for converter B power up to complete
	while (MCF_ADC_POWER & MCF_ADC_POWER_PSTS1){ asm{TPF};}
		
}
/**
 * Samples the data specified by which and loads the struct.
 * Can read x,y,z,pot individually.
 */
void sample_accelerometer_and_pot (int which, struct accel_data * datap){
	
	
	//disable samples based the data
	switch(which){
	case 1:
		MCF_ADC_ADSDIS = MCF_ADC_ADSDIS_DS0 | MCF_ADC_ADSDIS_DS1 | MCF_ADC_ADSDIS_DS2 | MCF_ADC_ADSDIS_DS3 | 
						 MCF_ADC_ADSDIS_DS5 | MCF_ADC_ADSDIS_DS6 | MCF_ADC_ADSDIS_DS7 ;
		break;
	case 2:
		MCF_ADC_ADSDIS = MCF_ADC_ADSDIS_DS0 | MCF_ADC_ADSDIS_DS1 | MCF_ADC_ADSDIS_DS2 | MCF_ADC_ADSDIS_DS3 | 
						 MCF_ADC_ADSDIS_DS4 | MCF_ADC_ADSDIS_DS6 | MCF_ADC_ADSDIS_DS7 ;
		break;
	case 4: 
		MCF_ADC_ADSDIS = MCF_ADC_ADSDIS_DS0 | MCF_ADC_ADSDIS_DS1 | MCF_ADC_ADSDIS_DS2 | MCF_ADC_ADSDIS_DS3 | 
						 MCF_ADC_ADSDIS_DS4 | MCF_ADC_ADSDIS_DS5 | MCF_ADC_ADSDIS_DS7 ;
		break;
	case 8: 
		MCF_ADC_ADSDIS =  MCF_ADC_ADSDIS_DS1 | MCF_ADC_ADSDIS_DS2 | MCF_ADC_ADSDIS_DS3 | 
						  MCF_ADC_ADSDIS_DS4 | MCF_ADC_ADSDIS_DS5 | MCF_ADC_ADSDIS_DS6 | MCF_ADC_ADSDIS_DS7 ;
		break;
	default: break;
	}
	
	
	// Start ATD conversion 
	MCF_ADC_CTRL1 |= MCF_ADC_CTRL1_START0;
	
	
	// wait for completion of scan
	while(!(MCF_ADC_ADSTAT & MCF_ADC_ADSTAT_EOSI0)){asm{TPF};}

	
	//read the sample based the data required
	switch(which){
	case 1:
		while(!(MCF_ADC_ADSTAT & MCF_ADC_ADSTAT_RDY4)){asm{TPF};}
		datap->x = ((MCF_ADC_ADRSLT(4)& 0x7FF8) >> 3);
		break;
	case 2:
		while(!(MCF_ADC_ADSTAT & MCF_ADC_ADSTAT_RDY5)){asm{TPF};}
		datap->y = ((MCF_ADC_ADRSLT(5)& 0x7FF8) >> 3);
		break;
	case 4:
		while(!(MCF_ADC_ADSTAT & MCF_ADC_ADSTAT_RDY6)){asm{TPF};}
		datap->z = ((MCF_ADC_ADRSLT(6)& 0x7FF8) >> 3);
		break;
	case 8:
		while(!(MCF_ADC_ADSTAT & MCF_ADC_ADSTAT_RDY0)){asm{TPF};}
		datap->pot = ((MCF_ADC_ADRSLT(0)& 0x7FF8) >> 3);
		break;
	default: break;
	}

	// clear end of scan
	MCF_ADC_ADSTAT = MCF_ADC_ADSTAT_EOSI0;

}

/*
 * accelerometerTennis.c
 * 
 *  Created on: Nov 29, 2010
 *      Author: Vikram Adiga
 */
#include "accelerometerTennis.h"
#include <stdio.h>
//prototypes
__declspec(interrupt:0) void handler_pit0_int (void);
__declspec(interrupt:0) void handler_irq7_int (void);
__declspec(interrupt:0) void handler_irq4_int (void);
static unsigned char *fnSetIntHandler(int iVectNumber, unsigned char *new_handler);
static void init_interrupt_controller (void);
void enable_interrupts(void);


static unsigned short randDistrubution(unsigned short potValue);
static int flick_detector(int flag);
static int readpot_enabletimer(int readflag);
static void disabletimer(void);

//globals reqd 
static int led = 0, volley_count = 0 , completion_flag = 0 ,led_dir = 1, sw1_flag = 0;

//initializing the tennis
void init_tennis(){
	int i = 0;
	//init accelerometer and pot with 00 = 1.5g
	init_accelerometer_and_pot(0);

	//read flick values to reach equilibrium 
	for(i=0;i<=10000;i++)
		flick_detector(0);

	//set timer interrupt
	fnSetIntHandler(119,(unsigned char *)handler_pit0_int);

	//switches
	fnSetIntHandler(71, (unsigned char *)handler_irq7_int);
	fnSetIntHandler(68, (unsigned char *)handler_irq4_int);
	init_interrupt_controller();
	enable_interrupts();
		
}

//start the game
void accel_tennis(){
    int flick_value ,i, potvalue;
    
    
	//forever
	while(1){
		
		//game loop 	
		while (volley_count < 6){
			//reset the values
			sw1_flag = 0;
			flick_value = 0;		
			
			
			//reset completion flag
			//set the flag for forward display
			// read the pot and set the timer
			completion_flag = 0;
			led_dir = 1;
			potvalue = readpot_enabletimer(1);

			//check for completion of process
			while((!completion_flag) && (!sw1_flag)){
				//start reading the flick detector
				flick_detector(1);
			}
			
			//stop timer 
			disabletimer();
			
			//time allowed for flick detection
			for(i = 0; i < (potvalue * 10); i++){
				flick_value |= flick_detector(1);
			}

			if( flick_value && (!sw1_flag)){
				
				//reset completion flag
				//set the timer 
				//set the flag for reverse display
				completion_flag = 0;
				led_dir = 0;
				readpot_enabletimer(0);
				//check for completion of process
				while((!completion_flag) && (!sw1_flag)){asm{TPF};}
				//stop timer
				disabletimer();
			}
			if(!sw1_flag)
				volley_count++;
			
			
			//random delay
			for(i = 0; i < 100000; i++);
		}

		//inactive state
		while(!sw1_flag){asm {TPF};}

	}

}
//returns pot value
static int readpot_enabletimer(int readflag){
	
	int which = 8;
	struct accel_data *potdatap;
	unsigned short potvalue = 0;
	//read the flag to check if the timer is to be set
	if(readflag){
		
		potdatap = (struct accel_data *)malloc(sizeof(struct accel_data));
		sample_accelerometer_and_pot(which, potdatap);
		potvalue = potdatap->pot;
		
		//set the speed
		MCF_PIT0_PCSR &=  ~MCF_PIT_PCSR_EN;
		MCF_PIT_PMR(0) = randDistrubution(potvalue) * 16;
		
		//Timer interrupt
		MCF_PIT0_PCSR = MCF_PIT_PCSR_PIE | MCF_PIT_PCSR_PIF | MCF_PIT_PCSR_RLD | MCF_PIT_PCSR_PRE(7);
		
		free(potdatap);
	}
	
	//enable timer
	MCF_PIT0_PCSR |=  MCF_PIT_PCSR_EN;
		
	return potvalue;
	    
}


static void disabletimer(){
	//disable timer
	MCF_PIT0_PCSR &=  ~MCF_PIT_PCSR_EN;
}

/**
 * Distribution centred around pot
 */

static unsigned short randDistrubution(unsigned short potValue){
	 unsigned short random = (unsigned short)(potValue + ((rand() - 16384) / 100)); 
	  if(random < 0)
		return 100;
	  else
		return random; 
}

//flick detection code
static int flick_detector(int flag){
	static int prev_flick = 0 ;
	int which = 1;
	struct accel_data *datap = (struct accel_data *)malloc(sizeof(struct accel_data));
	datap->x = 0;
	datap->y = 0;
	datap->z = 0;
	datap->pot = 0;

	sample_accelerometer_and_pot(which, datap);
    //calibration to find to the mean
	if(flag == 0){
		prev_flick = ((MCF_ADC_ADRSLT(4)& 0x7FF8) >> 3);
	
	}else if((datap->x - prev_flick) > 600){
		free(datap);
		//flick detected
		return 1;
	}else if((prev_flick - datap->x) > 600){
		free(datap);
		//flick detected
		return 1;
	}
	free(datap);

	return 0;
}
//timer handler
__declspec(interrupt:0) void handler_pit0_int (void)
{
   //the led direction depends on serve/return
	if(led_dir)
	{
		switch(led)
		{
		case 0: MCF_GPIO_PORTTC = MCF_GPIO_PORTTC_PORTTC3;
		break;
		case 1: MCF_GPIO_PORTTC = MCF_GPIO_PORTTC_PORTTC2;
		break;
		case 2: MCF_GPIO_PORTTC = MCF_GPIO_PORTTC_PORTTC1;
		break;
		case 3: MCF_GPIO_PORTTC = MCF_GPIO_PORTTC_PORTTC0;
		break;
		case 4: MCF_GPIO_PORTTC = 0;
		completion_flag = 1;
		led = -1;
		break;
		default: MCF_GPIO_PORTTC = MCF_GPIO_PORTTC_PORTTC0 | MCF_GPIO_PORTTC_PORTTC1; //shouldnt happen         
		}
	}else {
		switch(led)
		{
		case 0: MCF_GPIO_PORTTC = MCF_GPIO_PORTTC_PORTTC0;
		break;
		case 1: MCF_GPIO_PORTTC = MCF_GPIO_PORTTC_PORTTC1;
		break;
		case 2: MCF_GPIO_PORTTC = MCF_GPIO_PORTTC_PORTTC2;
		break;
		case 3: MCF_GPIO_PORTTC = MCF_GPIO_PORTTC_PORTTC3;
		break;
		case 4: MCF_GPIO_PORTTC = 0;
		completion_flag = 1;
		led = -1;
		break;
		default: MCF_GPIO_PORTTC = MCF_GPIO_PORTTC_PORTTC2 | MCF_GPIO_PORTTC_PORTTC3; //shouldnt happen
		}
	}

	led++;
	MCF_PIT0_PCSR |= MCF_PIT_PCSR_PIF;

}

//sw2  handler - flick debug mode
__declspec(interrupt:0) void handler_irq7_int (void)
{
	volatile int i = 0;
	MCF_EPORT0_EPIER &= ~MCF_EPORT_EPIER_EPIE7;
	disabletimer();
	MCF_GPIO_PORTTC = 0;
	
	//do this till SW1 or reset is pressed
	//check the interrupt bit for SW1 to break the loop
	while((MCF_INTC0_IPRL & MCF_INTC_IPRL_INT4) == 0){
		//chk flick detector
		if(flick_detector(1)){
	
			MCF_GPIO_PORTTC = MCF_GPIO_PORTTC_PORTTC0 | MCF_GPIO_PORTTC_PORTTC1 | MCF_GPIO_PORTTC_PORTTC2 | MCF_GPIO_PORTTC_PORTTC3;
			//approx delay
			for(i = 0; i < 1000000; i++){asm {TPF};}
			MCF_GPIO_PORTTC = 0;
		}
	}
	MCF_EPORT0_EPFR |= (MCF_EPORT_EPFR_EPF7);
	MCF_EPORT0_EPIER |= MCF_EPORT_EPIER_EPIE7;
	
}
//sw1 handler - resets flags and counter
__declspec(interrupt:0) void handler_irq4_int (void)
{
	MCF_EPORT0_EPIER &= ~MCF_EPORT_EPIER_EPIE4;
	disabletimer();
	volley_count = 0;
	sw1_flag = 1;
	led = 0;
	MCF_EPORT0_EPFR |= (MCF_EPORT_EPFR_EPF4);
	MCF_EPORT0_EPIER |= MCF_EPORT_EPIER_EPIE4;
	//printf("Interrupt4\n");
	
}
//set up the inteerupt handler in vector table
static unsigned char *fnSetIntHandler(int iVectNumber, unsigned char *new_handler)
{
	extern unsigned long __VECTOR_RAM[];
	unsigned char *old_handler;

	old_handler = (unsigned char *)__VECTOR_RAM[iVectNumber];
	__VECTOR_RAM[iVectNumber] = (unsigned long)new_handler;
	return old_handler;     
}
//initializes all interrupts
static void init_interrupt_controller (void)
{
	//set the interrupt for IRQ 4 and IRQ 7
	MCF_EPORT0_EPPAR = MCF_EPORT_EPPAR_EPPA4_LEVEL |MCF_EPORT_EPPAR_EPPA7_LEVEL ;
	//Direction input
	MCF_EPORT0_EPDDR = 0x00;
	//Enable
	MCF_EPORT0_EPIER = MCF_EPORT_EPIER_EPIE4 | MCF_EPORT_EPIER_EPIE7;

	//Timer interrupt
	MCF_INTC0_ICR55 |= MCF_INTC_ICR_IL (0x2) | MCF_INTC_ICR_IP (0x2);
	MCF_INTC0_IMRH &= (~MCF_INTC_IMRH_INT_MASK55);
	MCF_INTC0_IMRL &= (~MCF_INTC_IMRL_MASKALL);


}
//enable interrupt
void enable_interrupts(void)
{
	asm {
		move.l   #0x00002000,d0
		move.w   d0,SR
	}
}

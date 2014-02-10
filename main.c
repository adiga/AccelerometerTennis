/*
 * main implementation: use this sample to create your own application
 * Accelerometer Tennis 
 * Authors: Vikram Adiga/Sudersan Sampath
 */


#include "support_common.h" /* include peripheral declarations and more */
#include "accelerometerTennis.h"
#include <stdio.h>

int main(void)
{
	//init 
	init_tennis();
	//start the game
	accel_tennis();
		
}

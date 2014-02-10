/*
 * accerelometerDriver.h
 * Prototype decleration for accelerometer driver
 *  Created on: Nov 20, 2010
 *      Author: Vikram Adiga/Sudersan Sampath
 */

#ifndef ACCERELOMETERDRIVER_H_
#define ACCERELOMETERDRIVER_H_
//struct hold data
struct accel_data {
	int x, y, z, pot;
};
//initializes accelerometer and pot. Requires mode to set acceleration
void init_accelerometer_and_pot (int mode);
//samples data and stores it into struct
void sample_accelerometer_and_pot (int which, struct accel_data * datap);

#endif /* ACCERELOMETERDRIVER_H_ */

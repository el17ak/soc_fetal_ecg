/*
 * display.c
 *
 *  Created on: 6 May 2022
 *      Author: el17a
 */

#include "display.h"

unsigned short display_set(float *dataset, unsigned int n_samples){
	int i;
	unsigned int counter = 0;
	unsigned short colour = 0;
	unsigned short timer;
	while(counter < (n_samples - 320)){
		timer = 0;
		while(timer < 10){
			timer++;
		}
		LT24_clearDisplay(65535);
		for(i = 0; i < 320; i++){
			signed int a = (dataset[counter+i] < 0)? (120 - (dataset[counter+i]/10)): (120 - (dataset[counter+i]/10));
			if(a >= 0 && a < 240){
				signed int flag = LT24_drawPixel(colour, a, i);
			}
		}
		counter++;
		HPS_ResetWatchdog();
	}
	return 1;
}

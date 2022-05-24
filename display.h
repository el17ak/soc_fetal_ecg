/*
 * display.h
 *
 *  Created on: 7 May 2022
 *      Author: el17a
 */

#ifndef DISPLAY_H_
#define DISPLAY_H_

#include "../Drivers/DE1SoC_LT24/DE1SoC_LT24.h"
#include "../Drivers/HPS_Watchdog/HPS_Watchdog.h"
#include <stdbool.h>
#include <stdio.h>

unsigned short display_set(float **dataset, unsigned int n_samples);

#endif /* DISPLAY_H_ */

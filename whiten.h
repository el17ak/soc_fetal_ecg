/*
 * whiten.h
 *
 *  Created on: 27 Apr 2022
 *      Author: el17a
 */

#ifndef WHITEN_H_
#define WHITEN_H_

#include <stdlib.h>
#include <stdio.h>
#include "math_helpers/matrix_operators.h"
#include "../Drivers/HPS_Watchdog/HPS_Watchdog.h"

float **whiten(int **data, int SIZE_N, int SIZE_M);

#endif /* WHITEN_H_ */

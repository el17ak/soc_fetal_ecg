/*
 * fastica.h
 *
 *  Created on: 27 Apr 2022
 *      Author: el17a
 */

#ifndef FASTICA_H_
#define FASTICA_H_

#include <stdlib.h>
#include <math.h>
#include "math_helpers/matrix_operators.h"
#include "math_helpers/hyperfunctions.h"

float **transform(float **whitened_data, unsigned short SIZE_N, unsigned short SIZE_M, unsigned short SIZE_C);

#endif /* FASTICA_H_ */

/*
 * matrix_operators.h
 *
 *  Created on: 27 Apr 2022
 *      Author: el17a
 */

#ifndef MATH_HELPERS_MATRIX_OPERATORS_H_
#define MATH_HELPERS_MATRIX_OPERATORS_H_

#include <stdlib.h>
#include <math.h>
#include <stdio.h>
#include <string.h>
#include "../Drivers/HPS_Watchdog/HPS_Watchdog.h"

float **transpose_matrix(float **mat, unsigned short size[2]);

float **add_matrices(float **mat_a, float **mat_b, unsigned short size[2]);

float **substract_matrices(float **mat_a, float **mat_b, unsigned short size[2]);

float **multiply_matrices(float **mat_a, float **mat_b, unsigned short sizes[3]);

float **substract_matrix_scalar(float **mat, float scalar, unsigned short size[2]);

float **multiply_matrix_scalar(float **mat, float scalar, unsigned short size[2]);

float **divide_matrix_scalar(float **mat, float scalar, unsigned short size[2]);

float frobenius_norm(float **mat, unsigned short size[2]);

#endif /* MATH_HELPERS_MATRIX_OPERATORS_H_ */

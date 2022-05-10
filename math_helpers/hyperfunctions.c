/*
 * hyperfunctions.c
 *
 *  Created on: 14 Apr 2022
 *      Author: Anna Irma Elizabeth KENNEDY
 */
#include "hyperfunctions.h"

float **function_g(float **vector, unsigned short size){
	float **result;
	int i;
	for(i = 0; i < size; i++){
		result[i][0] = vector[i][0] * exp(0 - (pow(vector[i][0], 2) / 2));
	}
	return result;
}

float **function_g_prime(float **vector, unsigned short size){
	float **result;
	int i;
	for(i = 0; i < size; i++){
		float squared = pow(vector[i][0], 2);
		result[i][0] = (1 - squared) * exp(0 - squared / 2);
	}
	return result;
}

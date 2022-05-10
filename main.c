#include <stdio.h>
#include "file_handler.h"
#include "whiten.h"
#include "fastica.h"
#include "display.h"
#include "../Drivers/HPS_Watchdog/HPS_Watchdog.h"
#include "../Drivers/DE1SoC_LT24/DE1SoC_LT24.h"
#include "sub01_fecg1.h"

int main(void){
	unsigned short SIZE_N = 8;
	unsigned short SIZE_M = 512;
	unsigned short SIZE_C = 2;
	int **data;

	float **whitened_data;
	float **transformed_data; // (C x M)

	int i, j, k;

	//Allocate memory for input data
	data = (int**)malloc(sizeof(int*) * SIZE_N);
	for(i = 0; i < SIZE_N; i++) data[i] = (int*)malloc(sizeof(int) * SIZE_M);

	//How to retrieve data from FPGA input?
	//data = read_matrix(&rows, &cols, "sub01_fecg1.txt");
	for(i = 0; i < SIZE_N; i++){
		for(j = 0; j < SIZE_M; j++){
			data[i][j] = (0x8000&(sub01_fecg1[(j*8)+i]) ? (int)(0x7FFF&(sub01_fecg1[(j*8)+i]))-0x8000 : (sub01_fecg1[(j*8)+i]));
		}
	}
	HPS_ResetWatchdog();


	/*for(j = 0; j < SIZE_N; j++){
		for(k = 0; k < SIZE_M; k++){
			printf("%d ", data[j][k]);
		}
		printf("\n");
	}*/

	//Initialise LCD
	LT24_initialise(0xFF200060, 0xFF200080);
	HPS_ResetWatchdog();

	printf("Whitening\n");
	whitened_data = whiten(data, SIZE_N, SIZE_M);
	printf("Transforming\n");
	transformed_data = transform(whitened_data, SIZE_N, SIZE_M, SIZE_C);

	//Free allocated memory for input data
	for(i = 0; i < SIZE_N; i++) free(data[i]);
	free(data);

	// Display a single IC of the signal
	display_set(transformed_data[0], 512);
}

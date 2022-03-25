#include <stdio.h>

int main(void){
	int SIZE_N = 8;
	int SIZE_M = 512;
	int data[SIZE_N][SIZE_M];

	double **whitened_data;
	whitened_data = whiten(data, SIZE_N, SIZE_M);

	double **transformed_data;

	printf("Hello");
}

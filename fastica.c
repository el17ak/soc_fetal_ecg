double **transform(double whitened_data[][], int SIZE_N, int SIZE_M){
	double **res;
	res = malloc(sizeof(float*) * SIZE_N);
	for(int i = 0; i < SIZE_M; i++){
		res[i] = malloc(sizeof(float*) * SIZE_M);
	}



	return res;
}

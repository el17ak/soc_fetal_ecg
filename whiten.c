
double **whiten(int data[][], int SIZE_N, int SIZE_M){

	//Allocate memory for the result matrix (N x M)
	double **res;
	res = malloc(sizeof(float*) * SIZE_N);
	for(int i = 0; i < SIZE_M; i++){
		res[i] = malloc(sizeof(float*) * SIZE_M);
	}

	//Compute mean row vector (1 x M) of the data matrix


	//Substract mean row vector from the columns of the data matrix to obtain the centered matrix (N x M)


	//Compute the covariance matrix for the centered matrix D -> (N x N)





	//Finalise computation by projecting data onto the PCA whitening space


	return res;

}

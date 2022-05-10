#define epsilon 0.0001

#include "whiten.h"

float **whiten(int **data, int SIZE_N, int SIZE_M){

	int *sum;
	int *mean;
	float **centered_data; // (N x M)
	int i, j, k;
	float **covariance_matrix; // (N x N)
	float **vectors[2];
	float **eigenvectors;
	float *eigenvalues;
	float **vector_t;
	float **w_matrix;
	float **product;
	float **mult_term, **mult_res;

	//Allocate memory for the result matrix (N x M)
	float **res;

	unsigned short size_a[2], size_b[3], size_c[3], size_d[3], size_e[3], size_f[2], size_g[3], size_v[2];
	size_a[0] = SIZE_N; size_a[1] = SIZE_M;
	size_b[0] = SIZE_N; size_b[1] = SIZE_M; size_b[2] = SIZE_N;
	size_c[0] = SIZE_N; size_c[1] = SIZE_N; size_c[2] = 1;
	size_d[0] = 1; size_d[1] = SIZE_N; size_d[2] = SIZE_N;
	size_e[0] = 1; size_e[1] = SIZE_N; size_e[2] = 1;
	size_f[0] = SIZE_N; size_f[1] = SIZE_N;
	size_g[0] = SIZE_N; size_g[1] = SIZE_N; size_g[2] = SIZE_M;
	size_v[0] = SIZE_N; size_v[1] = 1;

	sum = (int*)malloc(sizeof(int) * SIZE_M);
	mean = (int*)malloc(sizeof(int) * SIZE_M);
	res = (float**)malloc(sizeof(float**) * SIZE_N);
	centered_data = (float**)malloc(sizeof(float*) * SIZE_N);
	for(i = 0; i < SIZE_N; i++){
		res[i] = (float*)malloc(sizeof(float*) * SIZE_M);
		centered_data[i] = (float*)malloc(sizeof(float) * SIZE_M);
	}

	//Allocate memory for recursive vectors
	vectors[0] = (float**)malloc(sizeof(float*) * SIZE_N);
	vectors[1] = malloc(sizeof(float**) * SIZE_N);
	for(i = 0; i < SIZE_N; i++){
		vectors[0][i] = (float*)malloc(sizeof(float));
		vectors[1][i] = (float*)malloc(sizeof(float));
	}

	eigenvalues = (float*)malloc(sizeof(float) * SIZE_N);

	HPS_ResetWatchdog();
	//Compute mean row vector (1 x M) of the data matrix

	for(j = 0; j < SIZE_M; j++){
		for(i = 0; i < SIZE_N; i++){
			sum[j] = sum[j] + data[i][j];
		}
		HPS_ResetWatchdog();
		//printf("%d\n", sum[j]);
		mean[j] = (sum[j]/SIZE_N);
		HPS_ResetWatchdog();
		for(k = 0; k < SIZE_N; k++){
			//Subtract mean row vector from the columns of the data matrix to obtain the centered matrix (N x M)
			centered_data[k][j] = (float)(data[k][j] - mean[j]);
		}
		HPS_ResetWatchdog();
	}

	//Compute the covariance matrix for the centered matrix D -> (N x N)
	covariance_matrix = multiply_matrices(centered_data, transpose_matrix(centered_data, size_a), size_b);

	for(i = 0; i < SIZE_N; i++){
		vectors[0][i][0] = rand() % 5;
	}
	vectors[1] = multiply_matrices(covariance_matrix, vectors[0], size_c);

	//For each of the N channels of data
	for(i = 0; i < SIZE_N; i++){

		//While the following eigenvector differs from the previous one by at least epsilon
		while(frobenius_norm(substract_matrices(vectors[0], vectors[1], size_v), size_v) > epsilon){ //Convergence verification

			vectors[0] = vectors[1]; //Adjustment when moving to next iteration of while loop

			product = multiply_matrices(covariance_matrix, vectors[0], size_c);

			vectors[1] = divide_matrix_scalar(product, frobenius_norm(product, size_v), size_v); //Compute next iteration of eigenvector
		}

		//When the vectors have converged, we retrieve it to our final eigenvectors
		for(j = 0; j < SIZE_N; j++){
			eigenvectors[j][i] = vectors[1][j][0];
		}

		vector_t = transpose_matrix(vectors[1], size_v);

		//Find the corresponding eigenvalue to the eigenvector
		eigenvalues[i] = multiply_matrices(multiply_matrices(vector_t, covariance_matrix, size_d), vectors[1], size_e)[0][0];

		mult_term = multiply_matrices(vector_t, vectors[i], size_e)[0][0];
		__asm{
			VMUL mult_res, eigenvalues[i], mult_term
		}
		//Update the covariance matrix by removing some of the influence of the vector just calculated
		covariance_matrix = substract_matrix_scalar(covariance_matrix, mult_res, size_f);

		for(j = 0; j < SIZE_N; j++) w_matrix[j][i] = eigenvectors[i][j];
	}

	//Finalise computation by projecting data onto the PCA whitening space
	res = multiply_matrices(eigenvectors, centered_data, size_g);

	return res;
}

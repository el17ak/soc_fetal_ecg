#include "fastica.h"

float **transform(float **whitened_data, unsigned short SIZE_N, unsigned short SIZE_M, unsigned short SIZE_C){

	float **result, **w;
	int i, j, p;
	float **current_w, **previous_w, **w_prime, **w_second, **transpose_w_prime;
	float **common_term, **result_g, **term_left, **result_g_prime, **term_right, **sum_vectors;
	float **column_m;
	float **temp_w;

	//Size definitions for passing to math helpers
	unsigned short size_w[2], size_a[3], size_b[2], size_c[3], size_d[3], size_e[3], size_f[3];
	size_w[0] = SIZE_N; size_w[1] = 1;
	size_a[0] = 1; size_a[1] = SIZE_N; size_a[2] = SIZE_M;
	size_b[0] = 1; size_b[1] = SIZE_M;
	size_c[0] = SIZE_N; size_c[1] = SIZE_M; size_c[2] = 1;
	size_d[0] = 1; size_d[1] = SIZE_M; size_d[2] = 1;
	size_e[0] = 1; size_e[1] = SIZE_N; size_e[2] = 1;
	size_f[0] = SIZE_N; size_f[1] = SIZE_N; size_f[2] = SIZE_M;

	//Memory allocations
	current_w = (float**)malloc(sizeof(float*) * SIZE_N);
	result = (float**)malloc(sizeof(float*) * SIZE_N);
	column_m = (float**)malloc(sizeof(float*) * SIZE_M);
	w = (float**)malloc(sizeof(float*) * SIZE_N);
	for(j = 0; j < SIZE_C; j++){
		w[j] = (float*)malloc(sizeof(float) * SIZE_N);
	}
	for(j = 0; j < SIZE_M; j++){
		column_m[j] = (float*)malloc(sizeof(float));
		column_m[j][0] = 1;
	}
	for(j = 0; j < SIZE_N; j++){
		result[j] = (float*)malloc(sizeof(float) * SIZE_M);
		current_w[j] = (float*)malloc(sizeof(float));
	}

	//Main FastICA loop (once for every Independent Component)
	for(p = 0; p < SIZE_C; p++){
		for(i = 0; i < SIZE_N; i++){ //Initialise all elements of first vector
			current_w[i][0] = (float)(rand() % 100); //Generate values between 0 and 100
		}

		//Convergence while loop
		while(frobenius_norm(substract_matrices(current_w, previous_w, size_w), size_w)){
			previous_w = current_w; //Store the previous iteration's result

			//(w^T)*X
			common_term = multiply_matrices(transpose_matrix(current_w, size_w), whitened_data, size_a);

			//g((w^T)*X)^T
			result_g = transpose_matrix(function_g(common_term, SIZE_M), size_b);

			//(X*(g((w^T)*X)^T))/M
			term_left = divide_matrix_scalar(multiply_matrices(whitened_data, result_g, size_c), SIZE_M, size_w);

			//g((w^T)*X)
			result_g_prime = function_g_prime(common_term, SIZE_M);

			//(g((w^T)*X)*(column_m)*w)/M
			term_right = divide_matrix_scalar(multiply_matrix_scalar(current_w, multiply_matrices(result_g_prime, column_m, size_d)[0][0], size_w), SIZE_M, size_w);

			//w' = (X*(g((w^T)*X)^T))/M - (g((w^T)*X)*(column_m)*w)/M
			w_prime = substract_matrices(term_left, term_right, size_w);

			transpose_w_prime = transpose_matrix(w_prime, size_w);

			for(i = 0; i < p; i++){
				for(j = 0; j < SIZE_N; j++){
					temp_w[j][0] = w[j][i];
				}
				sum_vectors = add_matrices(sum_vectors, multiply_matrix_scalar(temp_w, multiply_matrices(transpose_w_prime, temp_w, size_e)[0][0], size_w), size_w);
			}

			//w" = w' - sum_vectors
			w_second = substract_matrices(w_prime, sum_vectors, size_w);

			//w = (w")/(||w"||)
			current_w = divide_matrix_scalar(w_second, frobenius_norm(w_second, size_w), size_w);
		}

		//Copy final weight vector to weight matrix
		for(i = 0; i < SIZE_N; i++){
			w[i][p] = current_w[i][0];
		}

	}

	result = multiply_matrices(w, whitened_data, size_f);

	return result;
}

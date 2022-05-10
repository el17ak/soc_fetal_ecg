/*
 * matrix_operators.c
 *
 *  Created on: 14 Apr 2022
 *      Author: Anna Irma Elizabeth KENNEDY
 */
#include "matrix_operators.h"

// Transpose (A x B) matrix into (B x A)
float **transpose_matrix(float **mat, unsigned short size[2]){
	float **result;
	int i, j;
	result = (float**)malloc(sizeof(float*) * size[1]);
	for(i = 0; i < size[1]; i++){
		result[i] = (float*)malloc(sizeof(float) * size[0]);
	}

	for(i = 0; i < size[0]; i++){
		for(j = 0; j < size[1]; j++){
			result[j][i] = mat[i][j];
			HPS_ResetWatchdog();
		}
	}
	return result;
}

// Add two matrices of size (A x B) element per element
float **add_matrices(float **mat_a, float **mat_b, unsigned short size[2]){
	float **result;
	int i, j;
	result = (float**)malloc(sizeof(float*) * size[0]);
	for(i = 0; i < size[0]; i++){
		result[i] = (float*)malloc(sizeof(float) * size[1]);
		for(j = 0; j < size[1]; j++){
			__asm{
				VADD.F32 result[i][j], mat_a[i][j], mat_b[i][j]
			}
			HPS_ResetWatchdog();
		}
	}
	return result;
}

// Subtract a matrix of size (A x B) by a matrix of the same size, element per element
float **substract_matrices(float **mat_a, float **mat_b, unsigned short size[2]){
	float **result;
	int i, j;
	result = (float**)malloc(sizeof(float**) * size[0]);
	for(i = 0; i < size[0]; i++){
		result[i] = (float*)malloc(sizeof(float*) * size[1]);
	}
	for(i = 0; i < size[0]; i++){
		for(j = 0; j < size[1]; j++){
			__asm{
				VSUB.F32 result[i][j], mat_a[i][j], mat_b[i][j]
			}
			HPS_ResetWatchdog();
		}
	}
	return result;
}

// Multiply a first matrix (A x B) by a second matrix (B x C)
float **multiply_matrices(float **mat_a, float **mat_b, unsigned short sizes[3]){
	float **result;
	float *res;
	int i, j, k;
		// FPU implementation (no FPGA)
		/*for(j = 0; j < sizes[2]; j++){
			for(k = 0; k < sizes[1]; k++){
				__asm{
					VMLA.F32 result[i][j], mat_a[i][k], mat_b[k][j]
				}
				HPS_ResetWatchdog();
			}
		}*/

//==========================================================================
// Extracted and derived from Cornell University ECE5670 Example code
// https://people.ece.cornell.edu/land/courses/ece5760/DE1_SOC/HPS_FPGA/DMA/BiDirectional/DMA_3.c
//==========================================================================

	#define H2F_AXI_MASTER_BASE   0xC0000000
	// main bus; scratch RAM
	#define FPGA_ONCHIP_BASE      0xC8000000
	#define FPGA_ONCHIP_SPAN      0x00010000
	// h2f bus
	// RAM FPGA port s2
	// main bus addess 0x0800_0000
	volatile unsigned int * sram_ptr = NULL ;
	void *sram_virtual_base;

	// ======================================
	// lw_bus; DMA  addresses
	#define HW_REGS_BASE        0xff200000
	#define HW_REGS_SPAN        0x00005000
	#define DMA					0xff200000

	// from datasheet
	// https://www.altera.com/en_US/pdfs/literature/ug/ug_embedded_ip.pdf
	#define DMA1_STATUS_OFFSET		0x00
	#define DMA1_READ_ADD_OFFSET	0x01
	#define DMA1_WRT_ADD_OFFSET		0x02
	#define DMA1_LENGTH_OFFSET		0x03
	#define DMA1_CNTL_OFFSET		0x06

	// second DMA is at offset 0x20 in Qsys
	// but these offsets are in WORDS
	// 0x20 bytes = 32 bytes = 8 words
	#define DMA2_STATUS_OFFSET		8+0
	#define DMA2_READ_ADD_OFFSET	8+1
	#define DMA2_WRT_ADD_OFFSET		8+2
	#define DMA2_LENGTH_OFFSET		8+3
	#define DMA2_CNTL_OFFSET		8+6

	#define LENGTH_ZERO_MASK		0x010
	// the h2f light weight bus base
	void *h2p_lw_virtual_base;
	// HPS_to_FPGA DMA address = 0
	volatile unsigned int * DMA_base_ptr = NULL ;

	// ======================================
	// HPS onchip memory base/span
	// 2^16 bytes at the top of memory
	#define HPS_ONCHIP_BASE		0xffff0000
	#define HPS_ONCHIP_SPAN		0x00010000
	// HPS onchip memory (HPS side!)
	volatile unsigned int * hps_onchip_ptr = NULL ;
	//volatile unsigned int * hps_onchip_ptr = NULL ;
	void *hps_onchip_virtual_base;

	result = (float**)malloc(sizeof(float**) * sizes[0]);
	for(i = 0; i < sizes[0]; i++){
		result[i] = (float*)malloc(sizeof(float*) * sizes[2]);

	// clear fpga memory (to make sure the DMA happens)
	for (i=0; i<16384; i++){
		*(sram_ptr+i) = 0 ;
	}

	// === put data in onchip ram
	// load a sequence
	memcpy((void*)hps_onchip_ptr, (const void*)sizes, 2*3);
	memcpy((void*)(hps_onchip_ptr + (3*2)), (const void*)mat_a, 4*16384);
	memcpy((void*)(hps_onchip_ptr + (3*2) + (4*16384)), (const void*)mat_b, 4*16384);

	// === DMA transfer HPS->FPGA
	// set up DMA
	// from https://www.altera.com/en_US/pdfs/literature/ug/ug_embedded_ip.pdf
	// section 25.4.3 Tables 224 and 225
	*(DMA_base_ptr + DMA1_STATUS_OFFSET) = 0;
	// read bus-master gets data from HPS addr=0xffff0000
	*(DMA_base_ptr + DMA1_READ_ADD_OFFSET) = 0xffff0000 ;
	// write bus_master for fpga sram is mapped to 0x08000000
	*(DMA_base_ptr + DMA1_WRT_ADD_OFFSET) = 0x08000000 ;
	// copy 4000 bytes for 1000 ints
	*(DMA_base_ptr + DMA1_LENGTH_OFFSET) = 16384*4 ;
	// set bit 2 for WORD transfer
	// set bit 3 to start DMA
	// set bit 7 to stop on byte-count
	// start the timer because DMA will start
	*(DMA_base_ptr + DMA1_CNTL_OFFSET) = 0x28;
	while ((*(DMA_base_ptr + DMA1_STATUS_OFFSET) & LENGTH_ZERO_MASK) == 0){}

	// clear onchip memory to make sure transfer back from FPGA works
	for (i=0; i<16384; i++){
		*(hps_onchip_ptr+i) = 0 ;
	}

	// === DMA FPGA memory -> HPS
	*(DMA_base_ptr + DMA2_STATUS_OFFSET) = 0;
	// read bus-master gets data from FPGA SRAM
	*(DMA_base_ptr + DMA2_READ_ADD_OFFSET) = 0x08000000 ;
	// write bus_master for HPS onchip
	*(DMA_base_ptr + DMA2_WRT_ADD_OFFSET) = 0xffff0000 ;
	// copy 4000 bytes for 1000 ints
	*(DMA_base_ptr + DMA2_LENGTH_OFFSET) = 16384*4 ;
	// set bit 2 for WORD transfer
	// set bit 3 to start DMA
	// set bit 7 to stop on byte-count
	*(DMA_base_ptr + DMA2_CNTL_OFFSET) = 0x28;
	while ((*(DMA_base_ptr + DMA2_STATUS_OFFSET) & LENGTH_ZERO_MASK) == 0){}

	// get the data back
	memcpy((void*)res, (const void*)hps_onchip_ptr, 4*16384);
	for(i = 0; i < sizes[0]; i++){
		for(j = 0; j < sizes[2]; j++){
			result[i][j] = res[(i*sizes[2])+j];
		}
	}
	}
	return result;
}


// Subtract the same one value from each element in a matrix
float **substract_matrix_scalar(float **mat, float scalar, unsigned short size[2]){
	float **result;
	int i, j;
	result = (float**)malloc(sizeof(float**) * size[0]);
		for(i = 0; i < size[0]; i++){
			result[i] = (float*)malloc(sizeof(float*) * size[1]);
			for(j = 0; j < size[1]; j++){
				__asm{
					VSUB.F32 result[i][j], mat[i][j], scalar
				}
				HPS_ResetWatchdog();
			}
		}
		return result;
}

// Multiply the same one value by each element in a matrix
float **multiply_matrix_scalar(float **mat, float scalar, unsigned short size[2]){
	float **result;
	int i, j;
	result = (float**)malloc(sizeof(float*) * size[0]);
	for(i = 0; i < size[0]; i++){
		result[i] = (float*)malloc(sizeof(float) * size[1]);
		for(j = 0; j < size[1]; j++){
			result[i][j] = mat[i][j] * scalar;
			__asm{
				VMUL.F32 result[i][j], mat[i][j], scalar
			}
			HPS_ResetWatchdog();
		}
	}
	return result;
}

// Divide each element in a matrix by the same one value
float **divide_matrix_scalar(float **mat, float scalar, unsigned short size[2]){
	float **result;
	int i, j;
	result = (float**)malloc(sizeof(float*) * size[0]);
	for(i = 0; i < size[0]; i++){
		result[i] = (float*)malloc(sizeof(float) * size[1]);
		for(j = 0; j < size[1]; j++){
			__asm{
				VDIV.F32 result[i][j], mat[i][j], scalar
			}
			HPS_ResetWatchdog();
		}
	}
	return result;
}

// Compute the Frobenius norm, i.e., square root of sum of all elements, squared
float frobenius_norm(float **mat, unsigned short size[2]){
	float result;
	float sum = 0;
	int i, j;
	for(i = 0; i < size[0]; i++){
		for(j = 0; j < size[1]; j++){
			__asm{
				VMLA.F32 sum, mat[i][j], mat[i][j]
			}
			HPS_ResetWatchdog();
		}
	}
	__asm{
		VSQRT.F32 result, sum
	}
	printf("Res: %f\n", result);
	return result;
}

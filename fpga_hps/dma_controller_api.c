/*
 * dma_controller_api.c

 *
 *  Created on: 3 May 2022
 *    Author: robertofem
 */

//API for the Qsys DMA Controller
#include "dma_controller_api.h"

//-----------------Generic functions--------------------//
//(Addresses are multiplied by 4 because the peripheral has 32-bit (4byte) regs
uint32_t fpga_dma_read_reg(int* addr, uint32_t reg){
	return (uint32_t)* (addr + 4*reg);
}


void fpga_dma_write_reg(int* addr, uint32_t reg, uint32_t val){
	*((uint32_t*) (addr + 4*reg)) = val;
}


uint32_t fpga_dma_read_bit(int* addr, uint32_t reg, uint32_t bit){
	return (bit & fpga_dma_read_reg(addr, reg));
}


void fpga_dma_write_bit(int* addr, uint32_t reg, uint32_t bit, uint32_t val){
	uint32_t old = fpga_dma_read_reg(addr, reg);
	if(val == 0){
	  fpga_dma_write_reg(addr, reg, (old & (~bit)));
	}
	else if(val == 1){
    fpga_dma_write_reg(addr, reg, (old | bit));
  }
  return;
}


//------------Some specific functions-------------------//
void fpga_dma_init(int* addr, uint32_t control_reg_val){
  fpga_dma_write_reg( addr, FPGA_DMA_CONTROL, control_reg_val);
}


void fpga_dma_config_transfer(int* addr, void* src, void* dst, uint32_t size){
	int counter, j;
	fpga_dma_write_bit( addr, FPGA_DMA_CONTROL, FPGA_DMA_GO, 0);
	fpga_dma_write_bit( addr, //clean the done bit
					  FPGA_DMA_STATUS,
					  FPGA_DMA_DONE,
					  0);
	fpga_dma_write_reg( addr,   //set source address
					  FPGA_DMA_READADDRESS,
					  (uint32_t) src);
	fpga_dma_write_reg( addr,  //set destiny address
					  FPGA_DMA_WRITEADDRESS,
					  (uint32_t) dst);
	fpga_dma_write_reg( addr, //set transfer size
					  FPGA_DMA_LENGTH,
					  (uint32_t) size);
	//Wait a small time. Needed for the read from HPS to work.
	counter = 0;
	for(j = 0; j < 10; j++) counter++;
}


void fpga_dma_start_transfer(int* addr){
  fpga_dma_write_bit( addr,
                      FPGA_DMA_CONTROL,
                      FPGA_DMA_GO,
                      1);//start transfer
}


//this function returns a 1 if the DMA transfer is finished, 0 otherwise
uint32_t fpga_dma_transfer_done(int* addr){
	return fpga_dma_read_bit(addr, FPGA_DMA_STATUS, FPGA_DMA_DONE);
}


//alligned allocation to the transfer size is needed for reading HPS from FPGA

void* align_malloc (size_t size, int** unaligned_addr){
	char* Buffer_8 = (char*) malloc(size*2);
	uint8_t* Buffer_8_ptr;
	int k;
	*unaligned_addr = (void*) Buffer_8;
	Buffer_8_ptr = (uint8_t*)Buffer_8;
	for(k=0; k<(size); k++){
		if ((((unsigned long int)Buffer_8_ptr) % ((unsigned long int)size))==0){
			return (void*)Buffer_8_ptr;
		}
		else{
			Buffer_8_ptr++;
		}
	}
	return 0;
}

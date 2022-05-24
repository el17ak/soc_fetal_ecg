/*
 * dma_controller_api.h

 *
 *  Created on: 3 May 2022
 *      Author: el17a
 */

#ifndef FPGA_HPS_DMA_CONTROLLER_API_H_
#define FPGA_HPS_DMA_CONTROLLER_API_H_

//API for the Qsys DMA Controller

#include <inttypes.h>
#include <stdlib.h>

//REGISTER MAP
#define FPGA_DMA_STATUS         0
#define FPGA_DMA_READADDRESS    1
#define FPGA_DMA_WRITEADDRESS   2
#define FPGA_DMA_LENGTH         3
//RESERVED                      4
//RESERVED                      5
#define FPGA_DMA_CONTROL        6
//RESERVED                      7

//MACROS to more easily read the control status register bits
#define FPGA_DMA_DONE                    0x01 //DONE
#define FPGA_DMA_BUSY                    0x02 //BUSY
#define FPGA_DMA_REOP                    0x04 //REOP
#define FPGA_DMA_WEOP                    0x08 //WEOP
#define FPGA_DMA_LEN                     0x10 //LEN

//MACROS to more easily program the control register
#define FPGA_DMA_BYTE_TRANSFERS          0x0001 //BYTE
#define FPGA_DMA_HALFWORD_TRANSFERS      0x0002 //HW
#define FPGA_DMA_WORD_TRANSFERS          0x0004 //WORD
#define FPGA_DMA_GO                      0x0008 //GO
#define FPGA_DMA_INTERRUPT_ENABLE        0x0010 //I_EN
#define FPGA_DMA_END_RD_END_OF_PACKET    0x0020 //REEN
#define FPGA_DMA_END_WR_END_OF_PACKET    0x0040 //WEEN
#define FPGA_DMA_END_WHEN_LENGHT_ZERO    0x0080 //LEEN
#define FPGA_DMA_READ_CONSTANT_ADDR      0x0100 //RCON
#define FPGA_DMA_WRITE_CONSTANT_ADDR     0x0200 //WCON
#define FPGA_DMA_DOUBLEWORD_TRANSFERS    0x0400 //DOUBLEWORD
#define FPGA_DMA_QUADWORD_TRANSFERS      0x0800 //QUADWORD
#define FPGA_DMA_SOFTWARE_RESET          0x1000 //SOFTWARE_RESET

//-----------------Generic functions--------------------//
unsigned int fpga_dma_read_reg(int* addr, uint32_t reg);
void fpga_dma_write_reg(int* addr, uint32_t reg, uint32_t val);
unsigned int fpga_dma_read_bit(int* addr, uint32_t reg, uint32_t bit);
void fpga_dma_write_bit(int* addr, uint32_t reg, uint32_t bit, uint32_t val);

//------------Some specific functions-------------------//
void fpga_dma_init();
void fpga_dma_config_transfer(int* addr, void* src, void* dst, unsigned int size);
void fpga_dma_start_transfer(int* addr);
uint32_t fpga_dma_transfer_done(int* addr);
void* align_malloc (size_t size, int** unaligned_addr);

#endif /* FPGA_HPS_DMA_CONTROLLER_API_H_ */

#include <stdio.h>
#include "file_handler.h"
#include "whiten.h"
#include "fastica.h"
#include "display.h"
#include "../Drivers/HPS_Watchdog/HPS_Watchdog.h"
#include "../Drivers/DE1SoC_LT24/DE1SoC_LT24.h"
#include "../Drivers/HPS_IRQ/HPS_IRQ.h"
#include "sub01_fecg1.h"
#include <time.h>

// ======================================
#define H2F_AXI_MASTER_BASE   0xC0000000
// main bus; scratch RAM
#define FPGA_ONCHIP_BASE      0xC8000000
#define FPGA_ONCHIP_SPAN      0x00010000
// h2f bus
// RAM FPGA port s2
unsigned int *sram_base;

// ======================================
// lw_bus; DMA  addresses
#define HW_REGS_BASE        0xFF200000
#define HW_REGS_SPAN        0x00005000
#define DMA					0xFF200000

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
unsigned int *h2p_lw_base;

// ======================================
// HPS onchip memory base/span
// 2^16 bytes at the top of memory
#define HPS_ONCHIP_BASE		0xffff0000
#define HPS_ONCHIP_SPAN		0x00010000
// HPS onchip memory (HPS side!)
unsigned int *hps_onchip_base;
#define WAIT {}

// Bluetooth disconnection Interrupt Service Routine
void bluetoothISR(HPSIRQSource interruptID, bool isInit, void* initParams) {
    if (!isInit) {
    	//Restart advertising until connection is secure
    }
    //Reset watchdog.
    HPS_ResetWatchdog();
}

int main(void){
	unsigned short SIZE_N = 8;
	unsigned short SIZE_M = 512;
	unsigned short SIZE_C = 2;
	int **data;
	int in_data[4096];

	float **whitened_data;
	float **transformed_data; // (C x M)
	float **show_data;

	int i;
	time_t mytime = time(NULL);
	char *t;

	//Allocate memory for input data
	data = (int**)malloc(sizeof(int*) * SIZE_N);
	for(i = 0; i < SIZE_N; i++) data[i] = (int*)malloc(sizeof(int) * SIZE_M);

	show_data = (float**)malloc(sizeof(float*) * SIZE_N);
	for(i = 0; i < SIZE_N; i++) show_data[i] = (float*)malloc(sizeof(float) * SIZE_M);

	//Initialise LCD
	LT24_initialise(0xFF200060, 0xFF200080);
	HPS_ResetWatchdog();

	//Initialise IRQs
	HPS_IRQ_initialise(NULL);
	HPS_ResetWatchdog();

	// Configure Push Buttons to interrupt on press
	// Register interrupt handler for keys
	HPS_IRQ_registerHandler(IRQ_FPGA18, bluetoothISR);
	HPS_ResetWatchdog();

	//============================================
	// get virtual addr that maps to physical
	// for light weight bus
	// DMA status register
	h2p_lw_base = (unsigned int *)0x00000020;

	//  RAM FPGA parameter addr
	sram_base = (unsigned int *)0x08000000;

	// HPS onchip ram
	hps_onchip_base = (unsigned int *)0xFFFF0000;

	while(1){
		//How to retrieve data from FPGA input?

		//============================================
		int j, k;

		// clear onchip memory to make sure transfer back from FPGA works
		for (i=0; i<512; i++){
			*(hps_onchip_base+i) = 0 ;
		}

		// === DMA FPGA memory -> HPS
		*(h2p_lw_base + DMA2_STATUS_OFFSET) = 0;
		// read bus-master gets data from FPGA SRAM
		*(h2p_lw_base + DMA2_READ_ADD_OFFSET) = 0x08000000 ;
		// write bus_master for HPS onchip
		*(h2p_lw_base + DMA2_WRT_ADD_OFFSET) = 0xffff0000 ;
		// copy 4000 bytes for 1000 ints
		*(h2p_lw_base + DMA2_LENGTH_OFFSET) = 512*8*4 ;
		// set bit 2 for WORD transfer
		// set bit 3 to start DMA
		// set bit 7 to stop on byte-count
		// start the timer because DMA will start
		t = ctime(&mytime);
		*(h2p_lw_base + DMA2_CNTL_OFFSET) = 0x008C;
		while ((*(h2p_lw_base + DMA2_STATUS_OFFSET) & LENGTH_ZERO_MASK) == 0) WAIT;
		// finish timing the transfer
		t = ctime(&mytime);


		//data = read_matrix(&rows, &cols, "sub01_fecg1.txt");
		for(i = 0; i < SIZE_N; i++){
			for(j = 0; j < SIZE_M; j++){
				//data[i][j] = in_data[(i*SIZE_M)+j];
				data[i][j] = (0x8000&(sub01_fecg1[(j*8)+i]) ? (int)(0x7FFF&(sub01_fecg1[(j*8)+i]))-0x8000 : (sub01_fecg1[(j*8)+i]));
			}
		}

		HPS_ResetWatchdog();


		/*for(j = 0; j < SIZE_N; j++){
			for(k = 0; k < SIZE_M; k++){
				show_data[j][k] = (float)data[j][k];
				printf("%f ", show_data[j][k]);
			}
			printf("\n");
		}*/

		printf("Whitening\n");
		whitened_data = whiten(data, SIZE_N, SIZE_M);
		printf("Transforming\n");
		transformed_data = transform(whitened_data, SIZE_N, SIZE_M, SIZE_C);


		// Display a single IC of the signal
		display_set(transformed_data, SIZE_M);

		// DMA write local-to-fpga
		// clear fpga memory (to make sure the DMA happens)
		for (i=0; i<512; i++){
			*(sram_base+i) = 0 ;
		}
		// === put data in onchip ram
		// start the timer
		t = ctime(&mytime);
		// generate a sequence
		memcpy((void*)hps_onchip_base, (const void*)in_data, 4*512);

		// finish timing the write
		t = ctime(&mytime);

		// === DMA transfer HPS->FPGA
		// set up DMA
		// from https://www.altera.com/en_US/pdfs/literature/ug/ug_embedded_ip.pdf
		// section 25.4.3 Tables 224 and 225
		*(h2p_lw_base + DMA1_STATUS_OFFSET) = 0;
		// read bus-master gets data from HPS addr=0xffff0000
		*(h2p_lw_base + DMA1_READ_ADD_OFFSET) = 0xffff0000 ;
		// write bus_master for fpga sram is mapped to 0x08000000
		*(h2p_lw_base + DMA1_WRT_ADD_OFFSET) = 0x08000000 ;
		// copy 4000 bytes for 1000 ints
		*(h2p_lw_base + DMA1_LENGTH_OFFSET) = 512*4 ;
		// set bit 2 for WORD transfer
		// set bit 3 to start DMA
		// set bit 7 to stop on byte-count
		// start the timer because DMA will start
		t = ctime(&mytime);
		*(h2p_lw_base + DMA1_CNTL_OFFSET) = 0x008C;
		while ((*(h2p_lw_base + DMA1_STATUS_OFFSET) & LENGTH_ZERO_MASK) == 0) WAIT;
		// finish timing the transfer
		t = ctime(&mytime);
	}

	//Free allocated memory for input data
	for(i = 0; i < SIZE_N; i++) free(data[i]);
	free(data);
}

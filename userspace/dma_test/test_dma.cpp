#include <fcntl.h>
#include <unistd.h>
#include <sys/mman.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <stdint.h>
#include <iostream>

#include <time.h>
#include <chrono>

#include <math.h>

#include "axi_dma_controller.h"
#include "reserved_mem.hpp"

#define DEVICE_FILENAME "/dev/reservedmemLKM"
#define DATA_LENGTH 64 // Feel free to change this value to test different lengths
#define RESERVED_MEM_START 0x70000000
#define I_OFFSET 0 // Offset in the reserved memory of the input data (in bytes) 
#define O_OFFSET 64 // Offset in the reserved memory of the output data (in bytes) 
#define I_OFFSET_WORDS (I_OFFSET / 4)
#define O_OFFSET_WORDS (O_OFFSET / 4)

#define UIO_DMA_N 0

std::chrono::_V2::system_clock::time_point t1;
void start_timer()
{
	t1 = std::chrono::high_resolution_clock::now();
}
double stop_timer()
{
	auto t2 = std::chrono::high_resolution_clock::now();
	auto ms_int = std::chrono::duration_cast<std::chrono::milliseconds>(t2 - t1);
	std::chrono::duration<double, std::milli> ms_double = t2 - t1;
	return ms_double.count();
}

void print_mem(void *virtual_address, int byte_count)
{
	char *data_ptr = (char *)virtual_address;

	for (int i = 0; i < byte_count; i++)
	{
		printf("%02X", data_ptr[i]);

		// Print a space every 4 bytes (0 indexed)
		if (i % 4 == 3)
		{
			printf(" ");
		}
	}

	printf("\n");
}

/**
 * Test of the DMA
 * 
 * Do not forget to add the kernel module to be able to access the reserved memory:
 * ```
 * sudo insmod path/to/reservedmemLKM.ko
 * ```
 * 
 * Do not forget to make the file (/dev/reservedmemLKM) readable and writable to the current user:
 * ```
 * sudo chown <user> /dev/reservedmenLKM
 * ```
 */
int main()
{
	double total_t = 0;
	double tmp = 0;
	printf("Running DMA transfer test application with specified memory.\n\n");

	Reserved_Mem pmem;
	AXIDMAController dma(UIO_DMA_N, 0x10000);

	uint32_t *i_buff = (uint32_t *)calloc(DATA_LENGTH, 1);
	if (i_buff == NULL)
	{
		printf("Could not allocate input buffer\n");
		return -1;
	}

	for (int i = 0; i < DATA_LENGTH / sizeof(uint32_t); i++) {
        i_buff[i] = i;
    }

	uint32_t *o_buff = (uint32_t *)calloc(DATA_LENGTH , 1);
	if (o_buff == NULL)
	{
		printf("Could not allocate output buffer\n");
		return -1;
	}

	printf("Memory at the source and destination addresses in the DDR before the transfert:\n");
	print_mem(i_buff, DATA_LENGTH);
	print_mem(o_buff, DATA_LENGTH);

	// Transfering the content of u_buff to the reserved memory
	tmp = 0;
	start_timer();
	pmem.transfer(i_buff, I_OFFSET, DATA_LENGTH); 
	total_t += stop_timer();
	std::cout << 
        "\nData transfered to reserved memory: " <<
        total_t << "ms [" << (float)DATA_LENGTH / 1000000. << "MB]" <<
        std::endl;

	start_timer();

    // Setting up the DMA for the transfert

	dma.MM2SReset();
	dma.S2MMReset();

	dma.MM2SHalt();
	dma.S2MMHalt();

	dma.MM2SInterruptEnable();
	dma.S2MMInterruptEnable();

	dma.MM2SSetSourceAddress(RESERVED_MEM_START + I_OFFSET);
	dma.S2MMSetDestinationAddress(RESERVED_MEM_START + O_OFFSET);

	dma.MM2SStart();
	dma.S2MMStart();

	dma.MM2SSetLength(DATA_LENGTH); // /!\ Will only work up to 2^23
	dma.S2MMSetLength(DATA_LENGTH);
	
	tmp = stop_timer();
	total_t += tmp;
	std::cout <<
        "\nDMA setup done, transfer begun: " <<
        tmp << "ms [" << (float)DATA_LENGTH / 1000000. <<
        "MB]\n" <<
        std::endl;

	start_timer();
	printf("...Waiting for MM2S synchronization...\n");
	while (!dma.MM2SIsSynced()) {}

	tmp = stop_timer();
	total_t += tmp;
	std::cout <<
        "\nData transfered to transfered by DMA: " <<
        tmp << "ms [" <<
        (float)DATA_LENGTH / 1000000. << "MB]\n" <<
        std::endl;

	printf("Waiting for S2MM sychronization...\n\n");
	while(!dma.S2MMIsSynced()) {}

	printf("DMA TRANSFERT ENDED!\n\n");

    // Making sure that the transfert has worked
	pmem.gather(i_buff, I_OFFSET_WORDS, DATA_LENGTH);
	pmem.gather(o_buff, O_OFFSET_WORDS, DATA_LENGTH);
	printf(
        "Memory at the source and destination addresses in the reserved memory after the whole"
        " transfert:\n"
    );
	print_mem(i_buff, DATA_LENGTH);
	print_mem(o_buff, DATA_LENGTH);

	std::cout <<
        "\nTotal duration of transfer: " <<
        total_t << "ms [" <<
        (float)DATA_LENGTH / 1000000. << "MB]" <<
        std::endl;

	free(i_buff);
	free(o_buff);

	return 0;
}

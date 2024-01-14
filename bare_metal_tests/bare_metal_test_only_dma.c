#include "xaxidma.h"
#include "xparameters.h"
#include "xdebug.h"

#define DMA_DEV_ID		XPAR_AXIDMA_0_DEVICE_ID

#ifdef XPAR_AXI_7SDDR_0_S_AXI_BASEADDR
#define DDR_BASE_ADDR		XPAR_AXI_7SDDR_0_S_AXI_BASEADDR
#elif defined (XPAR_MIG7SERIES_0_BASEADDR)
#define DDR_BASE_ADDR	XPAR_MIG7SERIES_0_BASEADDR
#elif defined (XPAR_MIG_0_BASEADDR)
#define DDR_BASE_ADDR	XPAR_MIG_0_BASEADDR
#elif defined (XPAR_PSU_DDR_0_S_AXI_BASEADDR)
#define DDR_BASE_ADDR	XPAR_PSU_DDR_0_S_AXI_BASEADDR
#endif

#ifndef DDR_BASE_ADDR
#warning COULD NOT FIND BASE DDR ADDRESS IN XPARAMETERS.H, SO DEFAULT HAS BEEN SET TO 0x01000000
#define MEM_BASE_ADDR		0x01000000
#else
#define MEM_BASE_ADDR		(DDR_BASE_ADDR + 0x1000000)
#endif

#define TX_BUFFER_BASE		(MEM_BASE_ADDR + 0x00100000)
#define RX_BUFFER_BASE		(MEM_BASE_ADDR + 0x00300000)
#define RX_BUFFER_HIGH		(MEM_BASE_ADDR + 0x004FFFFF)

#define TEST_DATA_LENGTH	64

int run_test(u16 DMA_device_id);

XAxiDma AXI_dma;

int main()
{
	xil_printf("\r\nStarting test\r\n");
	int status = run_test(DMA_DEV_ID);
	if (status != XST_SUCCESS) {
		xil_printf("Test failed\r\n");
		return XST_FAILURE;
	}
	xil_printf("Test ran successfully\r\n");
	return XST_SUCCESS;
}

int run_test(u16 DMA_device_id)
{
	XAxiDma_Config *AXI_cfg_ptr;
    int status;

	u8 *Tx_buffer_ptr = (u8 *)TX_BUFFER_BASE;
	u8 *rx_buffer_ptr = (u8 *)RX_BUFFER_BASE;

	xil_printf("Initializing DMA");
	AXI_cfg_ptr = XAxiDma_LookupConfig(DMA_device_id);
	if (!AXI_cfg_ptr) {
		xil_printf("DMA initialization failed\r\n");
		return XST_FAILURE;
	}
	status = XAxiDma_CfgInitialize(&AXI_dma, AXI_cfg_ptr);
	if (status != XST_SUCCESS) {
	    xil_printf("DMA initialization failed\r\n");
		return XST_FAILURE;
	}
	xil_printf("DMA intialized successfully\r\n");

	XAxiDma_IntrDisable(&AXI_dma, XAXIDMA_IRQ_ALL_MASK, XAXIDMA_DEVICE_TO_DMA);
	XAxiDma_IntrDisable(&AXI_dma, XAXIDMA_IRQ_ALL_MASK, XAXIDMA_DMA_TO_DEVICE);

    xil_printf("Filling tranfer buffer\r\n");
	for(int i = 0; i < TEST_DATA_LENGTH; i ++) {
		Tx_buffer_ptr[i] = i;
	}
    xil_printf("Successfully filled tranfer buffer\r\n");

    Xil_DCacheFlushRange((UINTPTR)Tx_buffer_ptr, TEST_DATA_LENGTH);
	Xil_DCacheFlushRange((UINTPTR)rx_buffer_ptr, TEST_DATA_LENGTH);

    xil_printf("DMA to device transfer started\r\n");
    status = XAxiDma_SimpleTransfer(
        &AXI_dma, (UINTPTR) Tx_buffer_ptr, TEST_DATA_LENGTH, XAXIDMA_DMA_TO_DEVICE
    );
    if (status != XST_SUCCESS) {
        xil_printf("DMA to device transfer failed\r\n");
        return XST_FAILURE;
    }
    while (XAxiDma_Busy(&AXI_dma,XAXIDMA_DMA_TO_DEVICE)) {}
    xil_printf("DMA to device transfer successful\r\n");

    xil_printf("Device to DMA transfer started\r\n");
    status = XAxiDma_SimpleTransfer(
        &AXI_dma, (UINTPTR) rx_buffer_ptr, TEST_DATA_LENGTH, XAXIDMA_DEVICE_TO_DMA
    );
    if (status != XST_SUCCESS) {
        xil_printf("Device to DMA transfer failed\r\n");
        return XST_FAILURE;
    }
    while (XAxiDma_Busy(&AXI_dma,XAXIDMA_DEVICE_TO_DMA)) {}
    xil_printf("Device to DMA transfer successful\r\n");

    xil_printf("Checking received data\r\n");
	for(int i = 0; i < TEST_DATA_LENGTH; i ++) {
        u8 received = rx_buffer_ptr[i];
        u8 expected = Tx_buffer_ptr[i];
		if (received != expected) {
            xil_printf("Error at index %d: %u != %u\r\n", received, expected);
            return XST_FAILURE;
        } else {
			xil_printf("%u / %u\r\n", received, expected);
		}
	}
    xil_printf("Received data matches what is expected, test passed !\r\n");

	return XST_SUCCESS;
}

// ==============================================================
// Vitis HLS - High-Level Synthesis from C, C++ and OpenCL v2020.2 (64-bit)
// Copyright 1986-2020 Xilinx, Inc. All Rights Reserved.
// ==============================================================
// AXI_CPU
// 0x0000 : Control signals
//          bit 0  - ap_start (Read/Write/COH)
//          bit 1  - ap_done (Read/COR)
//          bit 2  - ap_idle (Read)
//          bit 3  - ap_ready (Read)
//          bit 7  - auto_restart (Read/Write)
//          others - reserved
// 0x0004 : Global Interrupt Enable Register
//          bit 0  - Global Interrupt Enable (Read/Write)
//          others - reserved
// 0x0008 : IP Interrupt Enable Register (Read/Write)
//          bit 0  - enable ap_done interrupt (Read/Write)
//          bit 1  - enable ap_ready interrupt (Read/Write)
//          others - reserved
// 0x000c : IP Interrupt Status Register (Read/TOW)
//          bit 0  - ap_done (COR/TOW)
//          bit 1  - ap_ready (COR/TOW)
//          others - reserved
// 0x0010 : Data signal of ap_return
//          bit 31~0 - ap_return[31:0] (Read)
// 0x1000 ~
// 0x1fff : Memory 'input_img' (900 * 32b)
//          Word n : bit [31:0] - input_img[n]
// (SC = Self Clear, COR = Clear on Read, TOW = Toggle on Write, COH = Clear on Handshake)

#define XNN_INFERENCE_AXI_CPU_ADDR_AP_CTRL        0x0000
#define XNN_INFERENCE_AXI_CPU_ADDR_GIE            0x0004
#define XNN_INFERENCE_AXI_CPU_ADDR_IER            0x0008
#define XNN_INFERENCE_AXI_CPU_ADDR_ISR            0x000c
#define XNN_INFERENCE_AXI_CPU_ADDR_AP_RETURN      0x0010
#define XNN_INFERENCE_AXI_CPU_BITS_AP_RETURN      32
#define XNN_INFERENCE_AXI_CPU_ADDR_INPUT_IMG_BASE 0x1000
#define XNN_INFERENCE_AXI_CPU_ADDR_INPUT_IMG_HIGH 0x1fff
#define XNN_INFERENCE_AXI_CPU_WIDTH_INPUT_IMG     32
#define XNN_INFERENCE_AXI_CPU_DEPTH_INPUT_IMG     900


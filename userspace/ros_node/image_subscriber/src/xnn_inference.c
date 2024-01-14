// ==============================================================
// Vitis HLS - High-Level Synthesis from C, C++ and OpenCL v2020.2 (64-bit)
// Copyright 1986-2020 Xilinx, Inc. All Rights Reserved.
// ==============================================================
/***************************** Include Files *********************************/
#include "xnn_inference.h"

/************************** Function Implementation *************************/
#ifndef __linux__
int XNn_inference_CfgInitialize(XNn_inference *InstancePtr, XNn_inference_Config *ConfigPtr) {
    Xil_AssertNonvoid(InstancePtr != NULL);
    Xil_AssertNonvoid(ConfigPtr != NULL);

    InstancePtr->Axi_cpu_BaseAddress = ConfigPtr->Axi_cpu_BaseAddress;
    InstancePtr->IsReady = XIL_COMPONENT_IS_READY;

    return XST_SUCCESS;
}
#endif

void XNn_inference_Start(XNn_inference *InstancePtr) {
    u32 Data;

    Xil_AssertVoid(InstancePtr != NULL);
    Xil_AssertVoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    Data = XNn_inference_ReadReg(InstancePtr->Axi_cpu_BaseAddress, XNN_INFERENCE_AXI_CPU_ADDR_AP_CTRL) & 0x80;
    XNn_inference_WriteReg(InstancePtr->Axi_cpu_BaseAddress, XNN_INFERENCE_AXI_CPU_ADDR_AP_CTRL, Data | 0x01);
}

u32 XNn_inference_IsDone(XNn_inference *InstancePtr) {
    u32 Data;

    Xil_AssertNonvoid(InstancePtr != NULL);
    Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    Data = XNn_inference_ReadReg(InstancePtr->Axi_cpu_BaseAddress, XNN_INFERENCE_AXI_CPU_ADDR_AP_CTRL);
    return (Data >> 1) & 0x1;
}

u32 XNn_inference_IsIdle(XNn_inference *InstancePtr) {
    u32 Data;

    Xil_AssertNonvoid(InstancePtr != NULL);
    Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    Data = XNn_inference_ReadReg(InstancePtr->Axi_cpu_BaseAddress, XNN_INFERENCE_AXI_CPU_ADDR_AP_CTRL);
    return (Data >> 2) & 0x1;
}

u32 XNn_inference_IsReady(XNn_inference *InstancePtr) {
    u32 Data;

    Xil_AssertNonvoid(InstancePtr != NULL);
    Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    Data = XNn_inference_ReadReg(InstancePtr->Axi_cpu_BaseAddress, XNN_INFERENCE_AXI_CPU_ADDR_AP_CTRL);
    // check ap_start to see if the pcore is ready for next input
    return !(Data & 0x1);
}

void XNn_inference_EnableAutoRestart(XNn_inference *InstancePtr) {
    Xil_AssertVoid(InstancePtr != NULL);
    Xil_AssertVoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    XNn_inference_WriteReg(InstancePtr->Axi_cpu_BaseAddress, XNN_INFERENCE_AXI_CPU_ADDR_AP_CTRL, 0x80);
}

void XNn_inference_DisableAutoRestart(XNn_inference *InstancePtr) {
    Xil_AssertVoid(InstancePtr != NULL);
    Xil_AssertVoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    XNn_inference_WriteReg(InstancePtr->Axi_cpu_BaseAddress, XNN_INFERENCE_AXI_CPU_ADDR_AP_CTRL, 0);
}

u32 XNn_inference_Get_return(XNn_inference *InstancePtr) {
    u32 Data;

    Xil_AssertNonvoid(InstancePtr != NULL);
    Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    Data = XNn_inference_ReadReg(InstancePtr->Axi_cpu_BaseAddress, XNN_INFERENCE_AXI_CPU_ADDR_AP_RETURN);
    return Data;
}
u32 XNn_inference_Get_input_img_BaseAddress(XNn_inference *InstancePtr) {
    Xil_AssertNonvoid(InstancePtr != NULL);
    Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    return (InstancePtr->Axi_cpu_BaseAddress + XNN_INFERENCE_AXI_CPU_ADDR_INPUT_IMG_BASE);
}

u32 XNn_inference_Get_input_img_HighAddress(XNn_inference *InstancePtr) {
    Xil_AssertNonvoid(InstancePtr != NULL);
    Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    return (InstancePtr->Axi_cpu_BaseAddress + XNN_INFERENCE_AXI_CPU_ADDR_INPUT_IMG_HIGH);
}

u32 XNn_inference_Get_input_img_TotalBytes(XNn_inference *InstancePtr) {
    Xil_AssertNonvoid(InstancePtr != NULL);
    Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    return (XNN_INFERENCE_AXI_CPU_ADDR_INPUT_IMG_HIGH - XNN_INFERENCE_AXI_CPU_ADDR_INPUT_IMG_BASE + 1);
}

u32 XNn_inference_Get_input_img_BitWidth(XNn_inference *InstancePtr) {
    Xil_AssertNonvoid(InstancePtr != NULL);
    Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    return XNN_INFERENCE_AXI_CPU_WIDTH_INPUT_IMG;
}

u32 XNn_inference_Get_input_img_Depth(XNn_inference *InstancePtr) {
    Xil_AssertNonvoid(InstancePtr != NULL);
    Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    return XNN_INFERENCE_AXI_CPU_DEPTH_INPUT_IMG;
}

u32 XNn_inference_Write_input_img_Words(XNn_inference *InstancePtr, int offset, word_type *data, int length) {
    Xil_AssertNonvoid(InstancePtr != NULL);
    Xil_AssertNonvoid(InstancePtr -> IsReady == XIL_COMPONENT_IS_READY);

    int i;

    if ((offset + length)*4 > (XNN_INFERENCE_AXI_CPU_ADDR_INPUT_IMG_HIGH - XNN_INFERENCE_AXI_CPU_ADDR_INPUT_IMG_BASE + 1))
        return 0;

    for (i = 0; i < length; i++) {
        *(int *)(InstancePtr->Axi_cpu_BaseAddress + XNN_INFERENCE_AXI_CPU_ADDR_INPUT_IMG_BASE + (offset + i)*4) = *(data + i);
    }
    return length;
}

u32 XNn_inference_Read_input_img_Words(XNn_inference *InstancePtr, int offset, word_type *data, int length) {
    Xil_AssertNonvoid(InstancePtr != NULL);
    Xil_AssertNonvoid(InstancePtr -> IsReady == XIL_COMPONENT_IS_READY);

    int i;

    if ((offset + length)*4 > (XNN_INFERENCE_AXI_CPU_ADDR_INPUT_IMG_HIGH - XNN_INFERENCE_AXI_CPU_ADDR_INPUT_IMG_BASE + 1))
        return 0;

    for (i = 0; i < length; i++) {
        *(data + i) = *(int *)(InstancePtr->Axi_cpu_BaseAddress + XNN_INFERENCE_AXI_CPU_ADDR_INPUT_IMG_BASE + (offset + i)*4);
    }
    return length;
}

u32 XNn_inference_Write_input_img_Bytes(XNn_inference *InstancePtr, int offset, char *data, int length) {
    Xil_AssertNonvoid(InstancePtr != NULL);
    Xil_AssertNonvoid(InstancePtr -> IsReady == XIL_COMPONENT_IS_READY);

    int i;

    if ((offset + length) > (XNN_INFERENCE_AXI_CPU_ADDR_INPUT_IMG_HIGH - XNN_INFERENCE_AXI_CPU_ADDR_INPUT_IMG_BASE + 1))
        return 0;

    for (i = 0; i < length; i++) {
        *(char *)(InstancePtr->Axi_cpu_BaseAddress + XNN_INFERENCE_AXI_CPU_ADDR_INPUT_IMG_BASE + offset + i) = *(data + i);
    }
    return length;
}

u32 XNn_inference_Read_input_img_Bytes(XNn_inference *InstancePtr, int offset, char *data, int length) {
    Xil_AssertNonvoid(InstancePtr != NULL);
    Xil_AssertNonvoid(InstancePtr -> IsReady == XIL_COMPONENT_IS_READY);

    int i;

    if ((offset + length) > (XNN_INFERENCE_AXI_CPU_ADDR_INPUT_IMG_HIGH - XNN_INFERENCE_AXI_CPU_ADDR_INPUT_IMG_BASE + 1))
        return 0;

    for (i = 0; i < length; i++) {
        *(data + i) = *(char *)(InstancePtr->Axi_cpu_BaseAddress + XNN_INFERENCE_AXI_CPU_ADDR_INPUT_IMG_BASE + offset + i);
    }
    return length;
}

void XNn_inference_InterruptGlobalEnable(XNn_inference *InstancePtr) {
    Xil_AssertVoid(InstancePtr != NULL);
    Xil_AssertVoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    XNn_inference_WriteReg(InstancePtr->Axi_cpu_BaseAddress, XNN_INFERENCE_AXI_CPU_ADDR_GIE, 1);
}

void XNn_inference_InterruptGlobalDisable(XNn_inference *InstancePtr) {
    Xil_AssertVoid(InstancePtr != NULL);
    Xil_AssertVoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    XNn_inference_WriteReg(InstancePtr->Axi_cpu_BaseAddress, XNN_INFERENCE_AXI_CPU_ADDR_GIE, 0);
}

void XNn_inference_InterruptEnable(XNn_inference *InstancePtr, u32 Mask) {
    u32 Register;

    Xil_AssertVoid(InstancePtr != NULL);
    Xil_AssertVoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    Register =  XNn_inference_ReadReg(InstancePtr->Axi_cpu_BaseAddress, XNN_INFERENCE_AXI_CPU_ADDR_IER);
    XNn_inference_WriteReg(InstancePtr->Axi_cpu_BaseAddress, XNN_INFERENCE_AXI_CPU_ADDR_IER, Register | Mask);
}

void XNn_inference_InterruptDisable(XNn_inference *InstancePtr, u32 Mask) {
    u32 Register;

    Xil_AssertVoid(InstancePtr != NULL);
    Xil_AssertVoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    Register =  XNn_inference_ReadReg(InstancePtr->Axi_cpu_BaseAddress, XNN_INFERENCE_AXI_CPU_ADDR_IER);
    XNn_inference_WriteReg(InstancePtr->Axi_cpu_BaseAddress, XNN_INFERENCE_AXI_CPU_ADDR_IER, Register & (~Mask));
}

void XNn_inference_InterruptClear(XNn_inference *InstancePtr, u32 Mask) {
    Xil_AssertVoid(InstancePtr != NULL);
    Xil_AssertVoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    XNn_inference_WriteReg(InstancePtr->Axi_cpu_BaseAddress, XNN_INFERENCE_AXI_CPU_ADDR_ISR, Mask);
}

u32 XNn_inference_InterruptGetEnabled(XNn_inference *InstancePtr) {
    Xil_AssertNonvoid(InstancePtr != NULL);
    Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    return XNn_inference_ReadReg(InstancePtr->Axi_cpu_BaseAddress, XNN_INFERENCE_AXI_CPU_ADDR_IER);
}

u32 XNn_inference_InterruptGetStatus(XNn_inference *InstancePtr) {
    Xil_AssertNonvoid(InstancePtr != NULL);
    Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    return XNn_inference_ReadReg(InstancePtr->Axi_cpu_BaseAddress, XNN_INFERENCE_AXI_CPU_ADDR_ISR);
}


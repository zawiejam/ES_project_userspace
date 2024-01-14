// ==============================================================
// Vitis HLS - High-Level Synthesis from C, C++ and OpenCL v2020.2 (64-bit)
// Copyright 1986-2020 Xilinx, Inc. All Rights Reserved.
// ==============================================================
#ifndef XNN_INFERENCE_H
#define XNN_INFERENCE_H

#ifdef __cplusplus
extern "C" {
#endif

/***************************** Include Files *********************************/
#ifndef __linux__
#include "xil_types.h"
#include "xil_assert.h"
#include "xstatus.h"
#include "xil_io.h"
#else
#include <stdint.h>
#include <assert.h>
#include <dirent.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/mman.h>
#include <unistd.h>
#include <stddef.h>
#endif
#include "xnn_inference_hw.h"

/**************************** Type Definitions ******************************/
#ifdef __linux__
typedef uint8_t u8;
typedef uint16_t u16;
typedef uint32_t u32;
typedef uint64_t u64;
#else
typedef struct {
    u16 DeviceId;
    u32 Axi_cpu_BaseAddress;
} XNn_inference_Config;
#endif

typedef struct {
    u64 Axi_cpu_BaseAddress;
    u32 IsReady;
} XNn_inference;

typedef u32 word_type;

/***************** Macros (Inline Functions) Definitions *********************/
#ifndef __linux__
#define XNn_inference_WriteReg(BaseAddress, RegOffset, Data) \
    Xil_Out32((BaseAddress) + (RegOffset), (u32)(Data))
#define XNn_inference_ReadReg(BaseAddress, RegOffset) \
    Xil_In32((BaseAddress) + (RegOffset))
#else
#define XNn_inference_WriteReg(BaseAddress, RegOffset, Data) \
    *(volatile u32*)((BaseAddress) + (RegOffset)) = (u32)(Data)
#define XNn_inference_ReadReg(BaseAddress, RegOffset) \
    *(volatile u32*)((BaseAddress) + (RegOffset))

#define Xil_AssertVoid(expr)    assert(expr)
#define Xil_AssertNonvoid(expr) assert(expr)

#define XST_SUCCESS             0
#define XST_DEVICE_NOT_FOUND    2
#define XST_OPEN_DEVICE_FAILED  3
#define XIL_COMPONENT_IS_READY  1
#endif

/************************** Function Prototypes *****************************/
#ifndef __linux__
int XNn_inference_Initialize(XNn_inference *InstancePtr, u16 DeviceId);
XNn_inference_Config* XNn_inference_LookupConfig(u16 DeviceId);
int XNn_inference_CfgInitialize(XNn_inference *InstancePtr, XNn_inference_Config *ConfigPtr);
#else
int XNn_inference_Initialize(XNn_inference *InstancePtr, const char* InstanceName);
int XNn_inference_Release(XNn_inference *InstancePtr);
#endif

void XNn_inference_Start(XNn_inference *InstancePtr);
u32 XNn_inference_IsDone(XNn_inference *InstancePtr);
u32 XNn_inference_IsIdle(XNn_inference *InstancePtr);
u32 XNn_inference_IsReady(XNn_inference *InstancePtr);
void XNn_inference_EnableAutoRestart(XNn_inference *InstancePtr);
void XNn_inference_DisableAutoRestart(XNn_inference *InstancePtr);
u32 XNn_inference_Get_return(XNn_inference *InstancePtr);

u32 XNn_inference_Get_input_img_BaseAddress(XNn_inference *InstancePtr);
u32 XNn_inference_Get_input_img_HighAddress(XNn_inference *InstancePtr);
u32 XNn_inference_Get_input_img_TotalBytes(XNn_inference *InstancePtr);
u32 XNn_inference_Get_input_img_BitWidth(XNn_inference *InstancePtr);
u32 XNn_inference_Get_input_img_Depth(XNn_inference *InstancePtr);
u32 XNn_inference_Write_input_img_Words(XNn_inference *InstancePtr, int offset, word_type *data, int length);
u32 XNn_inference_Read_input_img_Words(XNn_inference *InstancePtr, int offset, word_type *data, int length);
u32 XNn_inference_Write_input_img_Bytes(XNn_inference *InstancePtr, int offset, char *data, int length);
u32 XNn_inference_Read_input_img_Bytes(XNn_inference *InstancePtr, int offset, char *data, int length);

void XNn_inference_InterruptGlobalEnable(XNn_inference *InstancePtr);
void XNn_inference_InterruptGlobalDisable(XNn_inference *InstancePtr);
void XNn_inference_InterruptEnable(XNn_inference *InstancePtr, u32 Mask);
void XNn_inference_InterruptDisable(XNn_inference *InstancePtr, u32 Mask);
void XNn_inference_InterruptClear(XNn_inference *InstancePtr, u32 Mask);
u32 XNn_inference_InterruptGetEnabled(XNn_inference *InstancePtr);
u32 XNn_inference_InterruptGetStatus(XNn_inference *InstancePtr);

#ifdef __cplusplus
}
#endif

#endif

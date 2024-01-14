// ==============================================================
// Vitis HLS - High-Level Synthesis from C, C++ and OpenCL v2020.2 (64-bit)
// Copyright 1986-2020 Xilinx, Inc. All Rights Reserved.
// ==============================================================
#ifndef __linux__

#include "xstatus.h"
#include "xparameters.h"
#include "xnn_inference.h"

extern XNn_inference_Config XNn_inference_ConfigTable[];

XNn_inference_Config *XNn_inference_LookupConfig(u16 DeviceId) {
	XNn_inference_Config *ConfigPtr = NULL;

	int Index;

	for (Index = 0; Index < XPAR_XNN_INFERENCE_NUM_INSTANCES; Index++) {
		if (XNn_inference_ConfigTable[Index].DeviceId == DeviceId) {
			ConfigPtr = &XNn_inference_ConfigTable[Index];
			break;
		}
	}

	return ConfigPtr;
}

int XNn_inference_Initialize(XNn_inference *InstancePtr, u16 DeviceId) {
	XNn_inference_Config *ConfigPtr;

	Xil_AssertNonvoid(InstancePtr != NULL);

	ConfigPtr = XNn_inference_LookupConfig(DeviceId);
	if (ConfigPtr == NULL) {
		InstancePtr->IsReady = 0;
		return (XST_DEVICE_NOT_FOUND);
	}

	return XNn_inference_CfgInitialize(InstancePtr, ConfigPtr);
}

#endif


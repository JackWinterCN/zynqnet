// ==============================================================
// Vitis HLS - High-Level Synthesis from C, C++ and OpenCL v2020.1 (64-bit)
// Copyright 1986-2020 Xilinx, Inc. All Rights Reserved.
// ==============================================================
/***************************** Include Files *********************************/
#include "xfpga_top.h"
#include <cassert>
/************************** Function Implementation *************************/
#ifndef __NOT_EXIST_MACRO_
int XFpga_top_CfgInitialize(XFpga_top *InstancePtr, XFpga_top_Config *ConfigPtr) {
    assert(InstancePtr != NULL);
    assert(ConfigPtr != NULL);

    InstancePtr->Axilite_BaseAddress = ConfigPtr->Axilite_BaseAddress;
    InstancePtr->Control_BaseAddress = ConfigPtr->Control_BaseAddress;
    InstancePtr->IsReady = XIL_COMPONENT_IS_READY;

    return XST_SUCCESS;
}
#endif

void XFpga_top_Start(XFpga_top *InstancePtr) {
    u32 Data;

    assert(InstancePtr != NULL);
    assert(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    Data = XFpga_top_ReadReg(InstancePtr->Axilite_BaseAddress, XFPGA_TOP_AXILITE_ADDR_AP_CTRL) & 0x80;
    XFpga_top_WriteReg(InstancePtr->Axilite_BaseAddress, XFPGA_TOP_AXILITE_ADDR_AP_CTRL, Data | 0x01);
}

u32 XFpga_top_IsDone(XFpga_top *InstancePtr) {
    u32 Data;

    assert(InstancePtr != NULL);
    assert(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    Data = XFpga_top_ReadReg(InstancePtr->Axilite_BaseAddress, XFPGA_TOP_AXILITE_ADDR_AP_CTRL);
    return (Data >> 1) & 0x1;
}

u32 XFpga_top_IsIdle(XFpga_top *InstancePtr) {
    u32 Data;

    assert(InstancePtr != NULL);
    assert(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    Data = XFpga_top_ReadReg(InstancePtr->Axilite_BaseAddress, XFPGA_TOP_AXILITE_ADDR_AP_CTRL);
    return (Data >> 2) & 0x1;
}

u32 XFpga_top_IsReady(XFpga_top *InstancePtr) {
    u32 Data;

    assert(InstancePtr != NULL);
    assert(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    Data = XFpga_top_ReadReg(InstancePtr->Axilite_BaseAddress, XFPGA_TOP_AXILITE_ADDR_AP_CTRL);
    // check ap_start to see if the pcore is ready for next input
    return !(Data & 0x1);
}

void XFpga_top_EnableAutoRestart(XFpga_top *InstancePtr) {
    assert(InstancePtr != NULL);
    assert(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    XFpga_top_WriteReg(InstancePtr->Axilite_BaseAddress, XFPGA_TOP_AXILITE_ADDR_AP_CTRL, 0x80);
}

void XFpga_top_DisableAutoRestart(XFpga_top *InstancePtr) {
    assert(InstancePtr != NULL);
    assert(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    XFpga_top_WriteReg(InstancePtr->Axilite_BaseAddress, XFPGA_TOP_AXILITE_ADDR_AP_CTRL, 0);
}

void XFpga_top_Set_layer(XFpga_top *InstancePtr, XFpga_top_Layer Data) {
    assert(InstancePtr != NULL);
    assert(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    XFpga_top_WriteReg(InstancePtr->Axilite_BaseAddress, XFPGA_TOP_AXILITE_ADDR_LAYER_DATA + 0, Data.word_0);
    XFpga_top_WriteReg(InstancePtr->Axilite_BaseAddress, XFPGA_TOP_AXILITE_ADDR_LAYER_DATA + 4, Data.word_1);
    XFpga_top_WriteReg(InstancePtr->Axilite_BaseAddress, XFPGA_TOP_AXILITE_ADDR_LAYER_DATA + 8, Data.word_2);
    XFpga_top_WriteReg(InstancePtr->Axilite_BaseAddress, XFPGA_TOP_AXILITE_ADDR_LAYER_DATA + 12, Data.word_3);
    XFpga_top_WriteReg(InstancePtr->Axilite_BaseAddress, XFPGA_TOP_AXILITE_ADDR_LAYER_DATA + 16, Data.word_4);
    XFpga_top_WriteReg(InstancePtr->Axilite_BaseAddress, XFPGA_TOP_AXILITE_ADDR_LAYER_DATA + 20, Data.word_5);
    XFpga_top_WriteReg(InstancePtr->Axilite_BaseAddress, XFPGA_TOP_AXILITE_ADDR_LAYER_DATA + 24, Data.word_6);
    XFpga_top_WriteReg(InstancePtr->Axilite_BaseAddress, XFPGA_TOP_AXILITE_ADDR_LAYER_DATA + 28, Data.word_7);
    XFpga_top_WriteReg(InstancePtr->Axilite_BaseAddress, XFPGA_TOP_AXILITE_ADDR_LAYER_DATA + 32, Data.word_8);
    XFpga_top_WriteReg(InstancePtr->Axilite_BaseAddress, XFPGA_TOP_AXILITE_ADDR_LAYER_DATA + 36, Data.word_9);
}

XFpga_top_Layer XFpga_top_Get_layer(XFpga_top *InstancePtr) {
    XFpga_top_Layer Data;

    Data.word_0 = XFpga_top_ReadReg(InstancePtr->Axilite_BaseAddress, XFPGA_TOP_AXILITE_ADDR_LAYER_DATA + 0);
    Data.word_1 = XFpga_top_ReadReg(InstancePtr->Axilite_BaseAddress, XFPGA_TOP_AXILITE_ADDR_LAYER_DATA + 4);
    Data.word_2 = XFpga_top_ReadReg(InstancePtr->Axilite_BaseAddress, XFPGA_TOP_AXILITE_ADDR_LAYER_DATA + 8);
    Data.word_3 = XFpga_top_ReadReg(InstancePtr->Axilite_BaseAddress, XFPGA_TOP_AXILITE_ADDR_LAYER_DATA + 12);
    Data.word_4 = XFpga_top_ReadReg(InstancePtr->Axilite_BaseAddress, XFPGA_TOP_AXILITE_ADDR_LAYER_DATA + 16);
    Data.word_5 = XFpga_top_ReadReg(InstancePtr->Axilite_BaseAddress, XFPGA_TOP_AXILITE_ADDR_LAYER_DATA + 20);
    Data.word_6 = XFpga_top_ReadReg(InstancePtr->Axilite_BaseAddress, XFPGA_TOP_AXILITE_ADDR_LAYER_DATA + 24);
    Data.word_7 = XFpga_top_ReadReg(InstancePtr->Axilite_BaseAddress, XFPGA_TOP_AXILITE_ADDR_LAYER_DATA + 28);
    Data.word_8 = XFpga_top_ReadReg(InstancePtr->Axilite_BaseAddress, XFPGA_TOP_AXILITE_ADDR_LAYER_DATA + 32);
    Data.word_9 = XFpga_top_ReadReg(InstancePtr->Axilite_BaseAddress, XFPGA_TOP_AXILITE_ADDR_LAYER_DATA + 36);
    return Data;
}

void XFpga_top_Set_weights_offset(XFpga_top *InstancePtr, u32 Data) {
    assert(InstancePtr != NULL);
    assert(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    XFpga_top_WriteReg(InstancePtr->Axilite_BaseAddress, XFPGA_TOP_AXILITE_ADDR_WEIGHTS_OFFSET_DATA, Data);
}

u32 XFpga_top_Get_weights_offset(XFpga_top *InstancePtr) {
    u32 Data;

    assert(InstancePtr != NULL);
    assert(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    Data = XFpga_top_ReadReg(InstancePtr->Axilite_BaseAddress, XFPGA_TOP_AXILITE_ADDR_WEIGHTS_OFFSET_DATA);
    return Data;
}

void XFpga_top_Set_num_weights(XFpga_top *InstancePtr, u32 Data) {
    assert(InstancePtr != NULL);
    assert(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    XFpga_top_WriteReg(InstancePtr->Axilite_BaseAddress, XFPGA_TOP_AXILITE_ADDR_NUM_WEIGHTS_DATA, Data);
}

u32 XFpga_top_Get_num_weights(XFpga_top *InstancePtr) {
    u32 Data;

    assert(InstancePtr != NULL);
    assert(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    Data = XFpga_top_ReadReg(InstancePtr->Axilite_BaseAddress, XFPGA_TOP_AXILITE_ADDR_NUM_WEIGHTS_DATA);
    return Data;
}

void XFpga_top_Set_input_offset(XFpga_top *InstancePtr, u32 Data) {
    assert(InstancePtr != NULL);
    assert(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    XFpga_top_WriteReg(InstancePtr->Axilite_BaseAddress, XFPGA_TOP_AXILITE_ADDR_INPUT_OFFSET_DATA, Data);
}

u32 XFpga_top_Get_input_offset(XFpga_top *InstancePtr) {
    u32 Data;

    assert(InstancePtr != NULL);
    assert(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    Data = XFpga_top_ReadReg(InstancePtr->Axilite_BaseAddress, XFPGA_TOP_AXILITE_ADDR_INPUT_OFFSET_DATA);
    return Data;
}

void XFpga_top_Set_SHARED_DRAM(XFpga_top *InstancePtr, u64 Data) {
    assert(InstancePtr != NULL);
    assert(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    XFpga_top_WriteReg(InstancePtr->Control_BaseAddress, XFPGA_TOP_CONTROL_ADDR_SHARED_DRAM_DATA, (u32)(Data));
    XFpga_top_WriteReg(InstancePtr->Control_BaseAddress, XFPGA_TOP_CONTROL_ADDR_SHARED_DRAM_DATA + 4, (u32)(Data >> 32));
}

u64 XFpga_top_Get_SHARED_DRAM(XFpga_top *InstancePtr) {
    u64 Data;

    assert(InstancePtr != NULL);
    assert(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    Data = XFpga_top_ReadReg(InstancePtr->Control_BaseAddress, XFPGA_TOP_CONTROL_ADDR_SHARED_DRAM_DATA);
    Data += (u64)XFpga_top_ReadReg(InstancePtr->Control_BaseAddress, XFPGA_TOP_CONTROL_ADDR_SHARED_DRAM_DATA + 4) << 32;
    return Data;
}

void XFpga_top_InterruptGlobalEnable(XFpga_top *InstancePtr) {
    assert(InstancePtr != NULL);
    assert(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    XFpga_top_WriteReg(InstancePtr->Axilite_BaseAddress, XFPGA_TOP_AXILITE_ADDR_GIE, 1);
}

void XFpga_top_InterruptGlobalDisable(XFpga_top *InstancePtr) {
    assert(InstancePtr != NULL);
    assert(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    XFpga_top_WriteReg(InstancePtr->Axilite_BaseAddress, XFPGA_TOP_AXILITE_ADDR_GIE, 0);
}

void XFpga_top_InterruptEnable(XFpga_top *InstancePtr, u32 Mask) {
    u32 Register;

    assert(InstancePtr != NULL);
    assert(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    Register =  XFpga_top_ReadReg(InstancePtr->Axilite_BaseAddress, XFPGA_TOP_AXILITE_ADDR_IER);
    XFpga_top_WriteReg(InstancePtr->Axilite_BaseAddress, XFPGA_TOP_AXILITE_ADDR_IER, Register | Mask);
}

void XFpga_top_InterruptDisable(XFpga_top *InstancePtr, u32 Mask) {
    u32 Register;

    assert(InstancePtr != NULL);
    assert(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    Register =  XFpga_top_ReadReg(InstancePtr->Axilite_BaseAddress, XFPGA_TOP_AXILITE_ADDR_IER);
    XFpga_top_WriteReg(InstancePtr->Axilite_BaseAddress, XFPGA_TOP_AXILITE_ADDR_IER, Register & (~Mask));
}

void XFpga_top_InterruptClear(XFpga_top *InstancePtr, u32 Mask) {
    assert(InstancePtr != NULL);
    assert(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    XFpga_top_WriteReg(InstancePtr->Axilite_BaseAddress, XFPGA_TOP_AXILITE_ADDR_ISR, Mask);
}

u32 XFpga_top_InterruptGetEnabled(XFpga_top *InstancePtr) {
    assert(InstancePtr != NULL);
    assert(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    return XFpga_top_ReadReg(InstancePtr->Axilite_BaseAddress, XFPGA_TOP_AXILITE_ADDR_IER);
}

u32 XFpga_top_InterruptGetStatus(XFpga_top *InstancePtr) {
    assert(InstancePtr != NULL);
    assert(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    return XFpga_top_ReadReg(InstancePtr->Axilite_BaseAddress, XFPGA_TOP_AXILITE_ADDR_ISR);
}


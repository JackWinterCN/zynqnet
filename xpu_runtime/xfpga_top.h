// ==============================================================
// Vitis HLS - High-Level Synthesis from C, C++ and OpenCL v2020.1 (64-bit)
// Copyright 1986-2020 Xilinx, Inc. All Rights Reserved.
// ==============================================================
#ifndef XFPGA_TOP_H
#define XFPGA_TOP_H

#ifdef __cplusplus
extern "C" {
#endif

/***************************** Include Files *********************************/
#ifndef __NOT_EXIST_MACRO_
#include "xil_types.h"
// #include "xil_assert.h"
#include "xstatus.h"
// #include "xil_io.h"
#else
// #include <stdint.h>
// #include <assert.h>
// #include <dirent.h>
// #include <fcntl.h>
// #include <stdio.h>
// #include <stdlib.h>
// #include <string.h>
// #include <sys/mman.h>
// #include <unistd.h>
// #include <stddef.h>
#endif
#include "xfpga_top_hw.h"

/**************************** Type Definitions ******************************/
#ifdef __NOT_EXIST_MACRO_
// typedef uint8_t u8;
// typedef uint16_t u16;
// typedef uint32_t u32;
// typedef uint64_t u64;
#else
typedef struct {
    u16 DeviceId;
    u32 Axilite_BaseAddress;
    u32 Control_BaseAddress;
} XFpga_top_Config;
#endif

typedef struct {
    u32 Axilite_BaseAddress;
    u32 Control_BaseAddress;
    u32 IsReady;
} XFpga_top;

typedef u32 word_type;

typedef struct {
    u32 word_0;
    u32 word_1;
    u32 word_2;
    u32 word_3;
    u32 word_4;
    u32 word_5;
    u32 word_6;
    u32 word_7;
    u32 word_8;
    u32 word_9;
} XFpga_top_Layer;

/***************** Macros (Inline Functions) Definitions *********************/
#ifndef __NOT_EXIST_MACRO_
static inline void Xil_Out32(UINTPTR Addr, u32 Value)
{
	volatile u32 *LocalAddr = (volatile u32 *)Addr;
	*LocalAddr = Value;
}
static inline u32 Xil_In32(UINTPTR Addr)
{
	return *(volatile u32 *) Addr;
}
#define XFpga_top_WriteReg(BaseAddress, RegOffset, Data) \
    Xil_Out32((BaseAddress) + (RegOffset), (u32)(Data))
#define XFpga_top_ReadReg(BaseAddress, RegOffset) \
    Xil_In32((BaseAddress) + (RegOffset))
#else
// #define XFpga_top_WriteReg(BaseAddress, RegOffset, Data) \
//     *(volatile u32*)((BaseAddress) + (RegOffset)) = (u32)(Data)
// #define XFpga_top_ReadReg(BaseAddress, RegOffset) \
//     *(volatile u32*)((BaseAddress) + (RegOffset))

// #define Xil_AssertVoid(expr)    assert(expr)
// #define Xil_AssertNonvoid(expr) assert(expr)

// #define XST_SUCCESS             0
// #define XST_DEVICE_NOT_FOUND    2
// #define XST_OPEN_DEVICE_FAILED  3
// #define XIL_COMPONENT_IS_READY  1
#endif

/************************** Function Prototypes *****************************/
#ifndef __NOT_EXIST_MACRO_
int XFpga_top_Initialize(XFpga_top *InstancePtr, u16 DeviceId);
XFpga_top_Config* XFpga_top_LookupConfig(u16 DeviceId);
int XFpga_top_CfgInitialize(XFpga_top *InstancePtr, XFpga_top_Config *ConfigPtr);
#else
// int XFpga_top_Initialize(XFpga_top *InstancePtr, const char* InstanceName);
// int XFpga_top_Release(XFpga_top *InstancePtr);
#endif

void XFpga_top_Start(XFpga_top *InstancePtr);
u32 XFpga_top_IsDone(XFpga_top *InstancePtr);
u32 XFpga_top_IsIdle(XFpga_top *InstancePtr);
u32 XFpga_top_IsReady(XFpga_top *InstancePtr);
void XFpga_top_EnableAutoRestart(XFpga_top *InstancePtr);
void XFpga_top_DisableAutoRestart(XFpga_top *InstancePtr);

void XFpga_top_Set_layer(XFpga_top *InstancePtr, XFpga_top_Layer Data);
XFpga_top_Layer XFpga_top_Get_layer(XFpga_top *InstancePtr);
void XFpga_top_Set_weights_offset(XFpga_top *InstancePtr, u32 Data);
u32 XFpga_top_Get_weights_offset(XFpga_top *InstancePtr);
void XFpga_top_Set_num_weights(XFpga_top *InstancePtr, u32 Data);
u32 XFpga_top_Get_num_weights(XFpga_top *InstancePtr);
void XFpga_top_Set_input_offset(XFpga_top *InstancePtr, u32 Data);
u32 XFpga_top_Get_input_offset(XFpga_top *InstancePtr);
void XFpga_top_Set_SHARED_DRAM(XFpga_top *InstancePtr, u64 Data);
u64 XFpga_top_Get_SHARED_DRAM(XFpga_top *InstancePtr);

void XFpga_top_InterruptGlobalEnable(XFpga_top *InstancePtr);
void XFpga_top_InterruptGlobalDisable(XFpga_top *InstancePtr);
void XFpga_top_InterruptEnable(XFpga_top *InstancePtr, u32 Mask);
void XFpga_top_InterruptDisable(XFpga_top *InstancePtr, u32 Mask);
void XFpga_top_InterruptClear(XFpga_top *InstancePtr, u32 Mask);
u32 XFpga_top_InterruptGetEnabled(XFpga_top *InstancePtr);
u32 XFpga_top_InterruptGetStatus(XFpga_top *InstancePtr);

#ifdef __cplusplus
}
#endif

#endif

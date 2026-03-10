// ==============================================================
// Vitis HLS - High-Level Synthesis from C, C++ and OpenCL v2020.1 (64-bit)
// Copyright 1986-2020 Xilinx, Inc. All Rights Reserved.
// ==============================================================
#ifndef XFPGA_TOP_H
#define XFPGA_TOP_H

#ifdef __cplusplus
extern "C" {
#endif

#define __NOT_EXIST_MACRO_ 1
/***************************** Include Files *********************************/
#include <sys/mman.h>

#ifndef __NOT_EXIST_MACRO_
#include "xil_types.h"
// #include "xil_assert.h"
#include "xstatus.h"
// #include "xil_io.h"
#else
#include <stdint.h>
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
#include "xparameters.h"
/**************************** Type Definitions ******************************/
#ifdef __NOT_EXIST_MACRO_
// typedef uint8_t u8;
// typedef uint16_t uint16_t;
// typedef uint32_t uint16_t;
// typedef uint64_t uint64_t;
#else
typedef struct {
    u16 DeviceId;
    uint32_t Axilite_BaseAddress;
    uint32_t Control_BaseAddress;
} XFpga_top_Config;
#endif

typedef struct {
  volatile uint32_t *Axilite_BaseAddress;
  volatile uint32_t *Control_BaseAddress;
  uint32_t IsReady;
} XFpga_top;

typedef uint32_t word_type;

typedef struct {
    uint32_t word_0;
    uint32_t word_1;
    uint32_t word_2;
    uint32_t word_3;
    uint32_t word_4;
    uint32_t word_5;
    uint32_t word_6;
    uint32_t word_7;
    uint32_t word_8;
    uint32_t word_9;
} XFpga_top_Layer;

/***************** Macros (Inline Functions) Definitions *********************/
#ifndef __NOT_EXIST_MACRO_
static inline void Xil_Out32(UINTPTR Addr, uint32_t Value)
{
    volatile uint32_t *LocalAddr = (volatile uint32_t *)Addr;
	*LocalAddr = Value;
}
static inline uint32_t Xil_In32(UINTPTR Addr)
{
    return *(volatile uint32_t *) Addr;
}
#define XFpga_top_WriteReg(BaseAddress, RegOffset, Data) \
    Xil_Out32((BaseAddress) + (RegOffset), (uint32_t)(Data))
#define XFpga_top_ReadReg(BaseAddress, RegOffset) \
    Xil_In32((BaseAddress) + (RegOffset))
#else
#define XFpga_top_WriteReg(BaseAddress, RegOffset, Data) \
    *(volatile uint32_t*)((BaseAddress) + (RegOffset)) = (uint32_t)(Data)
#define XFpga_top_ReadReg(BaseAddress, RegOffset) \
    *(volatile uint32_t*)((BaseAddress) + (RegOffset))

// #define Xil_AssertVoid(expr)    assert(expr)
// #define Xil_AssertNonvoid(expr) assert(expr)

// #define XST_SUCCESS             0
// #define XST_DEVICE_NOT_FOUND    2
// #define XST_OPEN_DEVICE_FAILED  3
// #define XIL_COMPONENT_IS_READY  1
#endif



// typedef uint8_t u8;
// typedef uint16_t uint16_t;
// typedef uint32_t uint16_t;

// Location + Size of Axilite-MMAP segment:
// - from Vivado Block Designer (Address Editor):
//     AXILITE memory starts at 0x43c00000 – 0x43c0FFFF, SIZE: 64KB
// - from xfpga_top_hw.h:
//     highest byte-address is 0xa0. 0xa4 is "reserved"
// const off_t AXILITE_BASE_ADDR = 0x43c00000;
const off_t AXILITE_BASE_ADDR = XPAR_XFPGA_TOP_0_S_AXI_AXILITE_BASEADDR;
const size_t AXILITE_MEM_SIZE = 0xFFFF;  // actual address range is 64KB
extern int AXILITE_MEM_FD;
extern volatile uint32_t* AXILITE_MMAP_BUS;

volatile uint32_t* map_axilite_bus(off_t base_addr);
void release_axilite_bus(volatile uint32_t* axilite);
bool axilite_open();
bool axilite_close();
// 32-bit word read + write (other sizes not supported!)
void axilite_write(uint32_t byte_addr, uint32_t value);
uint32_t axilite_read(uint32_t byte_addr);


const off_t AXI_CONTROL_BASE_ADDR = XPAR_XFPGA_TOP_0_S_AXI_CONTROL_BASEADDR;
const size_t AXI_CONTROL_MEM_SIZE = 0xFFFF;  // actual address range is 64KB
extern int AXI_CONTROL_MEM_FD;
extern volatile uint32_t* AXI_CONTROL_MMAP_BUS;

volatile uint32_t* map_axi_control_bus(off_t base_addr);
void release_axi_control_bus(volatile uint32_t* axi_control);
bool axi_control_open();
bool axi_control_close();
void axi_control_write(uint32_t byte_addr, uint32_t value);
uint32_t axi_control_read(uint32_t byte_addr);

volatile uint32_t *XFPGA_shared_DRAM_virtual();
volatile uint32_t *XFPGA_shared_DRAM_physical();
/************************** Function Prototypes *****************************/
#ifndef __NOT_EXIST_MACRO_
int XFpga_top_Initialize(XFpga_top *InstancePtr, uint16_t DeviceId);
XFpga_top_Config* XFpga_top_LookupConfig(uint16_t DeviceId);
int XFpga_top_CfgInitialize(XFpga_top *InstancePtr, XFpga_top_Config *ConfigPtr);
#else
int XFpga_top_Initialize(XFpga_top *InstancePtr, const char* InstanceName);
int XFpga_top_Release(XFpga_top *InstancePtr);
#endif

void XFpga_top_Start(XFpga_top *InstancePtr);
uint32_t XFpga_top_IsDone(XFpga_top *InstancePtr);
uint32_t XFpga_top_IsIdle(XFpga_top *InstancePtr);
uint32_t XFpga_top_IsReady(XFpga_top *InstancePtr);
void XFpga_top_EnableAutoRestart(XFpga_top *InstancePtr);
void XFpga_top_DisableAutoRestart(XFpga_top *InstancePtr);

void XFpga_top_Set_layer(XFpga_top *InstancePtr, XFpga_top_Layer Data);
XFpga_top_Layer XFpga_top_Get_layer(XFpga_top *InstancePtr);
void XFpga_top_Set_weights_offset(XFpga_top *InstancePtr, uint32_t Data);
uint32_t XFpga_top_Get_weights_offset(XFpga_top *InstancePtr);
void XFpga_top_Set_num_weights(XFpga_top *InstancePtr, uint32_t Data);
uint32_t XFpga_top_Get_num_weights(XFpga_top *InstancePtr);
void XFpga_top_Set_input_offset(XFpga_top *InstancePtr, uint32_t Data);
uint32_t XFpga_top_Get_input_offset(XFpga_top *InstancePtr);
void XFpga_top_Set_SHARED_DRAM(XFpga_top *InstancePtr, uint64_t Data);
uint64_t XFpga_top_Get_SHARED_DRAM(XFpga_top *InstancePtr);

void XFpga_top_InterruptGlobalEnable(XFpga_top *InstancePtr);
void XFpga_top_InterruptGlobalDisable(XFpga_top *InstancePtr);
void XFpga_top_InterruptEnable(XFpga_top *InstancePtr, uint32_t Mask);
void XFpga_top_InterruptDisable(XFpga_top *InstancePtr, uint32_t Mask);
void XFpga_top_InterruptClear(XFpga_top *InstancePtr, uint32_t Mask);
uint32_t XFpga_top_InterruptGetEnabled(XFpga_top *InstancePtr);
uint32_t XFpga_top_InterruptGetStatus(XFpga_top *InstancePtr);

#ifdef __cplusplus
}
#endif

#endif

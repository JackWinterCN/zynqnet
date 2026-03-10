// ==============================================================
// Vitis HLS - High-Level Synthesis from C, C++ and OpenCL v2020.1 (64-bit)
// Copyright 1986-2020 Xilinx, Inc. All Rights Reserved.
// ==============================================================
/***************************** Include Files *********************************/
#include "xfpga_top.h"
#include <cassert>

#include <sys/mman.h>
#include <fcntl.h>
#include <err.h>
#include <errno.h>

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <stddef.h>
#include <stdint.h>

#include "xfpga_top_hw.h"   // Register addresses
#include "xparameters.h"
#include "xstatus.h"
#include "shared_dram.hpp"
/************************** Function Implementation *************************/


int AXILITE_FD = -1;
volatile uint32_t* AXILITE_BUS = NULL;
  
volatile uint32_t* map_axilite_bus(off_t base_addr) {
  // make sure that base addr is aligned to memory pages...
  base_addr &= ~(getpagesize() - 1);

  // Open /dev/mem file (need root privileges or setuid!)
  AXILITE_FD = open("/dev/mem", O_RDWR);
  if (AXILITE_FD < 0) err(errno, "could not open /dev/mem. need to be root");

  // Map AXILITE memory region to pointer
  volatile uint32_t* axilite = (uint32_t*)mmap(NULL, AXILITE_MEM_SIZE, PROT_READ | PROT_WRITE,
                            MAP_SHARED, AXILITE_FD, base_addr);
  if (axilite == MAP_FAILED) err(errno, "could not map memory for axilite bus");

  return axilite;
}

void release_axilite_bus(volatile uint32_t* axilite) {
  // Release AXILITE memory region (unmap)
  int retval = munmap((void*)axilite, AXILITE_MEM_SIZE);
  if (retval < 0) err(errno, "could not unmap memory region for axilite bus");
  
  // release file handle
  retval = close(AXILITE_FD);
  if (retval < 0) err(errno, "could not release /dev/mem file handle");

  // set file handle variable s.t. we know it's closed
  AXILITE_FD = -1;
}

bool axilite_open() {
  
  // Check that it's not yet open
  if (AXILITE_FD > -1) {
    printf("axilite bus already open!\n"); 
    return false;
  }
  
  // Memory Map Axilite Bus
  AXILITE_BUS = map_axilite_bus(AXILITE_BASE_ADDR);
  
  // Make sure the file handle is really set
  return (AXILITE_FD > -1);
}

/* Write one 32bit Word */
void axilite_write(uint32_t byte_addr, uint32_t value) {
  AXILITE_BUS[byte_addr / 4] = value;
}

/* Read one 32bit Word */
uint32_t axilite_read(uint32_t byte_addr) {
  return AXILITE_BUS[byte_addr / 4];
}

bool axilite_close() {
  
  // Check that memory file is really open
  if (AXILITE_FD == -1) {
    printf("axilite bus not open!\n"); 
    return false;
  }
  
  // Release Memory Region and File handle
  release_axilite_bus(AXILITE_BUS);
  
  // Make sure file was correctly released
  return (AXILITE_FD == -1);
}


int AXI_CONTROL_MEM_FD = -1;
volatile uint32_t* AXI_CONTROL_MMAP_BUS = NULL;
  
volatile uint32_t* map_axi_control_bus(off_t base_addr) {
  // make sure that base addr is aligned to memory pages...
  base_addr &= ~(getpagesize() - 1);

  // Open /dev/mem file (need root privileges or setuid!)
  AXI_CONTROL_MEM_FD = open("/dev/mem", O_RDWR);
  if (AXI_CONTROL_MEM_FD < 0) err(errno, "could not open /dev/mem. need to be root");

  // Map AXILITE memory region to pointer
  volatile uint32_t* axi_control = (uint32_t*)mmap(NULL, AXI_CONTROL_MEM_SIZE, PROT_READ | PROT_WRITE,
                            MAP_SHARED, AXI_CONTROL_MEM_FD, base_addr);
  if (axi_control == MAP_FAILED) err(errno, "could not map memory for axi control bus");

  return axi_control;
}

void release_axi_control_bus(volatile uint32_t* axi_control) {
  // Release AXILITE memory region (unmap)
  int retval = munmap((void*)axi_control, AXI_CONTROL_MEM_SIZE);
  if (retval < 0) err(errno, "could not unmap memory region for axi control bus");
  
  // release file handle
  retval = close(AXI_CONTROL_MEM_FD);
  if (retval < 0) err(errno, "could not release /dev/mem file handle");

  // set file handle variable s.t. we know it's closed
  AXI_CONTROL_MEM_FD = -1;
}

bool axi_control_open() {
  
  // Check that it's not yet open
  if (AXI_CONTROL_MEM_FD > -1) {
    printf("axi control bus already open!\n"); 
    return false;
  }
  
  // Memory Map Axilite Bus
  AXI_CONTROL_MMAP_BUS = map_axi_control_bus(AXI_CONTROL_BASE_ADDR);
  
  // Make sure the file handle is really set
  return (AXI_CONTROL_MEM_FD > -1);
}

bool axi_control_close() {
  
  // Check that memory file is really open
  if (AXI_CONTROL_MEM_FD == -1) {
    printf("axi control bus not open!\n"); 
    return false;
  }
  
  // Release Memory Region and File handle
  release_axi_control_bus(AXI_CONTROL_MMAP_BUS);
  
  // Make sure file was correctly released
  return (AXI_CONTROL_MEM_FD == -1);
}

/* Write one 32bit Word */
void axi_control_write(uint32_t byte_addr, uint32_t value) {
  AXI_CONTROL_MMAP_BUS[byte_addr / 4] = value;
}

/* Read one 32bit Word */
uint32_t axi_control_read(uint32_t byte_addr) {
  return AXI_CONTROL_MMAP_BUS[byte_addr / 4];
}


volatile uint32_t *XFPGA_shared_DRAM_virtual() {
    printf("XFPGA Driver: SHARED_DRAM_virtual() = %p\n", (void*)(SHARED_DRAM_virtual()));
    return (volatile uint32_t*) (SHARED_DRAM_virtual());
}

volatile uint32_t *XFPGA_shared_DRAM_physical() {
    printf("XFPGA Driver: SHARED_DRAM_physical() = %p\n", (void*)(SHARED_DRAM_physical()));
    return (volatile uint32_t*) (SHARED_DRAM_physical());
}


int XFpga_top_Initialize(XFpga_top *InstancePtr, const char* InstanceName) {
    printf("XFPGA Driver: Initialize\n");
    axilite_open();
    axi_control_open();

    InstancePtr->Axilite_BaseAddress = AXILITE_BUS;
    InstancePtr->Control_BaseAddress = AXI_CONTROL_MMAP_BUS;
    InstancePtr->IsReady = XIL_COMPONENT_IS_READY;

    SHARED_DRAM_open();

    return XST_SUCCESS;
}

int XFpga_top_Release(XFpga_top *InstancePtr) {
  printf("XFPGA Driver: Release\n");
  axilite_close();
  axi_control_close();
  SHARED_DRAM_close();
  InstancePtr->Axilite_BaseAddress = nullptr;
  InstancePtr->Control_BaseAddress = nullptr;
  InstancePtr->IsReady = XIL_COMPONENT_IS_READY;
  return XST_SUCCESS;
}

void XFpga_top_Start(XFpga_top *InstancePtr) {
    uint32_t Data;

    assert(InstancePtr != NULL);
    assert(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    Data = XFpga_top_ReadReg(InstancePtr->Axilite_BaseAddress, XFPGA_TOP_AXILITE_ADDR_AP_CTRL) & 0x80;
    XFpga_top_WriteReg(InstancePtr->Axilite_BaseAddress, XFPGA_TOP_AXILITE_ADDR_AP_CTRL, Data | 0x01);
}

uint32_t XFpga_top_IsDone(XFpga_top *InstancePtr) {
    uint32_t Data;

    assert(InstancePtr != NULL);
    assert(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    Data = XFpga_top_ReadReg(InstancePtr->Axilite_BaseAddress, XFPGA_TOP_AXILITE_ADDR_AP_CTRL);
    return (Data >> 1) & 0x1;
}

uint32_t XFpga_top_IsIdle(XFpga_top *InstancePtr) {
    uint32_t Data;

    assert(InstancePtr != NULL);
    assert(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    Data = XFpga_top_ReadReg(InstancePtr->Axilite_BaseAddress, XFPGA_TOP_AXILITE_ADDR_AP_CTRL);
    return (Data >> 2) & 0x1;
}

uint32_t XFpga_top_IsReady(XFpga_top *InstancePtr) {
    uint32_t Data;

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

void XFpga_top_Set_weights_offset(XFpga_top *InstancePtr, uint32_t Data) {
    assert(InstancePtr != NULL);
    assert(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    XFpga_top_WriteReg(InstancePtr->Axilite_BaseAddress, XFPGA_TOP_AXILITE_ADDR_WEIGHTS_OFFSET_DATA, Data);
}

uint32_t XFpga_top_Get_weights_offset(XFpga_top *InstancePtr) {
    uint32_t Data;

    assert(InstancePtr != NULL);
    assert(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    Data = XFpga_top_ReadReg(InstancePtr->Axilite_BaseAddress, XFPGA_TOP_AXILITE_ADDR_WEIGHTS_OFFSET_DATA);
    return Data;
}

void XFpga_top_Set_num_weights(XFpga_top *InstancePtr, uint32_t Data) {
    assert(InstancePtr != NULL);
    assert(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    XFpga_top_WriteReg(InstancePtr->Axilite_BaseAddress, XFPGA_TOP_AXILITE_ADDR_NUM_WEIGHTS_DATA, Data);
}

uint32_t XFpga_top_Get_num_weights(XFpga_top *InstancePtr) {
    uint32_t Data;

    assert(InstancePtr != NULL);
    assert(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    Data = XFpga_top_ReadReg(InstancePtr->Axilite_BaseAddress, XFPGA_TOP_AXILITE_ADDR_NUM_WEIGHTS_DATA);
    return Data;
}

void XFpga_top_Set_input_offset(XFpga_top *InstancePtr, uint32_t Data) {
    assert(InstancePtr != NULL);
    assert(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    XFpga_top_WriteReg(InstancePtr->Axilite_BaseAddress, XFPGA_TOP_AXILITE_ADDR_INPUT_OFFSET_DATA, Data);
}

uint32_t XFpga_top_Get_input_offset(XFpga_top *InstancePtr) {
    uint32_t Data;

    assert(InstancePtr != NULL);
    assert(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    Data = XFpga_top_ReadReg(InstancePtr->Axilite_BaseAddress, XFPGA_TOP_AXILITE_ADDR_INPUT_OFFSET_DATA);
    return Data;
}

void XFpga_top_Set_SHARED_DRAM(XFpga_top *InstancePtr, uint64_t Data) {
    assert(InstancePtr != NULL);
    assert(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    XFpga_top_WriteReg(InstancePtr->Control_BaseAddress, XFPGA_TOP_CONTROL_ADDR_SHARED_DRAM_DATA, (uint32_t)(Data));
    XFpga_top_WriteReg(InstancePtr->Control_BaseAddress, XFPGA_TOP_CONTROL_ADDR_SHARED_DRAM_DATA + 4, (uint32_t)(Data >> 32));
}

uint64_t XFpga_top_Get_SHARED_DRAM(XFpga_top *InstancePtr) {
    uint64_t Data;

    assert(InstancePtr != NULL);
    assert(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    Data = XFpga_top_ReadReg(InstancePtr->Control_BaseAddress, XFPGA_TOP_CONTROL_ADDR_SHARED_DRAM_DATA);
    Data += (uint64_t)XFpga_top_ReadReg(InstancePtr->Control_BaseAddress, XFPGA_TOP_CONTROL_ADDR_SHARED_DRAM_DATA + 4) << 32;
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

void XFpga_top_InterruptEnable(XFpga_top *InstancePtr, uint32_t Mask) {
    uint32_t Register;

    assert(InstancePtr != NULL);
    assert(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    Register =  XFpga_top_ReadReg(InstancePtr->Axilite_BaseAddress, XFPGA_TOP_AXILITE_ADDR_IER);
    XFpga_top_WriteReg(InstancePtr->Axilite_BaseAddress, XFPGA_TOP_AXILITE_ADDR_IER, Register | Mask);
}

void XFpga_top_InterruptDisable(XFpga_top *InstancePtr, uint32_t Mask) {
    uint32_t Register;

    assert(InstancePtr != NULL);
    assert(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    Register =  XFpga_top_ReadReg(InstancePtr->Axilite_BaseAddress, XFPGA_TOP_AXILITE_ADDR_IER);
    XFpga_top_WriteReg(InstancePtr->Axilite_BaseAddress, XFPGA_TOP_AXILITE_ADDR_IER, Register & (~Mask));
}

void XFpga_top_InterruptClear(XFpga_top *InstancePtr, uint32_t Mask) {
    assert(InstancePtr != NULL);
    assert(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    XFpga_top_WriteReg(InstancePtr->Axilite_BaseAddress, XFPGA_TOP_AXILITE_ADDR_ISR, Mask);
}

uint32_t XFpga_top_InterruptGetEnabled(XFpga_top *InstancePtr) {
    assert(InstancePtr != NULL);
    assert(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    return XFpga_top_ReadReg(InstancePtr->Axilite_BaseAddress, XFPGA_TOP_AXILITE_ADDR_IER);
}

uint32_t XFpga_top_InterruptGetStatus(XFpga_top *InstancePtr) {
    assert(InstancePtr != NULL);
    assert(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    return XFpga_top_ReadReg(InstancePtr->Axilite_BaseAddress, XFPGA_TOP_AXILITE_ADDR_ISR);
}


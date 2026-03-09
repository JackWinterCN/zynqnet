// ==============================================================
// Vitis HLS - High-Level Synthesis from C, C++ and OpenCL v2020.1 (64-bit)
// Copyright 1986-2020 Xilinx, Inc. All Rights Reserved.
// ==============================================================
// axilite
// 0x00 : Control signals
//        bit 0  - ap_start (Read/Write/COH)
//        bit 1  - ap_done (Read/COR)
//        bit 2  - ap_idle (Read)
//        bit 3  - ap_ready (Read)
//        bit 7  - auto_restart (Read/Write)
//        others - reserved
// 0x04 : Global Interrupt Enable Register
//        bit 0  - Global Interrupt Enable (Read/Write)
//        others - reserved
// 0x08 : IP Interrupt Enable Register (Read/Write)
//        bit 0  - enable ap_done interrupt (Read/Write)
//        bit 1  - enable ap_ready interrupt (Read/Write)
//        others - reserved
// 0x0c : IP Interrupt Status Register (Read/TOW)
//        bit 0  - ap_done (COR/TOW)
//        bit 1  - ap_ready (COR/TOW)
//        others - reserved
// 0x10 : Data signal of layer
//        bit 31~0 - layer[31:0] (Read/Write)
// 0x14 : Data signal of layer
//        bit 31~0 - layer[63:32] (Read/Write)
// 0x18 : Data signal of layer
//        bit 31~0 - layer[95:64] (Read/Write)
// 0x1c : Data signal of layer
//        bit 31~0 - layer[127:96] (Read/Write)
// 0x20 : Data signal of layer
//        bit 31~0 - layer[159:128] (Read/Write)
// 0x24 : Data signal of layer
//        bit 31~0 - layer[191:160] (Read/Write)
// 0x28 : Data signal of layer
//        bit 31~0 - layer[223:192] (Read/Write)
// 0x2c : Data signal of layer
//        bit 31~0 - layer[255:224] (Read/Write)
// 0x30 : Data signal of layer
//        bit 31~0 - layer[287:256] (Read/Write)
// 0x34 : Data signal of layer
//        bit 31~0 - layer[319:288] (Read/Write)
// 0x38 : reserved
// 0x3c : Data signal of weights_offset
//        bit 31~0 - weights_offset[31:0] (Read/Write)
// 0x40 : reserved
// 0x44 : Data signal of num_weights
//        bit 31~0 - num_weights[31:0] (Read/Write)
// 0x48 : reserved
// 0x4c : Data signal of input_offset
//        bit 31~0 - input_offset[31:0] (Read/Write)
// 0x50 : reserved
// (SC = Self Clear, COR = Clear on Read, TOW = Toggle on Write, COH = Clear on Handshake)

#define XFPGA_TOP_AXILITE_ADDR_AP_CTRL             0x00
#define XFPGA_TOP_AXILITE_ADDR_GIE                 0x04
#define XFPGA_TOP_AXILITE_ADDR_IER                 0x08
#define XFPGA_TOP_AXILITE_ADDR_ISR                 0x0c
#define XFPGA_TOP_AXILITE_ADDR_LAYER_DATA          0x10
#define XFPGA_TOP_AXILITE_BITS_LAYER_DATA          320
#define XFPGA_TOP_AXILITE_ADDR_WEIGHTS_OFFSET_DATA 0x3c
#define XFPGA_TOP_AXILITE_BITS_WEIGHTS_OFFSET_DATA 32
#define XFPGA_TOP_AXILITE_ADDR_NUM_WEIGHTS_DATA    0x44
#define XFPGA_TOP_AXILITE_BITS_NUM_WEIGHTS_DATA    32
#define XFPGA_TOP_AXILITE_ADDR_INPUT_OFFSET_DATA   0x4c
#define XFPGA_TOP_AXILITE_BITS_INPUT_OFFSET_DATA   32

// control
// 0x00 : reserved
// 0x04 : reserved
// 0x08 : reserved
// 0x0c : reserved
// 0x10 : Data signal of SHARED_DRAM
//        bit 31~0 - SHARED_DRAM[31:0] (Read/Write)
// 0x14 : Data signal of SHARED_DRAM
//        bit 31~0 - SHARED_DRAM[63:32] (Read/Write)
// 0x18 : reserved
// (SC = Self Clear, COR = Clear on Read, TOW = Toggle on Write, COH = Clear on Handshake)

#define XFPGA_TOP_CONTROL_ADDR_SHARED_DRAM_DATA 0x10
#define XFPGA_TOP_CONTROL_BITS_SHARED_DRAM_DATA 64


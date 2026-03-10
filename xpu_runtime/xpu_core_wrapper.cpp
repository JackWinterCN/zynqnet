//------------------------------------------------------------------------------
//  SqueezeNetOnFPGA
//------------------------------------------------------------------------------
//
//   File:  xpu_core_wrapper.cpp
//
//  CPU-Side Functions for SqueezeNetOnFPGA
//
//   (c) David Gschwend, 2016
//
//------------------------------------------------------------------------------

#include "xpu_core_wrapper.hpp"
#include "xparameters.h"
// ====================
// = Global Variables =
// ====================
// Pointers to Shared DRAM Memory
volatile char *SHARED_DRAM;
// layer_t *SHARED_DRAM_LAYER_CONFIG;
volatile data_t *SHARED_DRAM_WEIGHTS;
volatile data_t *SHARED_DRAM_DATA;

bool USE_FPGA_BLOCK = false;
bool BE_QUIET = false;

// XFpga_top_Config XFpga_top_ConfigTable[XPAR_XFPGA_TOP_NUM_INSTANCES];
XFpga_top g_fpga_top;
/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////



std::vector<float> nchw_to_nhwc(const std::vector<float> &nchw_data, int N,
                                int C, int H, int W) {
  // 1. 输入合法性校验
  const int total_elements = N * C * H * W;
  if (nchw_data.size() != total_elements) {
    std::cerr << "data size Not match, expect: " << total_elements
              << ", actual: " << nchw_data.size() << std::endl;
    return {};
  }

  std::vector<float> nhwc_data(total_elements, 0.0f);

  // 3. 核心转换逻辑：反向索引映射
  for (int n = 0; n < N; ++n) {       // 遍历批次
    for (int c = 0; c < C; ++c) {     // 遍历通道
      for (int h = 0; h < H; ++h) {   // 遍历高度
        for (int w = 0; w < W; ++w) { // 遍历宽度
          // NCHW索引公式：n*C*H*W + c*H*W + h*W + w
          int nchw_idx = n * C * H * W + c * H * W + h * W + w;
          // NHWC索引公式：n*H*W*C + h*W*C + w*C + c
          int nhwc_idx = n * H * W * C + h * W * C + w * C + c;
          // 赋值
          nhwc_data[nhwc_idx] = nchw_data[nchw_idx];
        }
      }
    }
  }
  return nhwc_data;
}

std::vector<float> oikk_to_iokk(const std::vector<float> &oikk_data, int OC,
                                int IC, int K) {
  // 1. 输入合法性校验：总元素数必须等于 OC*IC*K*K
  const int total_elements = OC * IC * K * K;
  if (oikk_data.size() != total_elements) {
    std::cerr << "data size Not match, expect: " << total_elements
              << ", actual: " << oikk_data.size() << std::endl;
    return {};
  }
  std::vector<float> iokk_data(total_elements, 0.0f);

  // 3. 核心转换逻辑：索引映射
  // 原格式索引：oc * (IC*K*K) + ic * (K*K) + kh * K + kw
  // 目标格式索引：ic * (OC*K*K) + oc * (K*K) + kh * K + kw
  for (int oc = 0; oc < OC; ++oc) {      // 遍历输出通道
    for (int ic = 0; ic < IC; ++ic) {    // 遍历输入通道
      for (int kh = 0; kh < K; ++kh) {   // 遍历卷积核行
        for (int kw = 0; kw < K; ++kw) { // 遍历卷积核列
          // 计算原格式索引
          int oikk_idx = oc * (IC * K * K) + ic * (K * K) + kh * K + kw;
          // 计算目标格式索引
          int iokk_idx = ic * (OC * K * K) + oc * (K * K) + kh * K + kw;
          // 赋值
          iokk_data[iokk_idx] = oikk_data[oikk_idx];
        }
      }
    }
  }
  return iokk_data;
}

std::vector<float> nhwc_to_nchw(const std::vector<float> &nhwc_data, int N,
                                int C, int H, int W) {
  // 1. 输入合法性校验
  const int total_elements = N * C * H * W;
  if (nhwc_data.size() != total_elements) {
    std::cerr << "data size Not match, expect: " << total_elements
              << ", actual: " << nhwc_data.size() << std::endl;
    return {};
  }
  std::vector<float> nchw_data(total_elements, 0.0f);
  // 3. 核心转换逻辑：遍历每个维度，计算索引映射
  for (int n = 0; n < N; ++n) {       // 遍历批次
    for (int h = 0; h < H; ++h) {     // 遍历高度
      for (int w = 0; w < W; ++w) {   // 遍历宽度
        for (int c = 0; c < C; ++c) { // 遍历通道
          // NHWC索引公式：n*H*W*C + h*W*C + w*C + c
          int nhwc_idx = n * H * W * C + h * W * C + w * C + c;
          // NCHW索引公式：n*C*H*W + c*H*W + h*W + w
          int nchw_idx = n * C * H * W + c * H * W + h * W + w;
          // 赋值
          nchw_data[nchw_idx] = nhwc_data[nhwc_idx];
        }
      }
    }
  }
  return nchw_data;
}

// =================
// = Main Function =
// =================
// static std::vector<float> conv2d(const std::vector<float> &input,
//                           const std::vector<float> &weight,
//                           const std::vector<float> &bias, int N, int inputH,
//                           int inputW, const Convolution2DCommonT &params)
std::vector<float> xpu_core_run(const std::vector<float> &input,
                                const std::vector<float> &weight,
                                const std::vector<float> &bias, int N,
                                int inputH, int inputW,
                                const XPUCoreConvParams &params,
                                bool simulation_mode, bool quiet) {
  // Parse Arguments:
  if (simulation_mode) {
    USE_FPGA_BLOCK = false;
  } else {
    USE_FPGA_BLOCK = true;
  }
  std::cout << "USE_FPGA_BLOCK: " << USE_FPGA_BLOCK << std::endl;

  if (quiet) {
      BE_QUIET = true;
  }

  LOG_LEVEL = 0;
  int res = 0;

   auto input_nhwc = nchw_to_nhwc(input, N, params.inputCount, inputH, inputW);
  // auto input_nhwc = input;

  auto weight_iokk = oikk_to_iokk(weight, params.outputCount, params.inputCount,
                                  params.kernelY);


  // Measure per-CNN Time:
  timeval t_net_start, t_net_end;
  double t_net_elapsed;
  gettimeofday(&t_net_start, NULL);

  // ========================
  // = Setup Network on CPU =
  // ========================

  network_t *net_CPU = new network_t(1, weight_iokk.size() + bias.size());
  // Layer Attributes: ( NAME   ,   W,   H,   CI,  CO, K, P, S, R, S1, S2, GP)
  addLayer(net_CPU, layer_t("cx    ", inputW, inputH, params.inputCount,
                            params.outputCount, params.kernelX, params.padX,
                            params.strideX, 0, 0, 0, 0));
  net_CPU->num_weights = weight_iokk.size() + bias.size();

  for (int i = 0; i < net_CPU->num_layers; i++) {
    layer_t *layer = &(net_CPU->layers[i]);
    int chout = layer->channels_out;
    int chin = layer->channels_in;
    int kernel = layer->kernel;

    // calculate address within weight memory section
    int num_weights = chout * chin * kernel * kernel + chout;
    float *weights_addr = (net_CPU->weights + layer->mem_addr_weights);
    // read portion of input file
    // fread(weights_addr, sizeof(data_t), num_weights, filehandle);
    memcpy(weights_addr, weight_iokk.data(),
           weight_iokk.size() * sizeof(float));
    memcpy(weights_addr + weight_iokk.size(), bias.data(),
           bias.size() * sizeof(float));
  }

  // ==========================
  // = Setup FPGA Accelerator =
  // ==========================
  // Initialize AXILITE Configuration Bus + Shared DRAM region
  if (USE_FPGA_BLOCK) {
    if(res = XFpga_top_Initialize(&g_fpga_top, "xpu_core")) {
      printf("[error] XFpga_top_Initialize Failed with error code %d\n", res);
      return std::vector<float>();
    }
  }
  // Allocate Shared Memory in DRAM for Weights + Data.
  allocate_DRAM_memory(net_CPU);

  // Calculate DRAM Address Offsets
  LOG("SHARED_DRAM is at address: %lu\n", (long)SHARED_DRAM);
  int weights_offset =
      ((long)SHARED_DRAM_WEIGHTS - (long)SHARED_DRAM) / sizeof(data_t);
  int input_offset =
      ((long)SHARED_DRAM_DATA - (long)SHARED_DRAM) / sizeof(data_t);
  // Copy Layer Weights to DRAM.
  copy_weights_to_DRAM(net_CPU);

  // ===========================
  // = Load + Copy Input Image =
  // ===========================
  layer_t input_layer = net_CPU->layers[0];
  // Allocate Memory for Input Image:
  data_t *input_image = allocate_image_memory(input_layer);
  // Load Input Image
  memcpy(input_image, input_nhwc.data(), input_nhwc.size() * sizeof(float));

  // Copy Input Image into shared DRAM
  copy_input_image_to_DRAM(input_layer, input_image);

  // Per-Layer Performance Timing
  timeval t_layer_start, t_layer_end;
  double t_layer_elapsed;
  // ===========================
  // = Loop through CNN Layers =
  // ===========================
  int num_layers = net_CPU->num_layers;
L_LAYERS:
  for (int id = 0; id < num_layers; id++) {
    // Fetch Layer Config
    layer_t layer = net_CPU->layers[id];

    if (!BE_QUIET) {
      // Logging
      LOG("Layer %2d: <%s>\n", id, layer.name);
      printf("CPU: Offload CONV Layer ");
      print_layer(&layer);
      fflush(stdout);
      LOG_LEVEL_INCR;
      // Per-Layer Performance Timing
      gettimeofday(&t_layer_start, NULL);
    }

    // ============================
    // = Execute FPGA Accelerator =
    // ============================
    if (USE_FPGA_BLOCK) {
      volatile uint32_t *SHARED_DRAM = XFPGA_shared_DRAM_physical();
      uint64_t SHARED_DRAM_64b = (uint64_t)SHARED_DRAM;
      // make sure DRAM address really fits into 32bits (no clipping)
      assert(SHARED_DRAM_64b - (uint32_t)SHARED_DRAM_64b == 0);
      // set DRAM address via AXILITE BUS
      XFpga_top_Set_SHARED_DRAM(&g_fpga_top, SHARED_DRAM_64b); // physical
      // XFPGA_setWeightsOffset(weights_offset);
      XFpga_top_Set_weights_offset(&g_fpga_top, weights_offset);
      uint32_t weights_per_filter = (layer.kernel == 3) ? 9 : 1;
      uint32_t num_weights =
          layer.channels_in * layer.channels_out * weights_per_filter;
      XFpga_top_Set_num_weights(&g_fpga_top, num_weights);
      // XFPGA_setInputOffset(input_offset);
      XFpga_top_Set_input_offset(&g_fpga_top, input_offset);

      // XFPGA_setLayerConfig(layer);
      union XPUCoreParamUnion param_temp;
      param_temp.layer_c = layer;
      XFpga_top_Set_layer(&g_fpga_top, param_temp.layer_pack);
      // XFPGA_Start();
      XFpga_top_Start(&g_fpga_top);
      while (!XFpga_top_IsDone(&g_fpga_top)) { // busy-wait
        // sleep(1);            // sleep 100us
        if (!BE_QUIET)
          LOG("XFPGA Status: Done = %d, Idle = %d, Ready = %d\n", XFpga_top_IsDone(&g_fpga_top),
          XFpga_top_IsIdle(&g_fpga_top), XFpga_top_IsReady(&g_fpga_top));
      }
    } else {  
      // CPU Simulation
      // Precalculate some Layer Parameters on CPU
      numfilterelems_t weights_per_filter = (layer.kernel == 3) ? 9 : 1;
      weightaddr_t num_weights =
          layer.channels_in * layer.channels_out * weights_per_filter;
      fpga_top(layer, (data_t *)SHARED_DRAM, weights_offset, num_weights,
               input_offset);
    }

    if (!BE_QUIET) {
      gettimeofday(&t_layer_end, NULL);
      t_layer_elapsed = (t_layer_end.tv_sec - t_layer_start.tv_sec) * 1000;
      t_layer_elapsed += (t_layer_end.tv_usec - t_layer_start.tv_usec) / 1000;
      printf("run time: %0.0fms\n", t_layer_elapsed);
      fflush(stdout);
      LOG_LEVEL_DECR;
    }
  }  // layer loop
  LOG_LEVEL = 0;

  // ======================================
  // = Copy Results back from SHARED DRAM =
  // ======================================
  // Verify that last layer reduces spatial dimensions to 1x1:
  layer_t *final = &net_CPU->layers[net_CPU->num_layers - 1];
 int outputH =
      (inputH + 2 * params.padY - params.kernelY) / params.strideY + 1;
  int outputW =
      (inputW + 2 * params.padX - params.kernelX) / params.strideX + 1;
  int output_size = outputH * outputW * params.outputCount;
  std::vector<float> output_nhwc(output_size, 0);
  // Copy Result Data:
  memcpy(output_nhwc.data(), (data_t *)SHARED_DRAM_DATA + final->mem_addr_output,
         output_size * sizeof(float));
  // ========================
  // = Performance / Timing =
  // ========================
  gettimeofday(&t_net_end, NULL);
  t_net_elapsed = (t_net_end.tv_sec - t_net_start.tv_sec) * 1000;
  t_net_elapsed += (t_net_end.tv_usec - t_net_start.tv_usec) / 1000;
  printf("\nTotal run time: %0.0fms\n", t_net_elapsed);
  fflush(stdout);

  // =====================
  // = Release FPGA core =
  // =====================
  if (USE_FPGA_BLOCK) {
    XFpga_top_Release(&g_fpga_top);
  }

  auto output_nchw =
      nhwc_to_nchw(output_nhwc, N, params.outputCount, outputH, outputW);

  return output_nchw;
}

// =======================================
// = Allocate CPU Memory for Input Image =
// =======================================
data_t *allocate_image_memory(layer_t &layer) {
  int win = layer.width;
  int hin = layer.height;
  int chin = layer.channels_in;
  return (data_t *)malloc(win * hin * chin * sizeof(data_t));
}

// ==========================================
// = Allocate CPU Memory for Data + Weights =
// ==========================================
// Reserve Memory and Assign Pointers
// DRAM:
//      ______________
//     |    weights   |  0
//     |              |  ...
//     |______________|  weightsize - 1
//     |     data     |  weightsize
//     |  in + output |  ...
//     |______________|  weightsize + datasize - 1
//
void allocate_DRAM_memory(network_t *net_CPU) {
  // For Simulation purposes, allocate space on Heap
  // For actual HW Implementation, fixed Memory Address in SHARED DRAM is used

  // Memory Requirements (Bytes)
  int weightsize = net_CPU->num_weights * sizeof(data_t);
  int datasize = net_CPU->total_pixel_mem * sizeof(data_t);

  // Round memory areas to 32-bit boundaries (4 bytes)
  weightsize = std::ceil(weightsize / 4.0) * 4;
  datasize = std::ceil(datasize / 4.0) * 4;

  int total_size = weightsize + datasize;

  // Memory Allocation
  if (USE_FPGA_BLOCK) {
    // Get Pointer to SHARED DRAM from XFPGA wrapper
    SHARED_DRAM = (volatile char *)XFPGA_shared_DRAM_virtual();
    // SHARED_DRAM = (volatile char *)malloc(total_size);
  } else {
    // Allocate SHARED DRAM on Heap
    SHARED_DRAM = (volatile char *)malloc(total_size);
  }

  SHARED_DRAM_WEIGHTS = (volatile data_t *)(SHARED_DRAM);
  SHARED_DRAM_DATA = (volatile data_t *)(SHARED_DRAM + weightsize);

  // Debug: Infos about Memory Regions
  printf("CPU: FPGA DRAM Memory Allocation:\n");
  printf("     Bytes allocated: %dB (config) + %dB/%dKB (weights) + %dB/%dKB (data)\n",
         0, weightsize, weightsize / 1024, datasize, datasize / 1024);
  printf("     region: %lu - %lu\n", (long)SHARED_DRAM,
         (long)(SHARED_DRAM + total_size));

  // Check that DRAM_DEPTH constant is correct for VHLS Co-Simulation
  int num_mem_elements = total_size / sizeof(data_t);
  if (DRAM_DEPTH != num_mem_elements) {
    printf("\n\n!! ATTENTION !!\n");
    printf("Please set DRAM_DEPTH = %d in network.hpp\n\n", num_mem_elements);
    // exit(-1);
  }
}

// =====================================================
// = Copy Weights to FPGA (shared DRAM) =
// =====================================================
void copy_weights_to_DRAM(network_t *net_CPU) {
  int weightsize = net_CPU->num_weights * sizeof(data_t);
  // Info:
  printf("CPU: Copy Weights: %dB/%dKB (weights) to FPGA DRAM\n", weightsize,
         weightsize / 1024);
  // Copy Weights:
  memcpy((void *)SHARED_DRAM_WEIGHTS, net_CPU->weights, weightsize);
}

// ==========================================
// = Copy Input Image to FPGA (shared DRAM) =
// ==========================================
void copy_input_image_to_DRAM(layer_t &layer, data_t *image) {
  // Calculate size of input data
  int win = layer.width;
  int hin = layer.height;
  int chin = layer.channels_in;
  int num_pixels = win * hin * chin;
  int input_size = num_pixels * sizeof(data_t);
  printf("CPU: Copy Input Image (%dB)/(%dKB)\n", input_size, input_size / 1024);

  // Copy Input Data:
  memcpy((void *)SHARED_DRAM_DATA, image, input_size);
}

// =============================================
// = Copy Results back from FPGA (shared DRAM) =
// =============================================
// Assumption: Last Layer reduces data to dimensions 1x1xch_out (global pool)
// Assumption: Output Data is written back to where initial image was placed
// data_t *results: Pointer to data_t array with enough space to hold results
void copy_results_from_DRAM(data_t *results, int ch_out) {
  // Output Data is put at beginning of INPUT_DATA section in shared DRAM
  int result_offset = 0;
  int result_size = ch_out * sizeof(data_t);
  printf("CPU: Copy Results from FPGA DRAM (%d Bytes)\n", result_size);

  // Copy Result Data:
  memcpy(results, (void *)(SHARED_DRAM_DATA + result_offset), result_size);
}

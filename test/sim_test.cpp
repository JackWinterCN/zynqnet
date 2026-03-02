#include <cmath>
#include <iomanip>
#include <iostream>
#include <vector>

// 卷积参数结构体：集中管理所有配置，提升代码可读性
struct ConvParams {
  int padX = 2;                         // X方向（宽度）填充
  int padY = 2;                         // Y方向（高度）填充
  int kernelX = 3;                      // 卷积核宽度
  int kernelY = 3;                      // 卷积核高度
  int strideX = 2;                      // X方向步长
  int strideY = 2;                      // Y方向步长
  int dilateX = 1;                      // X方向膨胀率
  int dilateY = 1;                      // Y方向膨胀率
  int group = 1;                        // 分组数
  int inputCount = 3;                   // 输入通道数
  int outputCount = 4;                  // 输出通道数
  bool relu = false;                    // 是否使用ReLU激活
  bool relu6 = false;                   // 是否使用ReLU6激活
  std::vector<int> pads = {2, 2, 2, 2}; // 填充细节 [左, 上, 右, 下]
};

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

std::vector<float> iokk_to_oikk(const std::vector<float> &iokk_data, int IC,
                                int OC, int K) {
  // 1. 输入合法性校验
  const int total_elements = IC * OC * K * K;
  if (iokk_data.size() != total_elements) {
    std::cerr << "data size Not match, expect: " << total_elements
              << ", actual: " << iokk_data.size() << std::endl;
    return {};
  }
  std::vector<float> oikk_data(total_elements, 0.0f);

  // 3. 核心转换逻辑：反向索引映射
  // 原格式索引：ic * (OC*K*K) + oc * (K*K) + kh * K + kw
  // 目标格式索引：oc * (IC*K*K) + ic * (K*K) + kh * K + kw
  for (int ic = 0; ic < IC; ++ic) {      // 遍历输入通道
    for (int oc = 0; oc < OC; ++oc) {    // 遍历输出通道
      for (int kh = 0; kh < K; ++kh) {   // 遍历卷积核行
        for (int kw = 0; kw < K; ++kw) { // 遍历卷积核列
          // 计算原格式索引
          int iokk_idx = ic * (OC * K * K) + oc * (K * K) + kh * K + kw;
          // 计算目标格式索引
          int oikk_idx = oc * (IC * K * K) + ic * (K * K) + kh * K + kw;
          // 赋值
          oikk_data[oikk_idx] = iokk_data[iokk_idx];
        }
      }
    }
  }
  return oikk_data;
}

// 计算输出特征图的高度和宽度
// inputH: 输入特征图高度, inputW: 输入特征图宽度, params: 卷积参数
std::pair<int, int> calculate_output_size(int inputH, int inputW,
                                          const ConvParams &params) {
  // 核心公式：OH = ((IH + 2*padY - kernelY) / strideY) + 1
  int outputH =
      (inputH + 2 * params.padY - params.kernelY) / params.strideY + 1;
  // 核心公式：OW = ((IW + 2*padX - kernelX) / strideX) + 1
  int outputW =
      (inputW + 2 * params.padX - params.kernelX) / params.strideX + 1;
  return {outputH, outputW};
}

// 对输入特征图进行零填充（CAFFE模式默认零填充）
// input: 输入特征图 (N, C, H, W)，返回填充后的特征图 (N, C, H+2*padY, W+2*padX)
std::vector<float> pad_input_nchw(const std::vector<float> &input, int N, int C,
                                  int H, int W, const ConvParams &params) {
  int padLeft = params.pads[0];
  int padTop = params.pads[1];
  int padRight = params.pads[2];
  int padBottom = params.pads[3];

  // 填充后的高度和宽度
  int paddedH = H + padTop + padBottom;
  int paddedW = W + padLeft + padRight;

  // 初始化填充后的特征图，默认值为0（零填充）
  std::vector<float> paddedInput(N * C * paddedH * paddedW, 0.0f);

  // 将原始输入拷贝到填充后的中心位置
  for (int n = 0; n < N; ++n) {       // 遍历批次
    for (int c = 0; c < C; ++c) {     // 遍历通道
      for (int h = 0; h < H; ++h) {   // 遍历高度
        for (int w = 0; w < W; ++w) { // 遍历宽度
          // 原始输入的一维索引
          int inputIdx = n * C * H * W + c * H * W + h * W + w;
          // 填充后对应的索引（偏移padTop和padLeft）
          int paddedIdx = n * C * paddedH * paddedW + c * paddedH * paddedW +
                          (h + padTop) * paddedW + (w + padLeft);
          paddedInput[paddedIdx] = input[inputIdx];
        }
      }
    }
  }

  return paddedInput;
}

std::vector<float> pad_input_nhwc(const std::vector<float> &input, int N,
                                  int inputC, int inputH, int inputW,
                                  const ConvParams &params) {
  int paddedH = inputH + 2 * params.padY;
  int paddedW = inputW + 2 * params.padX;
  // 初始化填充后的输入（NHWC格式，初始值0）
  std::vector<float> paddedInput(N * paddedH * paddedW * inputC, 0.0f);

  // 将原始输入拷贝到填充后的对应位置（NHWC索引）
  for (int n = 0; n < N; ++n) {
    for (int h = 0; h < inputH; ++h) {
      for (int w = 0; w < inputW; ++w) {
        for (int c = 0; c < inputC; ++c) {
          // 原始输入（NHWC）索引
          int inputIdx = n * inputH * inputW * inputC + h * inputW * inputC +
                         w * inputC + c;
          // 填充后输入（NHWC）索引
          int paddedIdx = n * paddedH * paddedW * inputC +
                          (h + params.padY) * paddedW * inputC +
                          (w + params.padX) * inputC + c;
          paddedInput[paddedIdx] = input[inputIdx];
        }
      }
    }
  }
  return paddedInput;
}

// 卷积算子核心实现
// input: 输入特征图 (N, inputC, H, W)
// weight: 卷积核 (outputC, inputC/group, kernelY, kernelX)
// bias: 偏置 (outputC)，为空则不使用偏置
// N: 批次大小, inputH/inputW: 输入特征图高/宽
std::vector<float> conv2d_nchw_oikk(const std::vector<float> &input,
                                    const std::vector<float> &weight,
                                    const std::vector<float> &bias, int N,
                                    int inputH, int inputW,
                                    const ConvParams &params) {
  // 提取核心参数
  int inputC = params.inputCount;
  int outputC = params.outputCount;
  int kernelY = params.kernelY;
  int kernelX = params.kernelX;
  int strideY = params.strideY;
  int strideX = params.strideX;
  int group = params.group;

  // 基础校验：输入/输出通道数需能被分组数整除
  if (inputC % group != 0 || outputC % group != 0) {
    std::cerr << "[error] inputC/outputC must be divisible by group count! "
              << "inputC =" << inputC << ", outputC=" << outputC
              << ", group=" << group << std::endl;
    return {};
  }
  int groupInputC = inputC / group;   // 每个分组的输入通道数
  int groupOutputC = outputC / group; // 每个分组的输出通道数

  // 计算输出特征图尺寸
  auto outputSize = calculate_output_size(inputH, inputW, params);
  int outputH = outputSize.first;
  int outputW = outputSize.second;
  std::cout << "outputH = " << outputH << ", outputW = " << outputW
            << std::endl;

  // 对输入进行零填充
  std::vector<float> paddedInput =
      pad_input_nchw(input, N, inputC, inputH, inputW, params);
  int paddedH = inputH + params.padY * 2;
  int paddedW = inputW + params.padX * 2;

  // 初始化输出特征图（默认值0）
  std::vector<float> output(N * outputC * outputH * outputW, 0.0f);

  // 核心卷积计算逻辑
  for (int n = 0; n < N; ++n) {            // 遍历批次
    for (int oc = 0; oc < outputC; ++oc) { // 遍历输出通道
      int g = oc / groupOutputC; // 当前输出通道所属分组（group=1时恒为0）
      for (int oh = 0; oh < outputH; ++oh) {   // 遍历输出高度
        for (int ow = 0; ow < outputW; ++ow) { // 遍历输出宽度
          float sum = 0.0f;                    // 卷积累加和

          // 遍历当前分组内的输入通道
          for (int ic = 0; ic < groupInputC; ++ic) {
            int realIC = g * groupInputC + ic; // 实际输入通道索引

            // 遍历卷积核
            for (int ky = 0; ky < kernelY; ++ky) {   // 核高度
              for (int kx = 0; kx < kernelX; ++kx) { // 核宽度
                // 计算填充后输入的对应位置
                int ih = oh * strideY + ky; // 输入高度位置
                int iw = ow * strideX + kx; // 输入宽度位置

                // 边界检查（填充后可省略，保留更健壮）
                if (ih < 0 || ih >= paddedH || iw < 0 || iw >= paddedW) {
                  continue;
                }

                // 计算输入特征图的一维索引
                int inputIdx = n * inputC * paddedH * paddedW +
                               realIC * paddedH * paddedW + ih * paddedW + iw;

                // 计算卷积核的一维索引
                int weightIdx = oc * groupInputC * kernelY * kernelX +
                                ic * kernelY * kernelX + ky * kernelX + kx;

                // 累加：输入值 × 权重值
                sum += paddedInput[inputIdx] * weight[weightIdx];
              }
            }
          }

          // 加上偏置（如果有）
          if (!bias.empty()) {
            sum += bias[oc];
          }

          // 激活函数（当前配置无激活，直接赋值）
          if (params.relu)
            sum = std::max(0.0f, sum);
          if (params.relu6)
            sum = std::min(std::max(0.0f, sum), 6.0f);

          // 赋值到输出特征图
          int outputIdx = n * outputC * outputH * outputW +
                          oc * outputH * outputW + oh * outputW + ow;
          output[outputIdx] = sum;
        }
      }
    }
  }

  return output;
}

std::vector<float> conv2d_nhwc_iokk(const std::vector<float> &input,
                                    const std::vector<float> &weight,
                                    const std::vector<float> &bias, int N,
                                    int inputH, int inputW,
                                    const ConvParams &params) {
  // 提取核心参数
  int inputC = params.inputCount;
  int outputC = params.outputCount;
  int kernelY = params.kernelY;
  int kernelX = params.kernelX;
  int strideY = params.strideY;
  int strideX = params.strideX;
  int group = params.group;

  // 基础校验：输入/输出通道数需能被分组数整除
  if (inputC % group != 0 || outputC % group != 0) {
    std::cerr << "[error] inputC/outputC must be divisible by group count! "
              << "inputC =" << inputC << ", outputC=" << outputC
              << ", group=" << group << std::endl;
    return {};
  }
  int groupInputC = inputC / group;   // 每个分组的输入通道数
  int groupOutputC = outputC / group; // 每个分组的输出通道数

  // 计算输出特征图尺寸
  auto outputSize = calculate_output_size(inputH, inputW, params);
  int outputH = outputSize.first;
  int outputW = outputSize.second;
  std::cout << "outputH = " << outputH << ", outputW = " << outputW
            << std::endl;

  // 对输入进行零填充（NHWC格式）
  std::vector<float> paddedInput =
      pad_input_nhwc(input, N, inputC, inputH, inputW, params);
  int paddedH = inputH + params.padY * 2;
  int paddedW = inputW + params.padX * 2;

  // 初始化输出特征图（NHWC格式，总元素数不变）
  std::vector<float> output(N * outputH * outputW * outputC, 0.0f);

  // 核心卷积计算逻辑
  for (int n = 0; n < N; ++n) {            // 遍历批次
    for (int oc = 0; oc < outputC; ++oc) { // 遍历输出通道
      int g = oc / groupOutputC; // 当前输出通道所属分组（group=1时恒为0）
      for (int oh = 0; oh < outputH; ++oh) {   // 遍历输出高度
        for (int ow = 0; ow < outputW; ++ow) { // 遍历输出宽度
          float sum = 0.0f;                    // 卷积累加和

          // 遍历当前分组内的输入通道
          for (int ic = 0; ic < groupInputC; ++ic) {
            int realIC = g * groupInputC + ic; // 实际输入通道索引

            // 遍历卷积核
            for (int ky = 0; ky < kernelY; ++ky) {   // 核高度
              for (int kx = 0; kx < kernelX; ++kx) { // 核宽度
                // 计算填充后输入的对应位置
                int ih = oh * strideY + ky; // 输入高度位置
                int iw = ow * strideX + kx; // 输入宽度位置

                // 边界检查
                if (ih < 0 || ih >= paddedH || iw < 0 || iw >= paddedW) {
                  continue;
                }

                // 输入特征图索引（NHWC格式）
                int inputIdx = n * paddedH * paddedW * inputC +
                               ih * paddedW * inputC + iw * inputC + realIC;

                // 权重索引（inputC/group, outputC, kY, kX格式）
                int weightIdx = ic * outputC * kernelY * kernelX +
                                oc * kernelY * kernelX + ky * kernelX + kx;

                // 累加：输入值 × 权重值
                sum += paddedInput[inputIdx] * weight[weightIdx];
              }
            }
          }

          // 加上偏置（如果有）
          if (!bias.empty()) {
            sum += bias[oc];
          }

          // 激活函数
          if (params.relu)
            sum = std::max(0.0f, sum);
          if (params.relu6)
            sum = std::min(std::max(0.0f, sum), 6.0f);

          // 【核心修改：输出索引改为NHWC格式】
          // 原NCHW索引：n*outputC*outputH*outputW + oc*outputH*outputW +
          // oh*outputW + ow 新NHWC索引：n*outputH*outputW*outputC +
          // oh*outputW*outputC + ow*outputC + oc
          int outputIdx = n * outputH * outputW * outputC +
                          oh * outputW * outputC + ow * outputC + oc;
          output[outputIdx] = sum;
        }
      }
    }
  }

  return output;
}

// 测试函数：验证卷积算子功能
int main() {
  // 初始化卷积参数（固定为用户指定配置）
  ConvParams params;

  // 输入特征图参数：批次=1，输入高=10，输入宽=10（可自行修改）
  int N = 1;
  int inputH = 10;
  int inputW = 10;

  // 生成测试数据：输入和权重全为1（方便验证计算结果）
  std::vector<float> input(N * params.inputCount * inputH * inputW, 1.0f);
  for (int i = 0; i < input.size(); i++) {
    // input[i] = i % 16;
    input[i] = i % 10;
  }

  std::vector<float> weight(params.outputCount *
                                (params.inputCount / params.group) *
                                params.kernelY * params.kernelX,
                            1.0f);
  for (int i = 0; i < weight.size(); i++) {
    // weight[i] = (i % 10) * 0.1;
    weight[i] = 0.1;
  }
  std::vector<float> bias(params.outputCount, 0.0f);
  for (int i = 0; i < bias.size(); i++) {
    // bias[i] = i % 4;
    bias[i] = i % 4;
  }

  {
    std::cout << "---------- NCHW ---------" << std::endl;
    std::vector<float> output =
        conv2d_nchw_oikk(input, weight, bias, N, inputH, inputW, params);

    // 输出结果信息
    int outputH =
        (inputH + 2 * params.padY - params.kernelY) / params.strideY + 1;
    int outputW =
        (inputW + 2 * params.padX - params.kernelX) / params.strideX + 1;
    std::cout << "output dim: " << N << "x" << params.outputCount << "x"
              << outputH << "x" << outputW << std::endl;
    std::cout << "output size: " << output.size() << std::endl;

    for (int n = 0; n < N; n++) {
      std::cout << "{" << std::endl;
      for (int oc = 0; oc < params.outputCount; oc++) {
        std::cout << "{" << std::endl;
        for (int h = 0; h < outputH; h++) {
          std::cout << "    ";
          for (int w = 0; w < outputW; w++) {
            int out_idx = n * params.outputCount * outputH * outputW +
                          oc * outputH * outputW + h * outputW + w;
            std::cout << std::setw(5) << output[out_idx] << "  ";
          }
          std::cout << std::endl;
        }
        std::cout << "}" << std::endl;
      }
      std::cout << "}" << std::endl;
    }
  }

  {
    std::cout << "---------- NHWC ---------" << std::endl;
    auto input_nhwc = nchw_to_nhwc(input, N, params.inputCount, inputH, inputW);
    auto weight_iokk = oikk_to_iokk(weight, params.outputCount,
                                    params.inputCount, params.kernelY);
    std::vector<float> output_nhwc = conv2d_nhwc_iokk(
        input_nhwc, weight_iokk, bias, N, inputH, inputW, params);

    int outputH =
        (inputH + 2 * params.padY - params.kernelY) / params.strideY + 1;
    int outputW =
        (inputW + 2 * params.padX - params.kernelX) / params.strideX + 1;

    auto output_nchw =
        nhwc_to_nchw(output_nhwc, N, params.outputCount, outputH, outputW);
    // 输出结果信息
    std::cout << "output dim: " << N << "x" << params.outputCount << "x"
              << outputH << "x" << outputW << std::endl;
    std::cout << "output size: " << output_nchw.size() << std::endl;

    for (int n = 0; n < N; n++) {
      std::cout << "{" << std::endl;
      for (int oc = 0; oc < params.outputCount; oc++) {
        std::cout << "{" << std::endl;
        for (int h = 0; h < outputH; h++) {
          std::cout << "    ";
          for (int w = 0; w < outputW; w++) {
            int out_idx = n * params.outputCount * outputH * outputW +
                          oc * outputH * outputW + h * outputW + w;
            std::cout << std::setw(5) << output_nchw[out_idx] << "  ";
          }
          std::cout << std::endl;
        }
        std::cout << "}" << std::endl;
      }
      std::cout << "}" << std::endl;
    }
  }

  return 0;
}
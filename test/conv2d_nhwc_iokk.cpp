#include <iostream>
#include <vector>
#include <algorithm> // 用于relu/relu6的max/min
#include <utility>   // 用于std::pair

// 补充ConvParams结构体定义（确保代码可独立编译）
struct ConvParams {
    int inputCount;    // 输入通道数（inputC）
    int outputCount;   // 输出通道数（outputC）
    int kernelY;       // 卷积核高度
    int kernelX;       // 卷积核宽度
    int strideY;       // 步长Y
    int strideX;       // 步长X
    int padY;          // 上下填充数
    int padX;          // 左右填充数
    int group;         // 分组数
    bool relu;         // 是否启用ReLU激活
    bool relu6;        // 是否启用ReLU6激活
};

// 补充calculateOutputSize函数（计算输出特征图尺寸，核心逻辑不变）
std::pair<int, int> calculateOutputSize(int inputH, int inputW, const ConvParams& params) {
    int outputH = (inputH + 2 * params.padY - params.kernelY) / params.strideY + 1;
    int outputW = (inputW + 2 * params.padX - params.kernelX) / params.strideX + 1;
    return {outputH, outputW};
}

// 【适配NHWC格式】输入填充函数
std::vector<float> padInput(const std::vector<float>& input, int N, int inputC, int inputH, int inputW, const ConvParams& params) {
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
                    int inputIdx = n * inputH * inputW * inputC + h * inputW * inputC + w * inputC + c;
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

// 【最终版】卷积算子：输入NHWC、权重(inputC/group, outputC, kY, kX)、输出NHWC
// input: 输入特征图 (N, H, W, inputC) 【NHWC】
// weight: 卷积核 (inputC/group, outputC, kernelY, kernelX)
// bias: 偏置 (outputC)
// N: 批次大小, inputH/inputW: 输入特征图高/宽
std::vector<float> conv2d_nhwc_iokk(const std::vector<float> &input,
                          const std::vector<float> &weight,
                          const std::vector<float> &bias, int N, int inputH,
                          int inputW, const ConvParams &params) {
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
  auto outputSize = calculateOutputSize(inputH, inputW, params);
  int outputH = outputSize.first;
  int outputW = outputSize.second;
  std::cout << "输出特征图尺寸：高=" << outputH << ", 宽=" << outputW
            << std::endl;

  // 对输入进行零填充（NHWC格式）
  std::vector<float> paddedInput =
      padInput(input, N, inputC, inputH, inputW, params);
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
                               ih * paddedW * inputC + 
                               iw * inputC + realIC;

                // 权重索引（inputC/group, outputC, kY, kX格式）
                int weightIdx = ic * outputC * kernelY * kernelX +
                                oc * kernelY * kernelX + 
                                ky * kernelX + kx;

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
          // 原NCHW索引：n*outputC*outputH*outputW + oc*outputH*outputW + oh*outputW + ow
          // 新NHWC索引：n*outputH*outputW*outputC + oh*outputW*outputC + ow*outputC + oc
          int outputIdx = n * outputH * outputW * outputC +
                          oh * outputW * outputC + 
                          ow * outputC + oc;
          output[outputIdx] = sum;
        }
      }
    }
  }

  return output;
}

// 测试用例（验证输出NHWC格式）
int main() {
    // 构造测试参数
    ConvParams params;
    params.inputCount = 2;    // inputC=2
    params.outputCount = 4;   // outputC=4
    params.kernelY = 2;       // 2x2卷积核
    params.kernelX = 2;
    params.strideY = 1;
    params.strideX = 1;
    params.padY = 0;          // 无填充
    params.padX = 0;
    params.group = 1;         // 分组数=2（每组inputC=1，outputC=2）
    params.relu = false;
    params.relu6 = false;

    // 构造NHWC格式输入（N=1, H=2, W=2, inputC=2）
    // 结构：n0,h0,w0,c0; n0,h0,w0,c1; n0,h0,w1,c0; n0,h0,w1,c1;
    //       n0,h1,w0,c0; n0,h1,w0,c1; n0,h1,w1,c0; n0,h1,w1,c1;
    std::vector<float> input_nhwc = {
        1.0f, 2.0f, 3.0f, 4.0f,
        5.0f, 6.0f, 7.0f, 8.0f
    };

    // 构造权重（inputC/group=1, outputC=4, kY=2, kX=2）
    std::vector<float> weight_icoc_kk = {
        1.0f, 1.0f, 1.0f, 1.0f, // ic0, oc0
        1.0f, 1.0f, 1.0f, 1.0f, // ic0, oc1
        1.0f, 1.0f, 1.0f, 1.0f, // ic0, oc2
        1.0f, 1.0f, 1.0f, 1.0f,  // ic0, oc3
        1.0f, 1.0f, 1.0f, 1.0f, // ic0, oc0
        1.0f, 1.0f, 1.0f, 1.0f, // ic0, oc1
        1.0f, 1.0f, 1.0f, 1.0f, // ic0, oc2
        1.0f, 1.0f, 1.0f, 1.0f  // ic0, oc3
    };

    // 偏置（outputC=4）
    std::vector<float> bias = {0.0f, 0.0f, 0.0f, 0.0f};

    // 执行卷积
    std::vector<float> output_nhwc = conv2d_nhwc_iokk(
        input_nhwc, weight_icoc_kk, bias, 1, 2, 2, params
    );

    // 输出结果（NHWC格式）
    std::cout << "卷积输出（NHWC格式）：";
    for (float val : output_nhwc) {
        std::cout << val << " ";
    }
    std::cout << std::endl;

    // 预期输出解释：
    // 输出尺寸：N=1, H=1, W=1, outputC=4（因为2x2输入+2x2核+步长1+无填充 → 输出1x1）
    // NHWC结构：n0,h0,w0,oc0; n0,h0,w0,oc1; n0,h0,w0,oc2; n0,h0,w0,oc3
    // 计算值：每个输出通道的和为 (1+2+5+6)=14（oc0/oc1）、(3+4+7+8)=22（oc2/oc3）
    // 最终输出：14 14 22 22
    return 0;
}
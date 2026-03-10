
#include "xpu_core_wrapper.hpp"


int main(int argc, char* argv[]) {
  bool run_siml = true;
  if (argc > 1) {
    run_siml = !(strcmp(argv[1], "0") == 0);
  }
  std::cout << "run_siml: " << run_siml << std::endl;
  LOG_LEVEL = 0;

  XPUCoreConvParams params;
  int N = 1;
  int inputH = 10;
  int inputW = 10;
  std::vector<float> input(N * params.inputCount * inputH * inputW, 1.0f);
  for (int i = 0; i < input.size(); i++) {
    input[i] = i % 10;
    // input[i] = i;
  }

  std::vector<float> weight(params.outputCount *
                                (params.inputCount / params.group) *
                                params.kernelY * params.kernelX,
                            1.0f);
  for (int i = 0; i < weight.size(); i++) {
    // weight[i] = (i % 10) * 0.1;
    // weight[i] = i * 0.1;
    weight[i] = 1;
  }
  std::vector<float> bias(params.outputCount, 0.0f);
  for (int i = 0; i < bias.size(); i++) {
    // bias[i] = i % 4;
    bias[i] = i % 4;
  }

  std::cout << "---------- NHWC ---------" << std::endl;
 

  auto output_nchw = xpu_core_run(input, weight, bias, N, inputH,
                                  inputW, params, run_siml, false);
  int outputH =
      (inputH + 2 * params.padY - params.kernelY) / params.strideY + 1;
  int outputW =
      (inputW + 2 * params.padX - params.kernelX) / params.strideX + 1;

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
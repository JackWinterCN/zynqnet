#include <iostream>
#include <vector>
#include <stdexcept>  // 用于异常处理

/**
 * @brief 将NHWC格式的float数组转换为NCHW格式
 * @param nhwc_data 输入：NHWC格式的vector<float>
 * @param N 批次大小
 * @param C 通道数
 * @param H 高度
 * @param W 宽度
 * @return NCHW格式的vector<float>
 */
std::vector<float> nhwc_to_nchw(const std::vector<float>& nhwc_data, int N, int C, int H, int W) {
    // 1. 输入合法性校验
    const int total_elements = N * C * H * W;
    if (nhwc_data.size() != total_elements) {
        throw std::invalid_argument(
            "NHWC数据长度不匹配！期望：" + std::to_string(total_elements) + 
            " 实际：" + std::to_string(nhwc_data.size())
        );
    }

    // 2. 初始化输出容器
    std::vector<float> nchw_data(total_elements, 0.0f);

    // 3. 核心转换逻辑：遍历每个维度，计算索引映射
    for (int n = 0; n < N; ++n) {       // 遍历批次
        for (int h = 0; h < H; ++h) {   // 遍历高度
            for (int w = 0; w < W; ++w) {   // 遍历宽度
                for (int c = 0; c < C; ++c) {   // 遍历通道
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

// 测试用例
int main() {
    try {
        // 测试参数：N=1, C=2, H=2, W=2（小维度方便手动验证）
        const int N = 1, C = 2, H = 2, W = 2;
        // 构造NHWC测试数据：按 N→H→W→C 顺序排列
        // 结构：(n0,h0,w0,c0), (n0,h0,w0,c1), (n0,h0,w1,c0), (n0,h0,w1,c1),
        //       (n0,h1,w0,c0), (n0,h1,w0,c1), (n0,h1,w1,c0), (n0,h1,w1,c1)
        std::vector<float> nhwc_data = {0.0f, 1.0f, 2.0f, 3.0f, 
                                        4.0f, 5.0f, 6.0f, 7.0f};

        // 转换
        std::vector<float> nchw_data = nhwc_to_nchw(nhwc_data, N, C, H, W);

        // 输出结果
        std::cout << "=== NHWC转NCHW 测试结果 ===" << std::endl;
        std::cout << "原始NHWC数据：";
        for (float val : nhwc_data) std::cout << val << " ";
        std::cout << "\n转换后NCHW数据：";
        for (float val : nchw_data) std::cout << val << " ";
        std::cout << std::endl;

        // 预期结果：[0,2,4,6, 1,3,5,7]（N0,C0所有H/W → N0,C1所有H/W）
    } catch (const std::exception& e) {
        std::cerr << "错误：" << e.what() << std::endl;
        return 1;
    }

    return 0;
}
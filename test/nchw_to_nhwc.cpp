#include <iostream>
#include <vector>
#include <stdexcept>  // 用于异常处理

/**
 * @brief 将NCHW格式的float数组转换为NHWC格式
 * @param nchw_data 输入：NCHW格式的vector<float>
 * @param N 批次大小
 * @param C 通道数
 * @param H 高度
 * @param W 宽度
 * @return NHWC格式的vector<float>
 */
std::vector<float> nchw_to_nhwc(const std::vector<float>& nchw_data, int N, int C, int H, int W) {
    // 1. 输入合法性校验
    const int total_elements = N * C * H * W;
    if (nchw_data.size() != total_elements) {
        throw std::invalid_argument(
            "NCHW数据长度不匹配！期望：" + std::to_string(total_elements) + 
            " 实际：" + std::to_string(nchw_data.size())
        );
    }

    // 2. 初始化输出容器
    std::vector<float> nhwc_data(total_elements, 0.0f);

    // 3. 核心转换逻辑：反向索引映射
    for (int n = 0; n < N; ++n) {       // 遍历批次
        for (int c = 0; c < C; ++c) {   // 遍历通道
            for (int h = 0; h < H; ++h) {   // 遍历高度
                for (int w = 0; w < W; ++w) {   // 遍历宽度
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

// 测试用例
int main() {
    try {
        // 测试参数：N=1, C=2, H=2, W=2（与上一个程序反向验证）
        const int N = 1, C = 2, H = 2, W = 2;
        // 构造NCHW测试数据（对应上一个程序的输出）
        std::vector<float> nchw_data = {0.0f, 2.0f, 4.0f, 6.0f, 
                                        1.0f, 3.0f, 5.0f, 7.0f};

        // 转换
        std::vector<float> nhwc_data = nchw_to_nhwc(nchw_data, N, C, H, W);

        // 输出结果
        std::cout << "=== NCHW转NHWC 测试结果 ===" << std::endl;
        std::cout << "原始NCHW数据：";
        for (float val : nchw_data) std::cout << val << " ";
        std::cout << "\n转换后NHWC数据：";
        for (float val : nhwc_data) std::cout << val << " ";
        std::cout << std::endl;

        // 预期结果：[0,1,2,3,4,5,6,7]（还原为原始NHWC）
    } catch (const std::exception& e) {
        std::cerr << "错误：" << e.what() << std::endl;
        return 1;
    }

    return 0;
}
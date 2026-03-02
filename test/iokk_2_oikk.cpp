#include <iostream>
#include <vector>
#include <stdexcept>  // 异常处理

/**
 * @brief 将权重格式从 (IC, OC, K, K) 转换为 (OC, IC, K, K)
 * @param iokk_data 输入：(IC, OC, K, K) 格式的权重数据
 * @param IC 输入通道数
 * @param OC 输出通道数
 * @param K 卷积核尺寸（K×K）
 * @return (OC, IC, K, K) 格式的权重数据
 */
std::vector<float> iokk_to_oikk(const std::vector<float>& iokk_data, int IC, int OC, int K) {
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
    for (int ic = 0; ic < IC; ++ic) {       // 遍历输入通道
        for (int oc = 0; oc < OC; ++oc) {   // 遍历输出通道
            for (int kh = 0; kh < K; ++kh) { // 遍历卷积核行
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

// 测试用例（反向验证程序1的结果）
int main() {
    try {
        // 测试参数：IC=3、OC=2、K=2（与程序1反向）
        const int IC = 3, OC = 2, K = 2;
        // 构造(IC, OC, K, K)格式的测试数据（对应程序1的输出）
        std::vector<float> iokk_data = {
            // IC=0
            0.0f, 1.0f, 2.0f, 3.0f,   // OC=0, K×K
            12.0f, 13.0f, 14.0f, 15.0f,// OC=1, K×K
            // IC=1
            4.0f, 5.0f, 6.0f, 7.0f,   // OC=0, K×K
            16.0f, 17.0f, 18.0f, 19.0f,// OC=1, K×K
            // IC=2
            8.0f, 9.0f, 10.0f, 11.0f, // OC=0, K×K
            20.0f, 21.0f, 22.0f, 23.0f // OC=1, K×K
        };

        // 执行转换
        std::vector<float> oikk_data = iokk_to_oikk(iokk_data, IC, OC, K);

        // 输出结果
        std::cout << "=== (IC,OC,K,K) → (OC,IC,K,K) 测试结果 ===" << std::endl;
        std::cout << "原始数据：";
        for (float val : iokk_data) std::cout << val << " ";
        std::cout << "\n转换后数据：";
        for (float val : oikk_data) std::cout << val << " ";
        std::cout << std::endl;

        // 预期结果：还原为程序1的原始(OC,IC,K,K)数据
    } catch (const std::exception& e) {
        std::cerr << "错误：" << e.what() << std::endl;
        return 1;
    }

    return 0;
}
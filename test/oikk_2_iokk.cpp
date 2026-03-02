#include <iostream>
#include <vector>
#include <stdexcept>  // 异常处理，避免下标越界

/**
 * @brief 将权重格式从 (OC, IC, K, K) 转换为 (IC, OC, K, K)
 * @param oikk_data 输入：(OC, IC, K, K) 格式的权重数据
 * @param OC 输出通道数
 * @param IC 输入通道数
 * @param K 卷积核尺寸（K×K）
 * @return (IC, OC, K, K) 格式的权重数据
 */
std::vector<float> oikk_to_iokk(const std::vector<float>& oikk_data, int OC, int IC, int K) {
    // 1. 输入合法性校验：总元素数必须等于 OC*IC*K*K
    const int total_elements = OC * IC * K * K;
    if (oikk_data.size() != total_elements) {
      std::cerr << "data size Not match, expect: " << total_elements
                << ", actual: " << oikk_data.size() << std::endl;
      return {};
    }

    // 2. 初始化输出容器
    std::vector<float> iokk_data(total_elements, 0.0f);

    // 3. 核心转换逻辑：索引映射
    // 原格式索引：oc * (IC*K*K) + ic * (K*K) + kh * K + kw
    // 目标格式索引：ic * (OC*K*K) + oc * (K*K) + kh * K + kw
    for (int oc = 0; oc < OC; ++oc) {       // 遍历输出通道
        for (int ic = 0; ic < IC; ++ic) {   // 遍历输入通道
            for (int kh = 0; kh < K; ++kh) { // 遍历卷积核行
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

// 测试用例（小维度方便手动验证）
int main() {
    try {
        // 测试参数：OC=2（输出通道）、IC=3（输入通道）、K=2（2x2卷积核）
        const int OC = 2, IC = 3, K = 2;
        // 构造(OC, IC, K, K)格式的测试数据：
        // 结构：OC0→IC0(K0K0, K0K1, K1K0, K1K1) → IC1 → IC2 → OC1→IC0 → IC1 → IC2
        std::vector<float> oikk_data = {
            // OC=0
            0.0f, 1.0f, 2.0f, 3.0f,  // IC=0, K×K
            4.0f, 5.0f, 6.0f, 7.0f,  // IC=1, K×K
            8.0f, 9.0f, 10.0f, 11.0f,// IC=2, K×K
            // OC=1
            12.0f, 13.0f, 14.0f, 15.0f,// IC=0, K×K
            16.0f, 17.0f, 18.0f, 19.0f,// IC=1, K×K
            20.0f, 21.0f, 22.0f, 23.0f // IC=2, K×K
        };

        // 执行转换
        std::vector<float> iokk_data = oikk_to_iokk(oikk_data, OC, IC, K);

        // 输出结果
        std::cout << "=== (OC,IC,K,K) → (IC,OC,K,K) 测试结果 ===" << std::endl;
        std::cout << "原始数据：";
        for (float val : oikk_data) std::cout << val << " ";
        std::cout << "\n转换后数据：";
        for (float val : iokk_data) std::cout << val << " ";
        std::cout << std::endl;

        // 预期结果（IC维度优先）：
        // IC0→OC0(K×K) → OC1(K×K) → IC1→OC0 → OC1 → IC2→OC0 → OC1
        // 即：0,1,2,3, 12,13,14,15, 4,5,6,7, 16,17,18,19, 8,9,10,11, 20,21,22,23
    } catch (const std::exception& e) {
        std::cerr << "错误：" << e.what() << std::endl;
        return 1;
    }

    return 0;
}
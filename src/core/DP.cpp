#include <iostream>
#include <vector>
#include <numeric>
#include <limits>
#include <algorithm>
#include <cmath>
#include "RNG.h" 

using namespace SECYAN; 

double generateLaplaceNoise(double sensitivity, double epsilon) {
    if (epsilon <= 0) {
        return 0;
    }
    double lambda = sensitivity / epsilon;
    // 使用 SECYAN::gRNG 生成 [0, 1) 的均匀分布随机数
    // 由于 SECYAN::RNG 主要提供整数类型，我们需要转换
    uint32_t rand_int = gRNG.NextUInt32();
    double u = static_cast<double>(rand_int) / (std::numeric_limits<uint32_t>::max() + 1.0); // 归一化到 [0, 1)

    return -lambda * std::log(1.0 - u);
}

double calculateAverage(const std::vector<uint32_t>& data) {
    if (data.empty()) {
        return 0;
    }
    return std::accumulate(data.begin(), data.end(), 0.0) / data.size();
}

double differentiallyPrivateAverage(const std::vector<uint32_t>& data, double epsilon, double data_range) {
    if (data.empty()) {
        return 0;
    }
    double sensitivity = data_range / data.size(); // 简化敏感度计算
    double raw_average = calculateAverage(data);
    double noise = generateLaplaceNoise(sensitivity, epsilon);
    return raw_average + noise;
}

double calculateDataRange(const std::vector<uint32_t>& data) {
    if (data.empty()) {
        return 0;
    }
    double min_val = *std::min_element(data.begin(), data.end());
    double max_val = *std::max_element(data.begin(), data.end());
    return max_val - min_val; 
}

void testEpsilonValues(const std::vector<uint32_t>& data, double suggestedDataRange, const std::vector<double>& epsilonValues) {
    std::cout << "\n--- 测试不同 Epsilon 值 (使用 SECYAN::RNG) ---" << std::endl;
    std::cout << "建议的 data_range (基于数组范围): " << suggestedDataRange << std::endl;
    std::cout << "原始平均值: " << calculateAverage(data) << std::endl;

    for (double epsilon : epsilonValues) {
        std::cout << "\nEpsilon = " << epsilon << ":" << std::endl;
        for (int i = 0; i < 5; ++i) { 
            std::cout << "  差分隐私平均值 (运行 " << i + 1 << "): " << differentiallyPrivateAverage(data, epsilon, suggestedDataRange) << std::endl;
        }
    }
    std::cout << "--- 测试结束 ---" << std::endl;
}
/*
int main() {
    std::vector<double> income_data = {50000, 60000, 75000, 55000, 80000, 65000};

    double suggested_data_range = calculateDataRange(income_data);

    std::vector<double> epsilon_values_to_test = {0.1, 0.5, 1.0, 5.0, 10.0};

    testEpsilonValues(income_data, suggested_data_range, epsilon_values_to_test);

    return 0;
}*/
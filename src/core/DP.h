#pragma once 

#include <vector>
#include <cmath>

namespace SECYAN { 

    double generateLaplaceNoise(double sensitivity, double epsilon);
    double calculateAverage(const std::vector<uint32_t>& data);
    double differentiallyPrivateAverage(const std::vector<uint32_t>& data, double epsilon, double data_range);
    double calculateDataRange(const std::vector<uint32_t>& data);
    void testEpsilonValues(const std::vector<uint32_t>& data, double suggestedDataRange, const std::vector<double>& epsilonValues);

} // namespace SECYAN
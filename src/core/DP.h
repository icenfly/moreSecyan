#pragma once 

#include <vector>
#include <cmath>

namespace SECYAN { 

    double generateLaplaceNoise(double sensitivity, double epsilon);
    double calculateAverage(std::vector<uint32_t>& data);
    double differentiallyPrivateAverage(std::vector<uint32_t>& data, double epsilon, double data_range);
    double calculateDataRange(std::vector<uint32_t>& data);
    void testEpsilonValues(std::vector<uint32_t>& data, double suggestedDataRange, std::vector<double>& epsilonValues);

} // namespace SECYAN
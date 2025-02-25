#pragma once 

#include <vector>
#include <cmath>

namespace SECYAN { 

    double generateLaplaceNoise(double sensitivity, double epsilon);
    double calculateAverage(std::vector<int> data);
    double differentiallyPrivateAverage(std::vector<int> data, double epsilon, double data_range);
    double calculateDataRange(std::vector<int> data);
    void testEpsilonValues(std::vector<int> data, double suggestedDataRange, std::vector<double> epsilonValues);

} // namespace SECYAN
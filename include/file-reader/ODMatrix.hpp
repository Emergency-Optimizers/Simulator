/**
 * @file ODMatrix.hpp
 *
 * @copyright Copyright (c) 2024 Emergency-Optimizers
 */

#pragma once

/* external libraries */
#include <string>
#include <vector>
#include <unordered_map>
#include <random>

class ODMatrix {
 private:
    std::vector<std::vector<float>> matrix;
    std::unordered_map<int64_t, int> idToIndexMap;
    const double noiseMean = 1.0;
    const double noiseStddev = 0.10;
    const double noiseLower = 0.95;
    const double noiseUpper = 1.05;
    std::normal_distribution<> normalDist = std::normal_distribution<>(noiseMean, noiseStddev);

    ODMatrix();
    void loadFromFile(const std::string& filename);

 public:
    ODMatrix(const ODMatrix&) = delete;
    ODMatrix& operator=(const ODMatrix&) = delete;
    static ODMatrix& getInstance() {
        static ODMatrix instance;
        return instance;
    }
    int getTravelTime(
        std::mt19937& rnd,
        const int64_t& id1,
        const int64_t& id2,
        const bool forceTrafficFactor,
        const std::string& triage,
        const time_t& time
    );
    bool gridIdExists(const int64_t& id);
};

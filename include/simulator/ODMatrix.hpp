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

class ODMatrix {
 private:
    std::vector<std::vector<float>> matrix;
    std::unordered_map<int64_t, int> idToIndexMap;

 public:
    ODMatrix(const std::string& filename);
    void loadFromFile(const std::string& filename);
    int getTravelTime(const int64_t& id1, const int64_t& id2);
    bool gridIdExists(const int64_t& id);
};

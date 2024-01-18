/**
 * @file ODMatrix.hpp
 * @author Sindre Eiklid
 * @version 1.0
 * @date 2023-11-08
 *
 * @copyright Copyright (c) 2023 Sindre Eiklid
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
    ODMatrix();
    void loadFromFile(const std::string& filename);
    int getTravelTime(const int64_t& id1, const int64_t& id2);
};

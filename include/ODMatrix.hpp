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
    std::vector<std::vector<int>> matrix;
    std::unordered_map<int, int> idToIndexMap;

 public:
    ODMatrix();
    void loadFromFile(const std::string& filename);
    int getTravelTime(const int& id1, const int& id2);
};

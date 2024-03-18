/**
 * @file ODMatrix.cpp
 *
 * @copyright Copyright (c) 2024 Emergency-Optimizers
 */

/* external libraries */
#include <fstream>
#include <sstream>
#include <iostream>
/* internal libraries */
#include "simulator/ODMatrix.hpp"

ODMatrix::ODMatrix() {
    loadFromFile("../../Data-Processing/data/oslo/od_matrix.txt");
}

void ODMatrix::loadFromFile(const std::string& filename) {
    std::ifstream file(filename);
    if (!file.is_open()) {
        std::cerr << "Error opening file\n";
        return;
    }

    std::string line;
    // reading the first line for IDs
    if (std::getline(file, line)) {
        std::istringstream ss(line);
        std::string id;
        int index = 0;
        // split the line by commas and populate the idToIndexMap
        while (getline(ss, id, ',')) {
            idToIndexMap[std::stoll(id)] = index++;
        }
    }

    // initialize the matrix now that we know the size
    int size = idToIndexMap.size();
    matrix.resize(size, std::vector<float>(size));

    // read the matrix values
    int i = 0;
    while (std::getline(file, line)) {
        std::istringstream ss(line);
        std::string value;
        int j = 0;
        while (getline(ss, value, ',')) {
            matrix[i][j++] = std::stof(value);
        }
        i++;
    }

    file.close();
}

int ODMatrix::getTravelTime(const int64_t& id1, const int64_t& id2) {
    if (idToIndexMap.find(id1) == idToIndexMap.end() || idToIndexMap.find(id2) == idToIndexMap.end()) {
        std::cerr << "Invalid IDs\n";
        return 0;
    }
    return matrix[idToIndexMap[id1]][idToIndexMap[id2]] + 20;
}

bool ODMatrix::gridIdExists(const int64_t& id) {
    return idToIndexMap.find(id) != idToIndexMap.end();
}

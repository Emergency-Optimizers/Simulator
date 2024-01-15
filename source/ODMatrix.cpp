/**
 * @file ODMatrix.cpp
 * @author Sindre Eiklid
 * @version 1.0
 * @date 2023-11-08
 *
 * @copyright Copyright (c) 2023 Sindre Eiklid
 */

/* external libraries */
#include <fstream>
#include <sstream>
#include <iostream>
/* internal libraries */
#include "ODMatrix.hpp"

ODMatrix::ODMatrix() { }

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
            idToIndexMap[std::stoi(id)] = index++;
        }
    }

    // initialize the matrix now that we know the size
    int size = idToIndexMap.size();
    matrix.resize(size, std::vector<int>(size));

    // read the matrix values
    int i = 0;
    while (std::getline(file, line)) {
        std::istringstream ss(line);
        std::string value;
        int j = 0;
        while (getline(ss, value, ',')) {
            matrix[i][j++] = std::stoi(value);
        }
        i++;
    }

    file.close();
}

int ODMatrix::getTravelTime(const int& id1, const int& id2) {
    if (idToIndexMap.find(id1) == idToIndexMap.end() || idToIndexMap.find(id2) == idToIndexMap.end()) {
        /// TODO: FIX THIS TEMP FIX
        // std::cerr << "Invalid IDs\n";
        return 300;
    }
    return matrix[idToIndexMap[id1]][idToIndexMap[id2]];
}

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
#include "file-reader/ODMatrix.hpp"
#include "file-reader/Traffic.hpp"
#include "ProgressBar.hpp"

ODMatrix::ODMatrix() {
    loadFromFile("../../Data-Processing/data/oslo/od_matrix.txt");
}

void ODMatrix::loadFromFile(const std::string& filename) {
    std::ifstream file(filename);
    if (!file.is_open()) {
        std::cerr << "Error opening file\n";
        return;
    }

    // count total lines
    size_t totalLines = 0;
    std::string tempLine;
    while (std::getline(file, tempLine)) {
        totalLines++;
    }

    // reset file to beginning
    file.clear();
    file.seekg(0, std::ios::beg);

    // setup progressBar
    ProgressBar progressBar(totalLines, "Loading O/D matrix");

    std::string line;
    int linesRead = 0;

    // reading the first line for IDs
    if (std::getline(file, line)) {
        std::istringstream ss(line);
        std::string id;
        int index = 0;
        // split the line by commas and populate the idToIndexMap
        while (getline(ss, id, ',')) {
            idToIndexMap[std::stoll(id)] = index++;
        }

        progressBar.update(++linesRead);
    }

    // initialize the matrix now that we know the size
    int size = static_cast<int>(idToIndexMap.size());
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

        progressBar.update(++linesRead);
    }

    file.close();
}

int ODMatrix::getTravelTime(
    std::mt19937& rnd,
    const int64_t& id1,
    const int64_t& id2,
    const bool forceTrafficFactor,
    const std::string& triage,
    const time_t& time
) {
    if (idToIndexMap.find(id1) == idToIndexMap.end() || idToIndexMap.find(id2) == idToIndexMap.end()) {
        std::cerr << "Invalid IDs\n";
        return 0;
    }

    double travelTime = static_cast<double>(matrix[idToIndexMap[id1]][idToIndexMap[id2]]);

    // set to 60 seconds if no travel time is given
    if (travelTime == 0) {
        travelTime = 60.0;
    }

    // branch if triage is V1 or ambulance is driving to depot
    if (forceTrafficFactor || triage == "V1") {
        double trafficFactor = Traffic::getInstance().getTrafficFactor(time);
        travelTime *= trafficFactor;
    }

    // branch if triage is A and ambulance is not driving to depot
    if (!forceTrafficFactor && triage == "A") {
        double acuteFactor = 0.7953711902650347;
        travelTime *= acuteFactor;
    }

    // add noise
    double noise = normalDist(rnd);
    travelTime *= noise;

    return static_cast<int>(floor(travelTime));
}

bool ODMatrix::gridIdExists(const int64_t& id) {
    return idToIndexMap.find(id) != idToIndexMap.end();
}

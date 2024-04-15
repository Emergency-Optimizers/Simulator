/**
 * @file Incidents.hpp
 *
 * @copyright Copyright (c) 2024 Emergency-Optimizers
 */

#pragma once

/* external libraries */
#include <string>
#include <vector>
#include <map>
/* internal libraries */
#include "file-reader/CSVReader.hpp"

class Incidents : public CSVReader {
 private:
    Incidents();

 public:
    std::map<int64_t, bool> gridIdUrban;

    Incidents(const Incidents&) = delete;
    Incidents& operator=(const Incidents&) = delete;
    static Incidents& getInstance() {
        static Incidents instance;
        return instance;
    }
    float timeDifferenceBetweenHeaders(const std::string& header1, const std::string& header2, const int index);
    std::vector<int> rowsWithinTimeFrame(const int month, const int day, const int windowSize);
};

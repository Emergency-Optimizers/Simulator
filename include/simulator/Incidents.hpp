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
#include "simulator/CSVReader.hpp"

class Incidents : public CSVReader {
 public:
    std::map<int64_t, bool> gridIdUrban;
    Incidents();
    explicit Incidents(const std::string& filename);
    float timeDifferenceBetweenHeaders(const std::string& header1, const std::string& header2, unsigned index);
    Incidents rowsWithinTimeFrame(const int month, const int day, const unsigned windowSize);
};

/**
 * @file Incidents.hpp
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
/* internal libraries */
#include "simulator/CSVReader.hpp"

class Incidents : public CSVReader {
 public:
    Incidents();
    float timeDifferenceBetweenHeaders(const std::string& header1, const std::string& header2, unsigned index);
    Incidents rowsWithinTimeFrame(const int month, const int day, const unsigned windowSize, const bool dayTime);
};

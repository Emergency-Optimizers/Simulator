/**
 * @file Incidents.hpp
 * @author Sindre Eiklid
 * @version 1.0
 * @date 2023-11-08
 *
 * @copyright Copyright (c) 2023 Sindre Eiklid
 */

#pragma once

/* internal libraries */
#include "CSVReader.hpp"

class Incidents : public CSVReader {
 public:
    Incidents();
    float timeDifferenceBetweenHeaders(const std::string& header1, const std::string& header2, unsigned index);
};

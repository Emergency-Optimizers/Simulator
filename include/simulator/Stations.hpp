/**
 * @file Stations.hpp
 * @author Sindre Eiklid
 * @version 1.0
 * @date 2023-11-09
 *
 * @copyright Copyright (c) 2023 Sindre Eiklid
 */

#pragma once

/* external libraries */
#include <vector>
/* internal libraries */
#include "simulator/CSVReader.hpp"

class Stations : public CSVReader {
 public:
    Stations();
    std::vector<unsigned> getDepotIndices();
    std::vector<unsigned> getHospitalIndices();
};

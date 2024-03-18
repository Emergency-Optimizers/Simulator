/**
 * @file Stations.hpp
 *
 * @copyright Copyright (c) 2024 Emergency-Optimizers
 */

#pragma once

/* external libraries */
#include <vector>
/* internal libraries */
#include "simulator/CSVReader.hpp"

class Stations : public CSVReader {
 public:
    Stations();
    explicit Stations(const std::string& filename);
    std::vector<unsigned> getDepotIndices();
    std::vector<unsigned> getHospitalIndices();
};

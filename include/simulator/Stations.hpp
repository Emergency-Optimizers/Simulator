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
 private:
    Stations();

 public:
    Stations(const Stations&) = delete;
    Stations& operator=(const Stations&) = delete;
    static Stations& getInstance() {
        static Stations instance;
        return instance;
    }
    std::vector<unsigned> getDepotIndices(const bool useExtraDepots);
    std::vector<unsigned> getHospitalIndices();
};

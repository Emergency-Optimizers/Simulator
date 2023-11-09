/**
 * @file AmbulanceAllocator.hpp
 * @author Sindre Eiklid
 * @version 1.0
 * @date 2023-11-09
 *
 * @copyright Copyright (c) 2023 Sindre Eiklid
 */

#pragma once

/* external libraries */
#include <fstream>
#include <string>
#include <vector>
/* internal libraries */
#include "AmbulanceAllocatorStrategy.hpp"
#include "Ambulance.hpp"

class AmbulanceAllocator {
 private:
    AmbulanceAllocatorStrategy strategy;

 public:
    std::vector<Ambulance> ambulances;
    void loadDepotsFromFile(const std::string& filename);
    void allocate(AmbulanceAllocatorStrategy strategy, int totalAmbulances);
};

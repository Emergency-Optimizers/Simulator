/**
 * @file AmbulanceAllocator.hpp
 *
 * @copyright Copyright (c) 2024 Emergency-Optimizers
 */

#pragma once

/* external libraries */
#include <vector>
/* internal libraries */
#include "simulator/Stations.hpp"
#include "simulator/Ambulance.hpp"

class AmbulanceAllocator {
 public:
    std::vector<Ambulance> ambulances;
    explicit AmbulanceAllocator();
    void allocate(const std::vector<int>& totalAllocatedAmbulancesAtDepots, const bool useExtraDepots);
};

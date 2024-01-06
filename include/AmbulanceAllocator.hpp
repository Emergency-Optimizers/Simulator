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
#include <vector>
/* internal libraries */
#include "Stations.hpp"
#include "Ambulance.hpp"

class AmbulanceAllocator {
 private:
    Stations& stations;

 public:
    std::vector<Ambulance> ambulances;
    AmbulanceAllocator(Stations& stations);
    void allocate(const std::vector<int>& totalAllocatedAmbulancesAtDepots);
};

/**
 * @file AmbulanceAllocator.cpp
 *
 * @copyright Copyright (c) 2024 Emergency-Optimizers
 */

/* internal libraries */
#include "simulator/AmbulanceAllocator.hpp"
#include "simulator/Ambulance.hpp"

AmbulanceAllocator::AmbulanceAllocator(Stations& stations) : stations(stations) {}

void AmbulanceAllocator::allocate(const std::vector<int>& totalAllocatedAmbulancesAtDepots) {
    ambulances.clear();

    std::vector<unsigned> depotIndices = stations.getDepotIndices();

    int ambulanceId = 0;
    for (int depotId = 0; depotId < totalAllocatedAmbulancesAtDepots.size(); depotId++) {
        int depotIndex = depotIndices[depotId];
        int numberOfAmbulancesInDepot = totalAllocatedAmbulancesAtDepots[depotId];
        const int64_t depotGridId = stations.get<int64_t>("grid_id", depotIndex);

        for (int i = 0; i < numberOfAmbulancesInDepot; i++) {
            Ambulance ambulance;
            ambulance.id = ambulanceId++;
            ambulance.allocatedDepotIndex = depotIndex;
            ambulance.currentGridId = depotGridId;

            ambulances.push_back(ambulance);
        }
    }
}

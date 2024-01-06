/**
 * @file AmbulanceAllocator.cpp
 * @author Sindre Eiklid
 * @version 1.0
 * @date 2024-01-05
 *
 * @copyright Copyright (c) 2023 Sindre Eiklid
 */

/* internal libraries */
#include "AmbulanceAllocator.hpp"
#include "Ambulance.hpp"

AmbulanceAllocator::AmbulanceAllocator(Stations& stations) : stations(stations) {}

void AmbulanceAllocator::allocate(const std::vector<int>& totalAllocatedAmbulancesAtDepots) {
    ambulances.clear();

    int ambulanceId = 0;
    for (int depotId = 0; depotId < totalAllocatedAmbulancesAtDepots.size(); depotId++) {
        int numberOfAmbulancesInDepot = totalAllocatedAmbulancesAtDepots[depotId];
        const int depotGridId = stations.get<int>("grid_id", depotId);

        for (int i = 0; i < numberOfAmbulancesInDepot; i++) {
            Ambulance ambulance;
            ambulance.id = ambulanceId++;
            ambulance.allocatedDepotId = depotId;
            ambulance.currentGridId = depotGridId;
            ambulance.targetGridId = -1;
            ambulance.currentStatus = EventType::NONE;

            ambulances.push_back(ambulance);
        }
    }
}

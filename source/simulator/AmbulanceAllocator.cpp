/**
 * @file AmbulanceAllocator.cpp
 *
 * @copyright Copyright (c) 2024 Emergency-Optimizers
 */

/* external libraries */
#include <map>
/* internal libraries */
#include "simulator/AmbulanceAllocator.hpp"
#include "file-reader/Settings.hpp"
#include "file-reader/Stations.hpp"

void AmbulanceAllocator::allocate(
    const std::vector<Event>& events,
    const std::vector<int>& totalAllocatedAmbulancesAtDepots,
    const bool dayshift
) {
    ambulances.clear();

    std::vector<unsigned> depotIndices = Stations::getInstance().getDepotIndices(dayshift);

    int ambulanceId = 0;
    for (int depotId = 0; depotId < totalAllocatedAmbulancesAtDepots.size(); depotId++) {
        int depotIndex = depotIndices[depotId];
        int numberOfAmbulancesInDepot = totalAllocatedAmbulancesAtDepots[depotId];
        const int64_t depotGridId = Stations::getInstance().get<int64_t>("grid_id", depotIndex);

        for (int i = 0; i < numberOfAmbulancesInDepot; i++) {
            Ambulance ambulance;
            ambulance.id = ambulanceId++;
            ambulance.allocatedDepotIndex = depotIndex;
            ambulance.currentGridId = depotGridId;

            ambulances.push_back(ambulance);
        }
    }

    std::tm shiftStartTm = *std::localtime(&events[0].timer);

    shiftStartTm.tm_hour = Settings::get<int>("DAY_SHIFT_START");
    shiftStartTm.tm_min = 0;
    shiftStartTm.tm_sec = 0;

    int shiftLengthHours = (12 * 60) * 60;

    time_t shiftStart = std::mktime(&shiftStartTm);
    time_t shiftEnd = shiftStart + shiftLengthHours;

    if (!dayshift) {
        shiftStart -= shiftLengthHours;
        shiftEnd -= shiftLengthHours;
    }

    allocateAndScheduleBreaks(shiftStart, shiftEnd);
}

void AmbulanceAllocator::allocateAndScheduleBreaks(const time_t& shiftStart, const time_t& shiftEnd) {
    std::map<int, int> depotAmbulanceCounts;
    std::map<int, int> depotAmbulanceOrdered;

    for (Ambulance& ambulance : ambulances) {
        depotAmbulanceCounts[ambulance.allocatedDepotIndex]++;
    }

    for (Ambulance& ambulance : ambulances) {
        int depotIndex = ambulance.allocatedDepotIndex;
        int ambulanceIndexWithinDepot = depotAmbulanceOrdered[depotIndex]++;
        int depotSize = depotAmbulanceCounts[depotIndex];

        ambulance.scheduleBreaks(shiftStart, shiftEnd, depotSize, ambulanceIndexWithinDepot);

        /*std::cout << "Ambulance ID: " << ambulance.id << ", Depot Index: " << depotIndex
                    << ", Ambulance Index Within Depot: " << ambulanceIndexWithinDepot
                    << ", Depot Size: " << depotSize << std::endl;

        for (time_t breakTime : ambulance.scheduledBreaks) {
            std::tm* ptm = std::localtime(&breakTime);
            std::cout << "Scheduled Break: " << std::put_time(ptm, "%Y-%m-%d %H:%M:%S") << std::endl;
        }*/
    }
}

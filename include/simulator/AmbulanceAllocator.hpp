/**
 * @file AmbulanceAllocator.hpp
 *
 * @copyright Copyright (c) 2024 Emergency-Optimizers
 */

#pragma once

/* external libraries */
#include <vector>
/* internal libraries */
#include "simulator/Ambulance.hpp"
#include "simulator/Event.hpp"

class AmbulanceAllocator {
 private:
    void allocateAndScheduleBreaks(const time_t& shiftStart, const time_t& shiftEnd);

 public:
    std::vector<Ambulance> ambulances;

    void allocate(
        const std::vector<Event>& events,
        const std::vector<int>& totalAllocatedAmbulancesAtDepots,
        const bool dayshift
    );
};

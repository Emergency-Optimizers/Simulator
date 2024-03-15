/**
 * @file Ambulance.hpp
 *
 * @copyright Copyright (c) 2024 Emergency-Optimizers
 */

#pragma once

/* internal libraries */
#include "simulator/EventType.hpp"

struct Ambulance {
    int id = -1;
    int allocatedDepotIndex = -1;
    int64_t currentGridId = -1;
    int assignedEventIndex = -1;
    int timeUnavailable = 0;
};

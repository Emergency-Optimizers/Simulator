/**
 * @file Ambulance.hpp
 *
 * @copyright Copyright (c) 2024 Emergency-Optimizers
 */

#pragma once

/* internal libraries */
#include "simulator/EventType.hpp"

struct Ambulance {
    int id;
    int allocatedDepotIndex;
    int64_t currentGridId;
    int assignedEventIndex;
};

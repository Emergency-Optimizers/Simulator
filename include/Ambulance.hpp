/**
 * @file Ambulance.hpp
 * @author Sindre Eiklid
 * @version 1.0
 * @date 2023-11-09
 *
 * @copyright Copyright (c) 2023 Sindre Eiklid
 */

#pragma once

/* internal libraries */
#include "EventType.hpp"

struct Ambulance {
    int id;
    int allocatedDepotIndex;
    int currentGridId;
    int assignedEventIndex;
};

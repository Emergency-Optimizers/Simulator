/**
 * @file Utils.hpp
 * @author Sindre Eiklid
 * @version 1.0
 * @date 2023-11-28
 *
 * @copyright Copyright (c) 2023 Sindre Eiklid
 */

/* external libraries */
#include <iostream>
#include <ctime>
/* internal libraries */
#include "IncidentStatus.hpp"

#pragma once

struct Event {
    int incidentIndex;
    IncidentStatus status;
    int assignedAmbulanceIndex;
    float timeSeconds;
};

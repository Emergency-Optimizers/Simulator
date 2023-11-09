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
#include "IncidentStatus.hpp"

struct Ambulance {
    int id;
    int allocatedGridId;
    int currentGridId;
    int targetGridId;
    IncidentStatus currentStatus;
    IncidentStatus nextStatus;
};

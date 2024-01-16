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
#include "EventType.hpp"
#include "EventPerformanceMetrics.hpp"

#pragma once

struct Event {
    EventType type;
    std::time_t timer;
    int incidentIndex;
    int assignedAmbulanceIndex;
    int64_t targetGridId;
    EventPerformanceMetrics metrics;

    void print() const {
        std::cout << "Event Type: " << static_cast<int>(type) << std::endl;
        std::cout << "Timer: " << std::asctime(std::localtime(&timer));
        std::cout << "Incident Index: " << incidentIndex << std::endl;
        std::cout << "Assigned Ambulance Index: " << assignedAmbulanceIndex << std::endl;
        std::cout << "Target Grid ID: " << targetGridId << std::endl;
    }
};

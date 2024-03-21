/**
 * @file Ambulance.hpp
 *
 * @copyright Copyright (c) 2024 Emergency-Optimizers
 */

#pragma once

/* external libraries */
#include <vector>
/* internal libraries */
#include "simulator/Event.hpp"
#include "simulator/EventType.hpp"

struct Ambulance {
    int id = -1;
    int allocatedDepotIndex = -1;
    int64_t currentGridId = -1;
    int assignedEventId = -1;
    int timeUnavailable = 0;
    int timeNotWorking = 0;
    time_t timeBreakStarted = 0;
    int breakLenght = 0;

    void setBreak(const int newBreakLength, const time_t& currentTime) {
        timeBreakStarted = currentTime;
        breakLenght = newBreakLength;
    }

    bool isAvailable(
        const std::vector<Event>& events,
        const int eventIndex,
        const time_t& currentTime
    ) {
        // if the ambulance is on a break and it is not finished, mark it as unavailable
        if (breakLenght != 0) {
            if (currentTime >= timeBreakStarted + breakLenght) {
                timeNotWorking += currentTime - timeBreakStarted;
                timeBreakStarted = 0;
                breakLenght = 0;

                return true;
            }
        }
        if (assignedEventId == -1) {
            return true;
        }
        if (events[eventIndex].type == EventType::FINISHED) {
            return true;
        }
        return false;
    }
};

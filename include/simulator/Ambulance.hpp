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
    std::vector<time_t> scheduledBreaks;

    void checkScheduledBreak(const time_t& currentTime) {
        if (!scheduledBreaks.empty() && currentTime >= scheduledBreaks.front()) {
            int breakLength = 30 * 60;

            setBreak(breakLength, currentTime);

            scheduledBreaks.erase(scheduledBreaks.begin());
        }
    }

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
        if (breakLenght == 0) {
            checkScheduledBreak(currentTime);
        }
        if (breakLenght != 0) {
            if (currentTime >= timeBreakStarted + breakLenght) {
                timeNotWorking += currentTime - timeBreakStarted;
                timeBreakStarted = 0;
                breakLenght = 0;

                return true;
            } else {
                return false;
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

    void scheduleBreaks(
        const time_t& shiftStart,
        const time_t& shiftEnd,
        const int depotSize,
        const int depotNum
    ) {
        time_t shiftLength = shiftEnd - shiftStart;
        // constraints: at least 1 hour after shift starts and 1 hour before shift ends. minimum 4 hours between each break
        time_t firstHourEnd = shiftStart + 3600;
        time_t lastHourStart = shiftEnd - 3600;
        time_t minBreakInterval = 14400;

        time_t break1Start = firstHourEnd + (depotNum % depotSize) * (minBreakInterval / depotSize);
        if (break1Start > lastHourStart - 1800) {
            break1Start = firstHourEnd;
        }

        time_t break2Start = break1Start + minBreakInterval;

        if (break2Start + 1800 > lastHourStart) {
            break2Start = lastHourStart - 1800;
        }

        scheduledBreaks.push_back(break1Start);
        scheduledBreaks.push_back(break2Start);
    }
};

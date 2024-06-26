/**
 * @file Ambulance.hpp
 *
 * @copyright Copyright (c) 2024 Emergency-Optimizers
 */

#pragma once

/* external libraries */
#include <vector>
#include <string>
/* internal libraries */
#include "simulator/Event.hpp"

struct Ambulance {
    int id = -1;
    int allocatedDepotIndex = -1;
    int64_t currentGridId = -1LL;
    int assignedEventId = -1;
    int timeUnavailable = 0;
    int timeNotWorking = 0;
    time_t timeBreakStarted = 0;
    int breakLength = 0;
    std::vector<time_t> scheduledBreaks;
    int currentAllocationIndex = 0;

    void checkScheduledBreak(const time_t& currentTime);
    void setBreak(const int newBreakLength, const time_t& currentTime);
    bool isAvailable(
        const std::vector<Event>& events,
        const std::vector<Ambulance>& ambulances,
        const int eventIndex,
        const time_t& currentTime,
        const std::string& currentEventTriageImpression
    );
    void scheduleBreaks(
        const time_t& shiftStart,
        const time_t& shiftEnd,
        const int depotSize,
        const int depotNum
    );
    bool higherTriagePriority(const std::string& triage, const std::string& triageToCompare);
};

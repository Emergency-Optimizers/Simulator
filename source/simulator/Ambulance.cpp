/**
 * @file Ambulance.cpp
 *
 * @copyright Copyright (c) 2024 Emergency-Optimizers
 */

/* internal libraries */
#include "simulator/Ambulance.hpp"
#include "file-reader/Settings.hpp"

void Ambulance::checkScheduledBreak(const time_t& currentTime) {
    if (!scheduledBreaks.empty() && currentTime >= scheduledBreaks.front()) {
        // start break (making it temporarly unavailable) and remove it from scheduled breaks
        const int MIN_30 = 30 * 60;

        setBreak(MIN_30, currentTime);

        scheduledBreaks.erase(scheduledBreaks.begin());
    }
}

void Ambulance::setBreak(const int newBreakLength, const time_t& currentTime) {
    timeBreakStarted = currentTime;
    breakLength = newBreakLength;
}

bool Ambulance::isAvailable(
    const std::vector<Event>& events,
    const std::vector<Ambulance>& ambulances,
    const int eventIndex,
    const time_t& currentTime,
    const std::string& currentEventTriageImpression
) {
    // check if ambulance should start break
    if (breakLength == 0) {
        if (assignedEventId == -1) {
            checkScheduledBreak(currentTime);
        } else if (!scheduledBreaks.empty() && currentTime >= scheduledBreaks.front()) {
            return false;
        }
    }

    // if the ambulance is on a break and it is not finished, mark it as unavailable
    if (breakLength != 0) {
        if (currentTime >= timeBreakStarted + breakLength) {
            timeNotWorking += static_cast<int>(currentTime - timeBreakStarted);
            timeBreakStarted = 0;
            breakLength = 0;
        } else {
            return false;
        }
    }

    // if this is the only available ambulance in the depot and Strategic Resorve policy is used
    if (assignedEventId == -1 && currentEventTriageImpression == "A" && Settings::get<bool>("DISPATCH_STRATEGY_RESPONSE_RESTRICTED")) {
        bool onlyAvailableAmbulance = true;

        for (int ambulanceIndex = 0; ambulanceIndex < ambulances.size(); ambulanceIndex++) {
            if (ambulances[ambulanceIndex].id == id) {
                continue;
            }

            const bool assignedToSameDepot = ambulances[ambulanceIndex].allocatedDepotIndex == allocatedDepotIndex;
            const bool available = ambulances[ambulanceIndex].assignedEventId == -1;

            if (assignedToSameDepot && available) {
                onlyAvailableAmbulance = false;

                break;
            }
        }

        if (onlyAvailableAmbulance) {
            return false;
        }
    }

    // if ambulance is not assigned to an event
    if (assignedEventId == -1) {
        return true;
    }

    // if ambulance is returing to depot
    if (events[eventIndex].type == EventType::DISPATCHING_TO_DEPOT) {
        return true;
    }

    // if ambulance is dispatching to scene and Dynamic Reassignment policy is used
    if (events[eventIndex].type == EventType::DISPATCHING_TO_SCENE) {
        bool shouldPrioritizeHigherTriage = Settings::get<bool>("DISPATCH_STRATEGY_PRIORITIZE_TRIAGE");
        bool eventIsHigherTriage =  higherTriagePriority(currentEventTriageImpression, events[eventIndex].triageImpression);
        if (shouldPrioritizeHigherTriage && eventIsHigherTriage) {
            return true;
        }
    }

    return false;
}

void Ambulance::scheduleBreaks(
    const time_t& shiftStart,
    const time_t& shiftEnd,
    const int depotSize,
    const int depotNum
) {
    const time_t HALF_HOUR = 1800;
    const time_t ONE_HOUR = 3600;
    const time_t FOUR_HOURS = 14400;

    // constraints: at least 1 hour after shift starts and 1 hour before shift ends. minimum 4 hours between each break
    time_t firstHourEnd = shiftStart + ONE_HOUR;
    time_t lastHourStart = shiftEnd - ONE_HOUR;
    time_t minBreakInterval = FOUR_HOURS;

    // try to spread it out according to initial allocation (will be less accurate when using time segmentation)
    time_t break1Start = firstHourEnd + (depotNum % depotSize) * (minBreakInterval / depotSize);
    if (break1Start > lastHourStart - HALF_HOUR) {
        break1Start = firstHourEnd;
    }

    time_t break2Start = break1Start + minBreakInterval;

    if (break2Start + HALF_HOUR > lastHourStart) {
        break2Start = lastHourStart - HALF_HOUR;
    }

    scheduledBreaks.push_back(break1Start);
    scheduledBreaks.push_back(break2Start);
}

bool Ambulance::higherTriagePriority(const std::string& triage, const std::string& triageToCompare) {
    if (triage == "A" && triageToCompare != "A") {
        return true;
    }

    if (triage == "H" && triageToCompare != "A" && triageToCompare != "H") {
        return true;
    }

    return false;
}

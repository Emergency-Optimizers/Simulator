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
        int breakLength = 30 * 60;

        setBreak(breakLength, currentTime);

        scheduledBreaks.erase(scheduledBreaks.begin());
    }
}

void Ambulance::setBreak(const int newBreakLength, const time_t& currentTime) {
    timeBreakStarted = currentTime;
    breakLenght = newBreakLength;
}

bool Ambulance::isAvailable(
    const std::vector<Event>& events,
    const int eventIndex,
    const time_t& currentTime,
    const std::string& currentEventTriageImpression
) {
    // check if ambulance should start break
    if (breakLenght == 0) {
        if (assignedEventId == -1) {
            checkScheduledBreak(currentTime);
        } else if (!scheduledBreaks.empty() && currentTime >= scheduledBreaks.front()) {
            return false;
        }
    }

    // if the ambulance is on a break and it is not finished, mark it as unavailable
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

    if (events[eventIndex].type == EventType::DISPATCHING_TO_DEPOT) {
        return true;
    }

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

bool Ambulance::higherTriagePriority(const std::string& triage, const std::string& triageToCompare) {
    if (triage == "A" && triageToCompare != "A") {
        return true;
    }

    if (triage == "H" && triageToCompare != "A" && triageToCompare != "H") {
        return true;
    }

    return false;
}

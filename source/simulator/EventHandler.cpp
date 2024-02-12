/**
 * @file EventHandler.cpp
 * @author Sindre Eiklid
 * @version 1.0
 * @date 2023-11-13
 *
 * @copyright Copyright (c) 2023 Sindre Eiklid
 */

/* external libraries */
#include <ctime>
/* internal libraries */
#include "simulator/EventHandler.hpp"
#include "Utils.hpp"

void EventHandler::populate(Incidents& incidents, const std::string& start, const std::string& end) {
    const std::tm startTime = Utils::stringToTm(start);
    const std::tm endTime = Utils::stringToTm(end);

    const std::vector<std::tm> times = incidents.getColumnOfTimes("time_call_received");
    int startIndex = Utils::findClosestTimeIndex(startTime, times);
    int endIndex = Utils::findClosestTimeIndex(endTime, times);

    events.clear();
    currentIndex = 0;

    for (std::size_t i = startIndex; i < endIndex + 1; i++) {
        EventOld event;
        event.type = EventType::CALL_PROCESSED;
        std::tm timeCallReceived = incidents.get<std::optional<std::tm>>("time_call_received", i).value();
        event.timer = std::mktime(&timeCallReceived);
        event.incidentIndex = i;
        event.assignedAmbulanceIndex = -1;
        event.targetGridId = incidents.get<int64_t>("grid_id", i);

        events.push_back(event);
    }
}

int EventHandler::getNextEventIndex() {
    for (; currentIndex < events.size(); currentIndex++) {
        if (events[currentIndex].type != EventType::NONE) return currentIndex;
    }

    return -1;
}

void EventHandler::sortEvent(size_t eventIndex) {
    std::vector<EventOld>::iterator newPosition = std::lower_bound(
        events.begin() + eventIndex,
        events.end(),
        events[eventIndex],
        [](const EventOld& a, const EventOld& b) { return a.timer < b.timer; }
    );

    if (newPosition != events.begin() + eventIndex) {
        std::rotate(events.begin() + eventIndex, events.begin() + eventIndex + 1, newPosition);
    }
}

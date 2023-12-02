/**
 * @file EventHandler.cpp
 * @author Sindre Eiklid
 * @version 1.0
 * @date 2023-11-13
 *
 * @copyright Copyright (c) 2023 Sindre Eiklid
 */

#include <ctime>
/* internal libraries */
#include "EventHandler.hpp"
#include "Utils.hpp"

void EventHandler::populate(Incidents& incidents, const std::string& start, const std::string& end) {
    const std::tm startTime = Utils::stringToTm(start);
    const std::tm endTime = Utils::stringToTm(end);

    const std::vector<std::tm> times = incidents.getColumnOfTimes("time_call_received");
    int startIndex = Utils::findClosestTimeIndex(startTime, times);
    int endIndex = Utils::findClosestTimeIndex(endTime, times);

    events.clear();

    for (std::size_t i = startIndex; i < endIndex + 1; i++) {
        const CellType cellTimeCallReceived = incidents.get("time_call_received", i);
        const std::tm& timeCallReceived = std::get<std::optional<std::tm>>(cellTimeCallReceived).value();

        const CellType cellTimeCallProcessed = incidents.get("time_call_processed", i);
        const std::tm& timeCallProcessed = std::get<std::optional<std::tm>>(cellTimeCallProcessed).value();

        Event event;
        event.type = EventType::DISPATCH_TO_SCENE;
        event.incidentIndex = i;
        event.assignedAmbulanceIndex = -1;
        event.timeSeconds = Utils::timeDifferenceInSeconds(timeCallReceived, timeCallProcessed);

        events.push_back(event);
    }
}

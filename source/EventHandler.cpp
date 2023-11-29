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
    std::tm startTime = Utils::stringToTm(start);
    std::tm endTime = Utils::stringToTm(end);

    for (std::size_t i = 0; i < incidents.size(); ++i) {
        const CellType cell = incidents.get("time_call_received", i);

        if (std::holds_alternative<std::optional<std::tm>>(cell)) {
            const auto& optTime = std::get<std::optional<std::tm>>(cell);
            if (optTime.has_value()) {
                const std::tm& incidentTime = optTime.value();
                if (Utils::compareTime(incidentTime, startTime) >= 0 && Utils::compareTime(incidentTime, endTime) <= 0) {
                    const CellType cellIncidentDispatchSceneTime = incidents.get("time_call_processed", i);
                    const std::tm& incidentDispatchSceneTime = std::get<std::optional<std::tm>>(cellIncidentDispatchSceneTime).value();

                    Event event;
                    event.incidentIndex = i;
                    event.status = IncidentStatus::TRAVELLING_TO_SCENE;
                    event.assignedAmbulanceIndex = -1;
                    event.timeSeconds = Utils::timeDifferenceInSeconds(incidentTime, incidentDispatchSceneTime);

                    events.push_back(event);
                }
            }
        }
    }
}

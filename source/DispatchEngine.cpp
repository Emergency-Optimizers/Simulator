/**
 * @file DispatchEngine.cpp
 * @author Sindre Eiklid
 * @version 1.0
 * @date 2024-01-06
 *
 * @copyright Copyright (c) 2023 Sindre Eiklid
 */

/* internal libraries */
#include "DispatchEngine.hpp"

void DispatchEngine::dispatch(
    const DispatchEngineStrategy strategy,
    std::mt19937& rng,
    Incidents& incidents,
    Stations& stations,
    ODMatrix& odMatrix,
    std::vector<Ambulance>& ambulances,
    Event& event
) {
    switch (strategy) {
        default:
            randomStrategy(rng, incidents, stations, odMatrix, ambulances, event);
    }
}

void DispatchEngine::randomStrategy(
    std::mt19937& rng,
    Incidents& incidents,
    Stations& stations,
    ODMatrix& odMatrix,
    std::vector<Ambulance>& ambulances,
    Event& event
) {
    if (event.assignedAmbulanceIndex == -1) {
        std::vector<unsigned> availableAmbulanceIndicies;
        for (int i = 0; i < ambulances.size(); i++) {
            if (ambulances[i].assignedEventIndex == -1) availableAmbulanceIndicies.push_back(i);
        }

        if (availableAmbulanceIndicies.empty()) return;

        std::uniform_int_distribution<int> rndBetween(0, availableAmbulanceIndicies.size() - 1);
        event.assignedAmbulanceIndex = availableAmbulanceIndicies[rndBetween(rng)];

        ambulances[event.assignedAmbulanceIndex].targetGridId = incidents.get<int>("grid_id", event.incidentIndex);
    }

    switch (event.type) {
        case EventType::DISPATCH_TO_SCENE:

            break;
    }
}

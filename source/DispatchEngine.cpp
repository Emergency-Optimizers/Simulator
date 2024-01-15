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
        std::vector<unsigned> availableAmbulanceIndicies = Utils::getAvailableAmbulanceIndicies(ambulances);
        /// TODO: Add some time before checking again (maybe 1 second after next event so we constantly check for available ambulances).
        if (availableAmbulanceIndicies.empty()) return;
        event.assignedAmbulanceIndex = Utils::getRandomElement(rng, availableAmbulanceIndicies);
    }

    switch (event.type) {
        case EventType::CALL_RECEIVED:
            int incrementSeconds = incidents.timeDifferenceBetweenHeaders(
                "time_call_received",
                "time_call_processed",
                event.incidentIndex
            );
            event.timer += incrementSeconds;

            event.type = EventType::DISPATCH_TO_SCENE;

            break;
        case EventType::DISPATCH_TO_SCENE:
            int incrementSeconds = odMatrix.getTravelTime(
                ambulances[event.assignedAmbulanceIndex].currentGridId,
                event.targetGridId
            );
            event.timer += incrementSeconds;

            ambulances[event.assignedAmbulanceIndex].currentGridId = event.targetGridId;

            event.type = EventType::ARRIVED_AT_SCENE;

            break;
        case EventType::ARRIVED_AT_SCENE:
            if (incidents.get<std::optional<std::tm>>("time_departure_scene", event.incidentIndex).has_value()) {
                int incrementSeconds = incidents.timeDifferenceBetweenHeaders(
                    "time_departure_scene",
                    "time_arrival_scene",
                    event.incidentIndex
                );
                event.timer += incrementSeconds;

                event.targetGridId = stations.get<int>(
                    "grid_id",
                    Utils::getRandomElement(rng, stations.getHospitalIndices())
                );

                event.type = EventType::DISPATCH_TO_HOSPITAL;
            } else {
                /// TODO: Add some time.

                event.targetGridId = stations.get<int>(
                    "grid_id",
                    ambulances[event.assignedAmbulanceIndex].allocatedDepotIndex
                );

                event.type = EventType::DISPATCH_TO_DEPOT;
            }

            break;
        case EventType::DISPATCH_TO_HOSPITAL:
            int incrementSeconds = odMatrix.getTravelTime(
                ambulances[event.assignedAmbulanceIndex].currentGridId,
                event.targetGridId
            );
            event.timer += incrementSeconds;

            ambulances[event.assignedAmbulanceIndex].currentGridId = event.targetGridId;

            event.type = EventType::ARRIVED_AT_HOSPITAL;

            break;
        case EventType::ARRIVED_AT_HOSPITAL:
            int incrementSeconds = incidents.timeDifferenceBetweenHeaders(
                "time_arrival_hospital",
                "time_available",
                event.incidentIndex
            );
            event.timer += incrementSeconds;

            event.targetGridId = stations.get<int>(
                "grid_id",
                ambulances[event.assignedAmbulanceIndex].allocatedDepotIndex
            );

            event.type = EventType::DISPATCH_TO_DEPOT;

            break;
        case EventType::DISPATCH_TO_DEPOT:
            int incrementSeconds = odMatrix.getTravelTime(
                ambulances[event.assignedAmbulanceIndex].currentGridId,
                event.targetGridId
            );
            event.timer += incrementSeconds;
            
            ambulances[event.assignedAmbulanceIndex].currentGridId = event.targetGridId;

            event.type = EventType::NONE;

            break;
    }
}

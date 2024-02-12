/**
 * @file DispatchEngine.cpp
 * @author Sindre Eiklid
 * @version 1.0
 * @date 2024-01-06
 *
 * @copyright Copyright (c) 2023 Sindre Eiklid
 */

/* internal libraries */
#include "simulator/DispatchEngine.hpp"

void DispatchEngine::dispatch(
    const DispatchEngineStrategyType strategy,
    std::mt19937& rng,
    Incidents& incidents,
    Stations& stations,
    ODMatrix& odMatrix,
    std::vector<Ambulance>& ambulances,
    EventOld& event
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
    EventOld& event
) {
    if (event.assignedAmbulanceIndex == -1) {
        std::vector<unsigned> availableAmbulanceIndicies = Utils::getAvailableAmbulanceIndicies(ambulances);
        /// TODO: Add some time before checking again (maybe 1 second after next event
        /// so we constantly check for available ambulances) or tell the simulator to make an ambulance available.
        if (availableAmbulanceIndicies.empty()) {
            event.timer += 60;
            event.metrics.waitingForAmbulanceTime += 60;
            return;
        }

        event.assignedAmbulanceIndex = Utils::getRandomElement(rng, availableAmbulanceIndicies);
    }

    int incrementSeconds = 0;

    switch (event.type) {
        case EventType::CALL_PROCESSED:
            incrementSeconds = incidents.timeDifferenceBetweenHeaders(
                "time_call_received",
                "time_call_answered",
                event.incidentIndex
            );
            event.timer += incrementSeconds;
            event.metrics.callProcessedTime += incrementSeconds;

            event.type = EventType::DISPATCH_TO_SCENE;

            break;
        case EventType::DISPATCH_TO_SCENE:
            incrementSeconds = odMatrix.getTravelTime(
                ambulances[event.assignedAmbulanceIndex].currentGridId,
                event.targetGridId
            );
            event.timer += incrementSeconds;
            event.metrics.dispatchToSceneTime += incrementSeconds;

            ambulances[event.assignedAmbulanceIndex].currentGridId = event.targetGridId;

            event.type = EventType::ARRIVED_AT_SCENE;

            break;
        case EventType::ARRIVED_AT_SCENE:
            if (incidents.get<std::optional<std::tm>>("time_departure_scene", event.incidentIndex).has_value()) {
                incrementSeconds = incidents.timeDifferenceBetweenHeaders(
                    "time_arrival_scene",
                    "time_departure_scene",
                    event.incidentIndex
                );
                event.timer += incrementSeconds;
                event.metrics.arrivalAtSceneTime += incrementSeconds;

                event.targetGridId = stations.get<int64_t>(
                    "grid_id",
                    Utils::getRandomElement(rng, stations.getHospitalIndices())
                );

                event.type = EventType::DISPATCH_TO_HOSPITAL;
            } else {
                incrementSeconds = incidents.timeDifferenceBetweenHeaders(
                    "time_arrival_scene",
                    "time_available",
                    event.incidentIndex
                );
                event.timer += incrementSeconds;
                event.metrics.arrivalAtSceneTime += incrementSeconds;

                event.targetGridId = stations.get<int64_t>(
                    "grid_id",
                    ambulances[event.assignedAmbulanceIndex].allocatedDepotIndex
                );

                event.type = EventType::DISPATCH_TO_DEPOT;
            }

            break;
        case EventType::DISPATCH_TO_HOSPITAL:
            incrementSeconds = odMatrix.getTravelTime(
                ambulances[event.assignedAmbulanceIndex].currentGridId,
                event.targetGridId
            );
            event.timer += incrementSeconds;
            event.metrics.dispatchToHospitalTime += incrementSeconds;

            ambulances[event.assignedAmbulanceIndex].currentGridId = event.targetGridId;

            event.type = EventType::ARRIVED_AT_HOSPITAL;

            break;
        case EventType::ARRIVED_AT_HOSPITAL:
            incrementSeconds = incidents.timeDifferenceBetweenHeaders(
                "time_arrival_hospital",
                "time_available",
                event.incidentIndex
            );
            event.timer += incrementSeconds;
            event.metrics.arrivalAtHospitalTime += incrementSeconds;

            event.targetGridId = stations.get<int64_t>(
                "grid_id",
                ambulances[event.assignedAmbulanceIndex].allocatedDepotIndex
            );

            event.type = EventType::DISPATCH_TO_DEPOT;

            break;
        case EventType::DISPATCH_TO_DEPOT:
            incrementSeconds = odMatrix.getTravelTime(
                ambulances[event.assignedAmbulanceIndex].currentGridId,
                event.targetGridId
            );
            event.timer += incrementSeconds;
            event.metrics.dispatchToDepotTime += incrementSeconds;

            ambulances[event.assignedAmbulanceIndex].currentGridId = event.targetGridId;
            ambulances[event.assignedAmbulanceIndex].assignedEventIndex = -1;
            event.assignedAmbulanceIndex = -1;

            event.type = EventType::NONE;

            break;
    }
}

/**
 * @file DispatchEngineStrategy.cpp
 *
 * @copyright Copyright (c) 2024 Emergency-Optimizers
 */

/* internal libraries */
#include "simulator/strategies/DispatchEngineStrategy.hpp"

void DispatchEngineStrategy::assigningAmbulance(
    std::mt19937& rng,
    Incidents& incidents,
    Stations& stations,
    ODMatrix& odMatrix,
    std::vector<Ambulance>& ambulances,
    std::vector<Event>& events,
    const int eventIndex
) {
    /// TODO: code here
}

void DispatchEngineStrategy::dispatchingToScene(
    std::mt19937& rng,
    Incidents& incidents,
    Stations& stations,
    ODMatrix& odMatrix,
    std::vector<Ambulance>& ambulances,
    std::vector<Event>& events,
    const int eventIndex
) {
    int incrementSeconds = odMatrix.getTravelTime(
        ambulances[events[eventIndex].assignedAmbulanceIndex].currentGridId,
        events[eventIndex].gridId
    );
    events[eventIndex].timer += incrementSeconds;
    events[eventIndex].metrics.dispatchToSceneTime += incrementSeconds;

    ambulances[events[eventIndex].assignedAmbulanceIndex].currentGridId = events[eventIndex].gridId;

    if (events[eventIndex].secondsWaitDepartureScene != -1) {
        incrementSeconds = events[eventIndex].secondsWaitDepartureScene;
        events[eventIndex].timer += incrementSeconds;
        events[eventIndex].metrics.arrivalAtSceneTime += incrementSeconds;

        events[eventIndex].type = EventType::DISPATCHING_TO_HOSPITAL;
    } else {
        incrementSeconds = events[eventIndex].secondsWaitAvailable;
        events[eventIndex].timer += incrementSeconds;
        events[eventIndex].metrics.arrivalAtSceneTime += incrementSeconds;

        events[eventIndex].type = EventType::DISPATCHING_TO_DEPOT;
    }
}

void DispatchEngineStrategy::dispatchingToHospital(
    std::mt19937& rng,
    Incidents& incidents,
    Stations& stations,
    ODMatrix& odMatrix,
    std::vector<Ambulance>& ambulances,
    std::vector<Event>& events,
    const int eventIndex
) {
    /// TODO: code here
}

void DispatchEngineStrategy::dispatchingToDepot(
    std::mt19937& rng,
    Incidents& incidents,
    Stations& stations,
    ODMatrix& odMatrix,
    std::vector<Ambulance>& ambulances,
    std::vector<Event>& events,
    const int eventIndex
) {
    events[eventIndex].gridId = stations.get<int64_t>(
        "grid_id",
        ambulances[events[eventIndex].assignedAmbulanceIndex].allocatedDepotIndex
    );

    int incrementSeconds = odMatrix.getTravelTime(
        ambulances[events[eventIndex].assignedAmbulanceIndex].currentGridId,
        events[eventIndex].gridId
    );
    events[eventIndex].timer += incrementSeconds;

    events[eventIndex].type = EventType::FINISHED;
}

void DispatchEngineStrategy::finishingEvent(
    std::mt19937& rng,
    Incidents& incidents,
    Stations& stations,
    ODMatrix& odMatrix,
    std::vector<Ambulance>& ambulances,
    std::vector<Event>& events,
    const int eventIndex
) {
    int incrementSeconds = odMatrix.getTravelTime(
        ambulances[events[eventIndex].assignedAmbulanceIndex].currentGridId,
        events[eventIndex].gridId
    );
    events[eventIndex].metrics.dispatchToDepotTime += incrementSeconds;

    ambulances[events[eventIndex].assignedAmbulanceIndex].currentGridId = events[eventIndex].gridId;
    ambulances[events[eventIndex].assignedAmbulanceIndex].assignedEventIndex = -1;
    events[eventIndex].assignedAmbulanceIndex = -1;

    events[eventIndex].type = EventType::NONE;
}

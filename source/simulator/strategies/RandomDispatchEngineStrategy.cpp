/**
 * @file RandomDispatchEngineStrategy.cpp
 *
 * @copyright Copyright (c) 2024 Emergency-Optimizers
 */

/* internal libraries */
#include "simulator/strategies/RandomDispatchEngineStrategy.hpp"

void RandomDispatchEngineStrategy::run(
    std::mt19937& rng,
    Incidents& incidents,
    Stations& stations,
    ODMatrix& odMatrix,
    std::vector<Ambulance>& ambulances,
    std::vector<Event>& events,
    const int eventIndex
) {
    if (events[eventIndex].assignedAmbulanceIndex == -1) {
        if (!assignAmbulance(rng, incidents, stations, odMatrix, ambulances, events, eventIndex)) return;
    }

    switch (events[eventIndex].type) {
        case EventType::CALL_PROCESSED:
            callProcessed(rng, incidents, stations, odMatrix, ambulances, events, eventIndex);
            break;
        case EventType::DISPATCH_TO_SCENE:
            dispatchToScene(rng, incidents, stations, odMatrix, ambulances, events, eventIndex);
            break;
        case EventType::ARRIVED_AT_SCENE:
            arrivedAtScene(rng, incidents, stations, odMatrix, ambulances, events, eventIndex);
            break;
        case EventType::DISPATCH_TO_HOSPITAL:
            dispatchToHospital(rng, incidents, stations, odMatrix, ambulances, events, eventIndex);
            break;
        case EventType::ARRIVED_AT_HOSPITAL:
            arrivedAtHospital(rng, incidents, stations, odMatrix, ambulances, events, eventIndex);
            break;
        case EventType::DISPATCH_TO_DEPOT:
            dispatchToDepot(rng, incidents, stations, odMatrix, ambulances, events, eventIndex);
            break;
    }
}

bool RandomDispatchEngineStrategy::assignAmbulance(
    std::mt19937& rng,
    Incidents& incidents,
    Stations& stations,
    ODMatrix& odMatrix,
    std::vector<Ambulance>& ambulances,
    std::vector<Event>& events,
    const int eventIndex
) {
    std::vector<unsigned> availableAmbulanceIndicies = Utils::getAvailableAmbulanceIndicies(ambulances);
    /// TODO: Add some time before checking again (maybe 1 second after next event
    /// so we constantly check for available ambulances) or tell the simulator to make an ambulance available.
    if (availableAmbulanceIndicies.empty()) {
        events[eventIndex].timer += 60;
        events[eventIndex].metrics.waitingForAmbulanceTime += 60;

        return false;
    }

    events[eventIndex].assignedAmbulanceIndex = Utils::getRandomElement(rng, availableAmbulanceIndicies);
    ambulances[events[eventIndex].assignedAmbulanceIndex].assignedEventIndex = eventIndex;

    return true;
}

void RandomDispatchEngineStrategy::callProcessed(
    std::mt19937& rng,
    Incidents& incidents,
    Stations& stations,
    ODMatrix& odMatrix,
    std::vector<Ambulance>& ambulances,
    std::vector<Event>& events,
    const int eventIndex
) {
    int incrementSeconds = events[eventIndex].secondsWaitCallAnswered;
    events[eventIndex].timer += incrementSeconds;
    events[eventIndex].metrics.callProcessedTime += incrementSeconds;

    events[eventIndex].type = EventType::DISPATCH_TO_SCENE;
}

void RandomDispatchEngineStrategy::dispatchToScene(
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

    events[eventIndex].type = EventType::ARRIVED_AT_SCENE;
}

void RandomDispatchEngineStrategy::arrivedAtScene(
    std::mt19937& rng,
    Incidents& incidents,
    Stations& stations,
    ODMatrix& odMatrix,
    std::vector<Ambulance>& ambulances,
    std::vector<Event>& events,
    const int eventIndex
) {
    if (events[eventIndex].secondsWaitDepartureScene != -1) {
        int incrementSeconds = events[eventIndex].secondsWaitDepartureScene;
        events[eventIndex].timer += incrementSeconds;
        events[eventIndex].metrics.arrivalAtSceneTime += incrementSeconds;

        events[eventIndex].gridId = stations.get<int64_t>(
            "grid_id",
            Utils::getRandomElement(rng, stations.getHospitalIndices())
        );

        events[eventIndex].type = EventType::DISPATCH_TO_HOSPITAL;
    } else {
        int incrementSeconds = events[eventIndex].secondsWaitAvailable;
        events[eventIndex].timer += incrementSeconds;
        events[eventIndex].metrics.arrivalAtSceneTime += incrementSeconds;

        events[eventIndex].gridId = stations.get<int64_t>(
            "grid_id",
            ambulances[events[eventIndex].assignedAmbulanceIndex].allocatedDepotIndex
        );

        events[eventIndex].type = EventType::DISPATCH_TO_DEPOT;
    }
}

void RandomDispatchEngineStrategy::dispatchToHospital(
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
    events[eventIndex].metrics.dispatchToHospitalTime += incrementSeconds;

    ambulances[events[eventIndex].assignedAmbulanceIndex].currentGridId = events[eventIndex].gridId;

    events[eventIndex].type = EventType::ARRIVED_AT_HOSPITAL;
}

void RandomDispatchEngineStrategy::arrivedAtHospital(
    std::mt19937& rng,
    Incidents& incidents,
    Stations& stations,
    ODMatrix& odMatrix,
    std::vector<Ambulance>& ambulances,
    std::vector<Event>& events,
    const int eventIndex
) {
    int incrementSeconds = events[eventIndex].secondsWaitAvailable;
    events[eventIndex].timer += incrementSeconds;
    events[eventIndex].metrics.arrivalAtHospitalTime += incrementSeconds;

    events[eventIndex].gridId = stations.get<int64_t>(
        "grid_id",
        ambulances[events[eventIndex].assignedAmbulanceIndex].allocatedDepotIndex
    );

    events[eventIndex].type = EventType::DISPATCH_TO_DEPOT;
}

void RandomDispatchEngineStrategy::dispatchToDepot(
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
    events[eventIndex].metrics.dispatchToDepotTime += incrementSeconds;

    ambulances[events[eventIndex].assignedAmbulanceIndex].currentGridId = events[eventIndex].gridId;
    ambulances[events[eventIndex].assignedAmbulanceIndex].assignedEventIndex = -1;
    events[eventIndex].assignedAmbulanceIndex = -1;

    events[eventIndex].type = EventType::NONE;
}

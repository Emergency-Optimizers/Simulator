/**
 * @file ClosestDispatchEngineStrategy.cpp
 *
 * @copyright Copyright (c) 2024 Emergency-Optimizers
 */

/* external libraries */
#include <limits>
/* internal libraries */
#include "simulator/strategies/ClosestDispatchEngineStrategy.hpp"

void ClosestDispatchEngineStrategy::run(
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

bool ClosestDispatchEngineStrategy::assignAmbulance(
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

    // find closest ambulance
    int closestAmbulanceIndex = -1;
    int closestAmbulanceTravelTime = std::numeric_limits<int>::max();
    int64_t eventGridId = events[eventIndex].gridId;
    for (int i = 0; i < availableAmbulanceIndicies.size(); i++) {
        int travelTime = odMatrix.getTravelTime(ambulances[i].currentGridId, eventGridId);
        if (travelTime < closestAmbulanceTravelTime) {
            closestAmbulanceIndex = i;
            closestAmbulanceTravelTime = travelTime;
        }
    }

    events[eventIndex].assignedAmbulanceIndex = availableAmbulanceIndicies[closestAmbulanceIndex];
    ambulances[events[eventIndex].assignedAmbulanceIndex].assignedEventIndex = eventIndex;

    return true;
}

void ClosestDispatchEngineStrategy::callProcessed(
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

void ClosestDispatchEngineStrategy::dispatchToScene(
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

void ClosestDispatchEngineStrategy::arrivedAtScene(
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

        // find closest hospital
        int closestHospitalIndex = -1;
        int closestHospitalTravelTime = std::numeric_limits<int>::max();
        int64_t eventGridId = events[eventIndex].gridId;
        std::vector<unsigned int> hospitals = stations.getHospitalIndices();
        for (int i = 0; i < hospitals.size(); i++) {
            int64_t hospitalGridId = stations.get<int64_t>(
                "grid_id",
                hospitals[i]
            );
            int travelTime = odMatrix.getTravelTime(hospitalGridId, eventGridId);
            if (travelTime < closestHospitalTravelTime) {
                closestHospitalIndex = i;
                closestHospitalTravelTime = travelTime;
            }
        }

        events[eventIndex].gridId = stations.get<int64_t>(
            "grid_id",
            hospitals[closestHospitalIndex]
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

void ClosestDispatchEngineStrategy::dispatchToHospital(
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

void ClosestDispatchEngineStrategy::arrivedAtHospital(
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

void ClosestDispatchEngineStrategy::dispatchToDepot(
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

bool DispatchEngineStrategy::assigningAmbulance(
    std::mt19937& rng,
    Incidents& incidents,
    Stations& stations,
    ODMatrix& odMatrix,
    std::vector<Ambulance>& ambulances,
    std::vector<Event>& events,
    const int eventIndex
) {
    /// TODO: code here
    return false;
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
    /// TODO: code here
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
    /// TODO: code here
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
    /// TODO: code here
}

/*

call received           (incident starts)
call answered           (wait time from histogram)
assign ambulance        (wait time until ambulance is available)
dispatch to scene       (OD matrix)
time spent on scene     (wait time from histogram)
dispatch to hospital    (OD matrix)
time spent at hospital  (wait time histogram)
dispatch to depot       (OD matrix)





*/

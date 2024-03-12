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
    switch (events[eventIndex].type) {
        case EventType::ASSIGNING_AMBULANCE:
            assigningAmbulance(rng, incidents, stations, odMatrix, ambulances, events, eventIndex);
            break;
        case EventType::DISPATCHING_TO_SCENE:
            dispatchingToScene(rng, incidents, stations, odMatrix, ambulances, events, eventIndex);
            break;
        case EventType::DISPATCHING_TO_HOSPITAL:
            dispatchingToHospital(rng, incidents, stations, odMatrix, ambulances, events, eventIndex);
            break;
        case EventType::DISPATCHING_TO_DEPOT:
            dispatchingToDepot(rng, incidents, stations, odMatrix, ambulances, events, eventIndex);
            break;
        case EventType::FINISHED:
            finishingEvent(rng, incidents, stations, odMatrix, ambulances, events, eventIndex);
            break;
    }
}

void ClosestDispatchEngineStrategy::assigningAmbulance(
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

        return;
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
    events[eventIndex].type = EventType::DISPATCHING_TO_SCENE;
}

void ClosestDispatchEngineStrategy::dispatchingToHospital(
    std::mt19937& rng,
    Incidents& incidents,
    Stations& stations,
    ODMatrix& odMatrix,
    std::vector<Ambulance>& ambulances,
    std::vector<Event>& events,
    const int eventIndex
) {
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

    int incrementSeconds = odMatrix.getTravelTime(
        ambulances[events[eventIndex].assignedAmbulanceIndex].currentGridId,
        events[eventIndex].gridId
    );
    events[eventIndex].timer += incrementSeconds;
    events[eventIndex].metrics.dispatchToHospitalTime += incrementSeconds;
    ambulances[events[eventIndex].assignedAmbulanceIndex].timeUnavailable += incrementSeconds;

    ambulances[events[eventIndex].assignedAmbulanceIndex].currentGridId = events[eventIndex].gridId;

    events[eventIndex].timer += events[eventIndex].secondsWaitAvailable;
    events[eventIndex].metrics.arrivalAtHospitalTime += events[eventIndex].secondsWaitAvailable;
    ambulances[events[eventIndex].assignedAmbulanceIndex].timeUnavailable += events[eventIndex].secondsWaitAvailable;

    events[eventIndex].type = EventType::DISPATCHING_TO_DEPOT;
}

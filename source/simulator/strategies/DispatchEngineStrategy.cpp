/**
 * @file DispatchEngineStrategy.cpp
 *
 * @copyright Copyright (c) 2024 Emergency-Optimizers
 */

/* internal libraries */
#include "simulator/strategies/DispatchEngineStrategy.hpp"

void DispatchEngineStrategy::assigningAmbulance(
    std::mt19937& rng,
    std::vector<Ambulance>& ambulances,
    std::vector<Event>& events,
    const int eventIndex
) {
    /// TODO: code here
}

void DispatchEngineStrategy::dispatchingToScene(
    std::mt19937& rng,
    std::vector<Ambulance>& ambulances,
    std::vector<Event>& events,
    const int eventIndex
) {
    int incrementSeconds = ODMatrix::getInstance().getTravelTime(
        ambulances[events[eventIndex].assignedAmbulanceIndex].currentGridId,
        events[eventIndex].gridId,
        false,
        events[eventIndex].triageImpression,
        events[eventIndex].timer
    );
    events[eventIndex].updateTimer(incrementSeconds, "duration_dispatching_to_scene");
    ambulances[events[eventIndex].assignedAmbulanceIndex].timeUnavailable += incrementSeconds;

    ambulances[events[eventIndex].assignedAmbulanceIndex].currentGridId = events[eventIndex].gridId;

    if (events[eventIndex].secondsWaitDepartureScene != -1) {
        incrementSeconds = events[eventIndex].secondsWaitDepartureScene;
        events[eventIndex].updateTimer(incrementSeconds, "duration_at_scene");
        ambulances[events[eventIndex].assignedAmbulanceIndex].timeUnavailable += incrementSeconds;

        events[eventIndex].type = EventType::DISPATCHING_TO_HOSPITAL;
    } else {
        incrementSeconds = events[eventIndex].secondsWaitAvailable;
        events[eventIndex].updateTimer(incrementSeconds, "duration_at_scene");
        ambulances[events[eventIndex].assignedAmbulanceIndex].timeUnavailable += incrementSeconds;

        events[eventIndex].type = EventType::DISPATCHING_TO_DEPOT;
    }
}

void DispatchEngineStrategy::dispatchingToHospital(
    std::mt19937& rng,
    std::vector<Ambulance>& ambulances,
    std::vector<Event>& events,
    const int eventIndex
) {
    /// TODO: code here
}

void DispatchEngineStrategy::dispatchingToDepot(
    std::mt19937& rng,
    std::vector<Ambulance>& ambulances,
    std::vector<Event>& events,
    const int eventIndex
) {
    events[eventIndex].gridId = Stations::getInstance().get<int64_t>(
        "grid_id",
        ambulances[events[eventIndex].assignedAmbulanceIndex].allocatedDepotIndex
    );

    int incrementSeconds = ODMatrix::getInstance().getTravelTime(
        ambulances[events[eventIndex].assignedAmbulanceIndex].currentGridId,
        events[eventIndex].gridId,
        true,
        events[eventIndex].triageImpression,
        events[eventIndex].timer
    );
    events[eventIndex].updateTimer(incrementSeconds);

    events[eventIndex].type = EventType::FINISHED;
}

void DispatchEngineStrategy::finishingEvent(
    std::mt19937& rng,
    std::vector<Ambulance>& ambulances,
    std::vector<Event>& events,
    const int eventIndex
) {
    int incrementSeconds = ODMatrix::getInstance().getTravelTime(
        ambulances[events[eventIndex].assignedAmbulanceIndex].currentGridId,
        events[eventIndex].gridId,
        true,
        events[eventIndex].triageImpression,
        events[eventIndex].prevTimer
    );
    events[eventIndex].metrics["duration_dispatching_to_depot"] += incrementSeconds;
    ambulances[events[eventIndex].assignedAmbulanceIndex].timeUnavailable += incrementSeconds;

    ambulances[events[eventIndex].assignedAmbulanceIndex].currentGridId = events[eventIndex].gridId;
    ambulances[events[eventIndex].assignedAmbulanceIndex].assignedEventId = -1;
    ambulances[events[eventIndex].assignedAmbulanceIndex].checkScheduledBreak(events[eventIndex].timer);
    events[eventIndex].assignedAmbulanceIndex = -1;

    events[eventIndex].type = EventType::NONE;
}

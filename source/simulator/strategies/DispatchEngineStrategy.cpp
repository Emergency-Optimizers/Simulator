/**
 * @file DispatchEngineStrategy.cpp
 *
 * @copyright Copyright (c) 2024 Emergency-Optimizers
 */

/* internal libraries */
#include "simulator/strategies/DispatchEngineStrategy.hpp"
#include "file-reader/Stations.hpp"
#include "file-reader/ODMatrix.hpp"

bool DispatchEngineStrategy::assigningAmbulance(
    std::mt19937& rnd,
    std::vector<Ambulance>& ambulances,
    std::vector<Event>& events,
    const int eventIndex
) {
    /// TODO: code here
    return false;
}

void DispatchEngineStrategy::preparingToDispatchToScene(
    std::mt19937& rnd,
    std::vector<Ambulance>& ambulances,
    std::vector<Event>& events,
    const int eventIndex
) {
    int incrementSeconds = ODMatrix::getInstance().getTravelTime(
        events[eventIndex].assignedAmbulance->currentGridId,
        events[eventIndex].gridId,
        false,
        events[eventIndex].triageImpression,
        events[eventIndex].timer
    );

    events[eventIndex].updateTimer(incrementSeconds);

    events[eventIndex].type = EventType::DISPATCHING_TO_SCENE;
}

void DispatchEngineStrategy::dispatchingToScene(
    std::mt19937& rnd,
    std::vector<Ambulance>& ambulances,
    std::vector<Event>& events,
    const int eventIndex
) {
    int incrementSeconds = ODMatrix::getInstance().getTravelTime(
        events[eventIndex].assignedAmbulance->currentGridId,
        events[eventIndex].gridId,
        false,
        events[eventIndex].triageImpression,
        events[eventIndex].prevTimer
    );
    events[eventIndex].metrics["duration_dispatching_to_scene"] += incrementSeconds;
    events[eventIndex].assignedAmbulance->timeUnavailable += incrementSeconds;

    events[eventIndex].assignedAmbulance->currentGridId = events[eventIndex].gridId;

    if (events[eventIndex].secondsWaitDepartureScene != -1) {
        incrementSeconds = events[eventIndex].secondsWaitDepartureScene;
        events[eventIndex].updateTimer(incrementSeconds, "duration_at_scene");
        events[eventIndex].assignedAmbulance->timeUnavailable += incrementSeconds;

        events[eventIndex].type = EventType::DISPATCHING_TO_HOSPITAL;
    } else {
        incrementSeconds = events[eventIndex].secondsWaitAvailable;
        events[eventIndex].updateTimer(incrementSeconds, "duration_at_scene");
        events[eventIndex].assignedAmbulance->timeUnavailable += incrementSeconds;

        events[eventIndex].type = EventType::PREPARING_DISPATCH_TO_DEPOT;
    }
}

void DispatchEngineStrategy::dispatchingToHospital(
    std::mt19937& rnd,
    std::vector<Ambulance>& ambulances,
    std::vector<Event>& events,
    const int eventIndex
) {
    /// TODO: code here
}

void DispatchEngineStrategy::dispatchingToDepot(
    std::mt19937& rnd,
    std::vector<Ambulance>& ambulances,
    std::vector<Event>& events,
    const int eventIndex
) {
    events[eventIndex].gridId = Stations::getInstance().get<int64_t>(
        "grid_id",
        events[eventIndex].assignedAmbulance->allocatedDepotIndex
    );

    int incrementSeconds = ODMatrix::getInstance().getTravelTime(
        events[eventIndex].assignedAmbulance->currentGridId,
        events[eventIndex].gridId,
        true,
        events[eventIndex].triageImpression,
        events[eventIndex].timer
    );
    events[eventIndex].updateTimer(incrementSeconds);

    events[eventIndex].type = EventType::DISPATCHING_TO_DEPOT;
}

void DispatchEngineStrategy::finishingEvent(
    std::mt19937& rnd,
    std::vector<Ambulance>& ambulances,
    std::vector<Event>& events,
    const int eventIndex
) {
    int incrementSeconds = ODMatrix::getInstance().getTravelTime(
        events[eventIndex].assignedAmbulance->currentGridId,
        events[eventIndex].gridId,
        true,
        events[eventIndex].triageImpression,
        events[eventIndex].prevTimer
    );
    events[eventIndex].metrics["duration_dispatching_to_depot"] += incrementSeconds;
    events[eventIndex].assignedAmbulance->timeUnavailable += incrementSeconds;
    events[eventIndex].assignedAmbulance->currentGridId = events[eventIndex].gridId;

    // check if ambulance has been reallocated and send it to new depot
    int64_t assignedDepotGridId = Stations::getInstance().get<int64_t>(
        "grid_id",
        events[eventIndex].assignedAmbulance->allocatedDepotIndex
    );
    if (events[eventIndex].assignedAmbulance->currentGridId != assignedDepotGridId) {
        events[eventIndex].type = EventType::PREPARING_DISPATCH_TO_DEPOT;

        return;
    }

    events[eventIndex].assignedAmbulance->checkScheduledBreak(events[eventIndex].timer);
    events[eventIndex].removeAssignedAmbulance();

    events[eventIndex].type = EventType::NONE;
}

void DispatchEngineStrategy::reallocating(
    std::mt19937& rnd,
    std::vector<Ambulance>& ambulances,
    std::vector<Event>& events,
    const int eventIndex
) {
    /// TODO: code here
}

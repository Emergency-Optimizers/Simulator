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
    // done by each strategy
    return false;
}

void DispatchEngineStrategy::preparingToDispatchToScene(
    std::mt19937& rnd,
    std::vector<Ambulance>& ambulances,
    std::vector<Event>& events,
    const int eventIndex
) {
    const bool forceTrafficFactor = false;
    int incrementSeconds = ODMatrix::getInstance().getTravelTime(
        rnd,
        events[eventIndex].assignedAmbulance->currentGridId,
        events[eventIndex].gridId,
        forceTrafficFactor,
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
    const bool forceTrafficFactor = false;
    int incrementSeconds = ODMatrix::getInstance().getTravelTime(
        rnd,
        events[eventIndex].assignedAmbulance->currentGridId,
        events[eventIndex].gridId,
        forceTrafficFactor,
        events[eventIndex].triageImpression,
        events[eventIndex].prevTimer
    );

    const bool dontUpdateTimer = true;
    events[eventIndex].updateTimer(incrementSeconds, "duration_dispatching_to_scene", dontUpdateTimer);

    events[eventIndex].assignedAmbulance->currentGridId = events[eventIndex].gridId;

    // set event type to travel directly to depot if event is set to cancelled
    // cancelled here is defined as a mission which doesn't bring the patient to the hospital
    const bool cancelledEvent = events[eventIndex].secondsWaitDepartureScene == -1;
    if (!cancelledEvent) {
        incrementSeconds = static_cast<int>(events[eventIndex].secondsWaitDepartureScene);
        events[eventIndex].updateTimer(incrementSeconds, "duration_at_scene");

        events[eventIndex].type = EventType::DISPATCHING_TO_HOSPITAL;
    } else {
        incrementSeconds = static_cast<int>(events[eventIndex].secondsWaitAvailable);
        events[eventIndex].updateTimer(incrementSeconds, "duration_at_scene");

        events[eventIndex].type = EventType::PREPARING_DISPATCH_TO_DEPOT;
    }
}

void DispatchEngineStrategy::dispatchingToHospital(
    std::mt19937& rnd,
    std::vector<Ambulance>& ambulances,
    std::vector<Event>& events,
    const int eventIndex
) {
    // done by each strategy
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

    const bool forceTrafficFactor = true;
    int incrementSeconds = ODMatrix::getInstance().getTravelTime(
        rnd,
        events[eventIndex].assignedAmbulance->currentGridId,
        events[eventIndex].gridId,
        forceTrafficFactor,
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
    const bool forceTrafficFactor = true;
    int incrementSeconds = ODMatrix::getInstance().getTravelTime(
        rnd,
        events[eventIndex].assignedAmbulance->currentGridId,
        events[eventIndex].gridId,
        forceTrafficFactor,
        events[eventIndex].triageImpression,
        events[eventIndex].prevTimer
    );
    const bool dontUpdateTimer = true;
    events[eventIndex].updateTimer(incrementSeconds, "duration_dispatching_to_depot", dontUpdateTimer);
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
    // done by each strategy
}

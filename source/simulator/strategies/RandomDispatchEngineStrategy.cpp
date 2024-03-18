/**
 * @file RandomDispatchEngineStrategy.cpp
 *
 * @copyright Copyright (c) 2024 Emergency-Optimizers
 */

/* internal libraries */
#include "simulator/strategies/RandomDispatchEngineStrategy.hpp"

void RandomDispatchEngineStrategy::run(
    std::mt19937& rng,
    std::vector<Ambulance>& ambulances,
    std::vector<Event>& events,
    const int eventIndex
) {
    switch (events[eventIndex].type) {
        case EventType::ASSIGNING_AMBULANCE:
            assigningAmbulance(rng, ambulances, events, eventIndex);
            break;
        case EventType::DISPATCHING_TO_SCENE:
            dispatchingToScene(rng, ambulances, events, eventIndex);
            break;
        case EventType::DISPATCHING_TO_HOSPITAL:
            dispatchingToHospital(rng, ambulances, events, eventIndex);
            break;
        case EventType::DISPATCHING_TO_DEPOT:
            dispatchingToDepot(rng, ambulances, events, eventIndex);
            break;
        case EventType::FINISHED:
            finishingEvent(rng, ambulances, events, eventIndex);
            break;
    }
}

void RandomDispatchEngineStrategy::assigningAmbulance(
    std::mt19937& rng,
    std::vector<Ambulance>& ambulances,
    std::vector<Event>& events,
    const int eventIndex
) {
    std::vector<unsigned> availableAmbulanceIndicies = Utils::getAvailableAmbulanceIndicies(ambulances, events);

    int randomAmbulanceIndex = -1;
    while (!availableAmbulanceIndicies.empty()) {
        int randomAvailableAmbulanceIndex = Utils::getRandomInt(rng, 0, availableAmbulanceIndicies.size() - 1);
        randomAmbulanceIndex = availableAmbulanceIndicies[randomAvailableAmbulanceIndex];

        if (ambulances[randomAmbulanceIndex].assignedEventId != -1) {
            int currentAmbulanceEventIndex = Utils::findEventIndexFromId(events, ambulances[randomAmbulanceIndex].assignedEventId);

            int totalTravelTime = ODMatrix::getInstance().getTravelTime(
                ambulances[randomAmbulanceIndex].currentGridId,
                events[currentAmbulanceEventIndex].gridId
            );

            int64_t ambulanceGridId = Utils::approximateLocation(
                ambulances[randomAmbulanceIndex].currentGridId,
                events[currentAmbulanceEventIndex].gridId,
                events[currentAmbulanceEventIndex].timer - totalTravelTime,
                events[eventIndex].timer
            );

            if (!ODMatrix::getInstance().gridIdExists(ambulanceGridId)) {
                availableAmbulanceIndicies.erase(availableAmbulanceIndicies.begin() + randomAvailableAmbulanceIndex);
                continue;
            }

            events[currentAmbulanceEventIndex].metrics.dispatchToDepotTime += ODMatrix::getInstance().getTravelTime(
                ambulances[randomAmbulanceIndex].currentGridId,
                ambulanceGridId
            );

            events[currentAmbulanceEventIndex].gridId = ambulanceGridId;
            events[currentAmbulanceEventIndex].assignedAmbulanceIndex = -1;
            events[currentAmbulanceEventIndex].type = EventType::NONE;

            ambulances[randomAmbulanceIndex].currentGridId = ambulanceGridId;
        }

        break;
    }

    /// TODO: Add some time before checking again (maybe 1 second after next event
    /// so we constantly check for available ambulances) or tell the simulator to make an ambulance available.
    if (availableAmbulanceIndicies.empty()) {
        events[eventIndex].timer += 60;
        events[eventIndex].metrics.waitingForAmbulanceTime += 60;

        return;
    }

    events[eventIndex].assignedAmbulanceIndex = randomAmbulanceIndex;
    ambulances[events[eventIndex].assignedAmbulanceIndex].assignedEventId = events[eventIndex].id;
    events[eventIndex].type = EventType::DISPATCHING_TO_SCENE;
}

void RandomDispatchEngineStrategy::dispatchingToHospital(
    std::mt19937& rng,
    std::vector<Ambulance>& ambulances,
    std::vector<Event>& events,
    const int eventIndex
) {
    events[eventIndex].gridId = Stations::getInstance().get<int64_t>(
        "grid_id",
        Utils::getRandomElement(rng, Stations::getInstance().getHospitalIndices())
    );

    int incrementSeconds = ODMatrix::getInstance().getTravelTime(
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

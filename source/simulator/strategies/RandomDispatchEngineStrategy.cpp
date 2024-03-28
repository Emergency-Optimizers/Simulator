/**
 * @file RandomDispatchEngineStrategy.cpp
 *
 * @copyright Copyright (c) 2024 Emergency-Optimizers
 */

/* external libraries */
#include <algorithm>
#include <numeric>
/* internal libraries */
#include "simulator/strategies/RandomDispatchEngineStrategy.hpp"
#include "Utils.hpp"
#include "file-reader/Stations.hpp"
#include "file-reader/ODMatrix.hpp"
#include "file-reader/Settings.hpp"

void RandomDispatchEngineStrategy::run(
    std::mt19937& rng,
    std::vector<Ambulance>& ambulances,
    std::vector<Event>& events,
    const int eventIndex
) {
    switch (events[eventIndex].type) {
        case EventType::RESOURCE_APPOINTMENT:
            assigningAmbulance(rng, ambulances, events, eventIndex);
            break;
        case EventType::DISPATCHING_TO_SCENE:
            dispatchingToScene(rng, ambulances, events, eventIndex);
            break;
        case EventType::DISPATCHING_TO_HOSPITAL:
            dispatchingToHospital(rng, ambulances, events, eventIndex);
            break;
        case EventType::PREPARING_DISPATCH_TO_DEPOT:
            dispatchingToDepot(rng, ambulances, events, eventIndex);
            break;
        case EventType::DISPATCHING_TO_DEPOT:
            finishingEvent(rng, ambulances, events, eventIndex);
            break;
        case EventType::REALLOCATE:
            reallocating(rng, ambulances, events, eventIndex);
            break;
    }
}

void RandomDispatchEngineStrategy::assigningAmbulance(
    std::mt19937& rng,
    std::vector<Ambulance>& ambulances,
    std::vector<Event>& events,
    const int eventIndex
) {
    std::vector<unsigned> availableAmbulanceIndicies = getAvailableAmbulanceIndicies(
        ambulances,
        events,
        events[eventIndex].timer
    );

    int randomAmbulanceIndex = -1;
    while (!availableAmbulanceIndicies.empty()) {
        int randomAvailableAmbulanceIndex = getRandomInt(rng, 0, availableAmbulanceIndicies.size() - 1);
        randomAmbulanceIndex = availableAmbulanceIndicies[randomAvailableAmbulanceIndex];

        if (ambulances[randomAmbulanceIndex].assignedEventId != -1) {
            int currentAmbulanceEventIndex = findEventIndexFromId(events, ambulances[randomAmbulanceIndex].assignedEventId);

            int64_t ambulanceGridId = approximateLocation(
                ambulances[randomAmbulanceIndex].currentGridId,
                events[currentAmbulanceEventIndex].gridId,
                events[currentAmbulanceEventIndex].prevTimer,
                events[eventIndex].timer,
                events[currentAmbulanceEventIndex].triageImpression
            );

            if (!ODMatrix::getInstance().gridIdExists(ambulanceGridId)) {
                availableAmbulanceIndicies.erase(availableAmbulanceIndicies.begin() + randomAvailableAmbulanceIndex);
                continue;
            }

            events[currentAmbulanceEventIndex].metrics["duration_dispatching_to_depot"] += ODMatrix::getInstance().getTravelTime(
                ambulances[randomAmbulanceIndex].currentGridId,
                ambulanceGridId,
                true,
                events[currentAmbulanceEventIndex].triageImpression,
                events[currentAmbulanceEventIndex].prevTimer
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
        events[eventIndex].updateTimer(60, "duration_resource_appointment");

        return;
    }

    events[eventIndex].assignedAmbulanceIndex = randomAmbulanceIndex;
    ambulances[events[eventIndex].assignedAmbulanceIndex].assignedEventId = events[eventIndex].id;
    events[eventIndex].type = EventType::DISPATCHING_TO_SCENE;
    events[eventIndex].updateTimer(
        events[eventIndex].secondsWaitResourcePreparingDeparture,
        "duration_resource_preparing_departure"
    );
    ambulances[events[eventIndex].assignedAmbulanceIndex].timeUnavailable += events[eventIndex].secondsWaitResourcePreparingDeparture;
}

void RandomDispatchEngineStrategy::dispatchingToHospital(
    std::mt19937& rng,
    std::vector<Ambulance>& ambulances,
    std::vector<Event>& events,
    const int eventIndex
) {
    events[eventIndex].gridId = Stations::getInstance().get<int64_t>(
        "grid_id",
        getRandomElement(rng, Stations::getInstance().getHospitalIndices())
    );

    int incrementSeconds = ODMatrix::getInstance().getTravelTime(
        ambulances[events[eventIndex].assignedAmbulanceIndex].currentGridId,
        events[eventIndex].gridId,
        false,
        events[eventIndex].triageImpression,
        events[eventIndex].timer
    );
    events[eventIndex].updateTimer(incrementSeconds, "duration_dispatching_to_hospital");
    ambulances[events[eventIndex].assignedAmbulanceIndex].timeUnavailable += incrementSeconds;

    ambulances[events[eventIndex].assignedAmbulanceIndex].currentGridId = events[eventIndex].gridId;

    events[eventIndex].updateTimer(events[eventIndex].secondsWaitAvailable, "duration_at_hospital");
    ambulances[events[eventIndex].assignedAmbulanceIndex].timeUnavailable += events[eventIndex].secondsWaitAvailable;

    events[eventIndex].type = EventType::PREPARING_DISPATCH_TO_DEPOT;
}

void RandomDispatchEngineStrategy::reallocating(
    std::mt19937& rng,
    std::vector<Ambulance>& ambulances,
    std::vector<Event>& events,
    const int eventIndex
) {
    // get depot indices
    bool dayShift = isDayShift(
        events[eventIndex].timer,
        Settings::get<int>("DAY_SHIFT_START"),
        Settings::get<int>("DAY_SHIFT_END")
    );
    std::vector<unsigned int> depotIndices = Stations::getInstance().getDepotIndices(dayShift);

    // create a vector of ambulance indices and shuffle it
    std::vector<int> ambulanceIndices(ambulances.size());
    std::iota(ambulanceIndices.begin(), ambulanceIndices.end(), 0);

    std::shuffle(ambulanceIndices.begin(), ambulanceIndices.end(), rng);

    // get the new allocation
    std::vector<int> allocation = events[eventIndex].reallocation;

    // loop through
    size_t currentAmbulanceIndex = 0;
    for (size_t depotIndex = 0; depotIndex < depotIndices.size() && currentAmbulanceIndex < ambulanceIndices.size(); depotIndex++) {
        unsigned int allocatedToDepot = allocation[depotIndex];
        for (unsigned int i = 0; i < allocatedToDepot && currentAmbulanceIndex < ambulanceIndices.size(); i++) {
            /*std::cout << "Ambulance " << ambulances[ambulanceIndices[currentAmbulanceIndex]].id << ": "
                << ambulances[ambulanceIndices[currentAmbulanceIndex]].allocatedDepotIndex
                << " -> " << depotIndices[depotIndex] << std::endl;*/

            // allocate ambulance to new depot
            ambulances[ambulanceIndices[currentAmbulanceIndex]].allocatedDepotIndex = depotIndices[depotIndex];

            // branch if it isn't responding to an incident and create an event that transfers the ambulance to the new depot
            if (ambulances[ambulanceIndices[currentAmbulanceIndex]].assignedEventId == -1) {
                Event newEvent;
                newEvent.id = events.size();
                newEvent.type = EventType::PREPARING_DISPATCH_TO_DEPOT;
                newEvent.timer = events[eventIndex].timer + 1;
                newEvent.prevTimer = events[eventIndex].timer + 1;
                newEvent.assignedAmbulanceIndex = ambulanceIndices[currentAmbulanceIndex];
                newEvent.triageImpression = "V1";
                newEvent.gridId = ambulances[ambulanceIndices[currentAmbulanceIndex]].currentGridId;
                newEvent.utility = true;

                ambulances[ambulanceIndices[currentAmbulanceIndex]].assignedEventId = newEvent.id;

                events.push_back(newEvent);
            }

            currentAmbulanceIndex++;
        }
    }

    // set the type to none so it doesn't trigger again
    events[eventIndex].type = EventType::NONE;
}

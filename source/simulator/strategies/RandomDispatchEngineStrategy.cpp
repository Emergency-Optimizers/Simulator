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

bool RandomDispatchEngineStrategy::run(
    std::mt19937& rnd,
    std::vector<Ambulance>& ambulances,
    std::vector<Event>& events,
    const int eventIndex
) {
    bool sortAllEvents = false;

    switch (events[eventIndex].type) {
        case EventType::RESOURCE_APPOINTMENT:
            sortAllEvents = assigningAmbulance(rnd, ambulances, events, eventIndex);
            break;
        case EventType::PREPARING_DISPATCH_TO_SCENE:
            preparingToDispatchToScene(rnd, ambulances, events, eventIndex);
            break;
        case EventType::DISPATCHING_TO_SCENE:
            dispatchingToScene(rnd, ambulances, events, eventIndex);
            break;
        case EventType::DISPATCHING_TO_HOSPITAL:
            dispatchingToHospital(rnd, ambulances, events, eventIndex);
            break;
        case EventType::PREPARING_DISPATCH_TO_DEPOT:
            dispatchingToDepot(rnd, ambulances, events, eventIndex);
            break;
        case EventType::DISPATCHING_TO_DEPOT:
            finishingEvent(rnd, ambulances, events, eventIndex);
            break;
        case EventType::REALLOCATE:
            reallocating(rnd, ambulances, events, eventIndex);
            break;
    }

    return sortAllEvents;
}

bool RandomDispatchEngineStrategy::assigningAmbulance(
    std::mt19937& rnd,
    std::vector<Ambulance>& ambulances,
    std::vector<Event>& events,
    const int eventIndex
) {
    bool sortAllEvents = false;

    std::vector<unsigned> availableAmbulanceIndicies = getAvailableAmbulanceIndicies(
        ambulances,
        events,
        events[eventIndex].timer,
        events[eventIndex].triageImpression
    );

    int randomAmbulanceIndex = -1;
    while (!availableAmbulanceIndicies.empty()) {
        int randomAvailableAmbulanceIndex = getRandomInt(rnd, 0, static_cast<int>(availableAmbulanceIndicies.size()) - 1);
        randomAmbulanceIndex = availableAmbulanceIndicies[randomAvailableAmbulanceIndex];

        // branch if the randomly selected ambulance is assigned to an event, and perform extra checks
        if (ambulances[randomAmbulanceIndex].assignedEventId != -1) {
            int currentAmbulanceEventIndex = findEventIndexFromId(events, ambulances[randomAmbulanceIndex].assignedEventId);

            int64_t ambulanceGridId = approximateLocation(
                rnd,
                ambulances[randomAmbulanceIndex].currentGridId,
                events[currentAmbulanceEventIndex].gridId,
                events[currentAmbulanceEventIndex].prevTimer,
                events[eventIndex].timer,
                events[currentAmbulanceEventIndex].triageImpression,
                events[currentAmbulanceEventIndex].type
            );

            if (!ODMatrix::getInstance().gridIdExists(ambulanceGridId)) {
                availableAmbulanceIndicies.erase(availableAmbulanceIndicies.begin() + randomAvailableAmbulanceIndex);
                continue;
            }

            int incrementSeconds;

            if (events[currentAmbulanceEventIndex].type == EventType::DISPATCHING_TO_DEPOT) {
                const bool forceTrafficFactor = true;
                incrementSeconds = ODMatrix::getInstance().getTravelTime(
                    rnd,
                    ambulances[randomAmbulanceIndex].currentGridId,
                    ambulanceGridId,
                    forceTrafficFactor,
                    events[currentAmbulanceEventIndex].triageImpression,
                    events[currentAmbulanceEventIndex].prevTimer
                );
                const bool dontUpdateTimer = true;
                events[currentAmbulanceEventIndex].updateTimer(incrementSeconds, "duration_dispatching_to_depot", dontUpdateTimer);

                events[currentAmbulanceEventIndex].gridId = ambulanceGridId;
                events[currentAmbulanceEventIndex].type = EventType::NONE;
            } else if (events[currentAmbulanceEventIndex].type == EventType::DISPATCHING_TO_SCENE) {
                const bool forceTrafficFactor = false;
                incrementSeconds = ODMatrix::getInstance().getTravelTime(
                    rnd,
                    ambulances[randomAmbulanceIndex].currentGridId,
                    ambulanceGridId,
                    forceTrafficFactor,
                    events[currentAmbulanceEventIndex].triageImpression,
                    events[currentAmbulanceEventIndex].prevTimer
                );

                // set old event metrics to resource appointment resetting the event
                int oldMetrics = events[currentAmbulanceEventIndex].metrics["duration_resource_preparing_departure"];
                events[currentAmbulanceEventIndex].metrics["duration_resource_preparing_departure"] = 0;

                events[currentAmbulanceEventIndex].metrics["duration_resource_appointment"] += incrementSeconds + oldMetrics;
                ambulances[randomAmbulanceIndex].timeUnavailable += incrementSeconds;

                events[currentAmbulanceEventIndex].type = EventType::RESOURCE_APPOINTMENT;

                // reset timer
                int oldEventTravelTime = ODMatrix::getInstance().getTravelTime(
                    rnd,
                    ambulances[randomAmbulanceIndex].currentGridId,
                    events[currentAmbulanceEventIndex].gridId,
                    false,
                    events[currentAmbulanceEventIndex].triageImpression,
                    events[currentAmbulanceEventIndex].prevTimer
                );

                events[currentAmbulanceEventIndex].timer -= oldEventTravelTime;
                events[currentAmbulanceEventIndex].timer += incrementSeconds;

                sortAllEvents = true;
            }

            events[currentAmbulanceEventIndex].removeAssignedAmbulance();

            ambulances[randomAmbulanceIndex].currentGridId = ambulanceGridId;
        }

        break;
    }

    /// TODO: Add some time before checking again (maybe 1 second after next event
    /// so we constantly check for available ambulances) or tell the simulator to make an ambulance available.
    if (availableAmbulanceIndicies.empty()) {
        int waitTime = 60;

        const bool noEventsLeft = eventIndex + 1 >= events.size();
        if (!noEventsLeft) {
            int durationUntilNextEvent = static_cast<int>(events[eventIndex + 1].timer - events[eventIndex].timer);

            waitTime = durationUntilNextEvent + 1;
        }

        events[eventIndex].updateTimer(waitTime, "duration_resource_appointment");

        return sortAllEvents;
    }

    events[eventIndex].assignAmbulance(ambulances[randomAmbulanceIndex]);
    events[eventIndex].type = EventType::PREPARING_DISPATCH_TO_SCENE;
    events[eventIndex].updateTimer(
        static_cast<int>(events[eventIndex].secondsWaitResourcePreparingDeparture),
        "duration_resource_preparing_departure"
    );

    return sortAllEvents;
}

void RandomDispatchEngineStrategy::dispatchingToHospital(
    std::mt19937& rnd,
    std::vector<Ambulance>& ambulances,
    std::vector<Event>& events,
    const int eventIndex
) {
    events[eventIndex].gridId = Stations::getInstance().get<int64_t>(
        "grid_id",
        getRandomElement(rnd, Stations::getInstance().getHospitalIndices())
    );

    const bool forceTrafficFactor = false;
    int incrementSeconds = ODMatrix::getInstance().getTravelTime(
        rnd,
        events[eventIndex].assignedAmbulance->currentGridId,
        events[eventIndex].gridId,
        forceTrafficFactor,
        events[eventIndex].triageImpression,
        events[eventIndex].timer
    );
    events[eventIndex].updateTimer(incrementSeconds, "duration_dispatching_to_hospital");

    events[eventIndex].assignedAmbulance->currentGridId = events[eventIndex].gridId;

    events[eventIndex].updateTimer(static_cast<int>(events[eventIndex].secondsWaitAvailable), "duration_at_hospital");

    events[eventIndex].type = EventType::PREPARING_DISPATCH_TO_DEPOT;
}

void RandomDispatchEngineStrategy::reallocating(
    std::mt19937& rnd,
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

    // get the new allocation
    std::vector<int> allocation = events[eventIndex].reallocation;

    // create a vector of ambulance indices
    std::vector<int> ambulanceIndices(ambulances.size());
    std::iota(ambulanceIndices.begin(), ambulanceIndices.end(), 0);

    // remove ambulances from possible reallocation if at correct depot
    for (size_t depotIndex = 0; depotIndex < depotIndices.size(); depotIndex++) {
        for (int ambulanceIndex = 0; ambulanceIndex < ambulances.size(); ambulanceIndex++) {
            if (allocation[depotIndex] <= 0) {
                break;
            }

            if (ambulances[ambulanceIndex].allocatedDepotIndex == depotIndex) {
                ambulanceIndices.erase(std::remove(ambulanceIndices.begin(), ambulanceIndices.end(), ambulanceIndex), ambulanceIndices.end());

                allocation[depotIndex]--;
            }
        }
    }

    // shuffle the indicies to adhere to the random strategy
    std::shuffle(ambulanceIndices.begin(), ambulanceIndices.end(), rnd);

    // reallocate by assigning the ambulance indicies to the depot according to allocation vector
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
                newEvent.id = static_cast<int>(events.size());
                newEvent.type = EventType::PREPARING_DISPATCH_TO_DEPOT;
                newEvent.timer = events[eventIndex].timer;
                newEvent.prevTimer = events[eventIndex].timer;
                newEvent.assignAmbulance(ambulances[ambulanceIndices[currentAmbulanceIndex]]);
                newEvent.triageImpression = "V1";
                newEvent.gridId = ambulances[ambulanceIndices[currentAmbulanceIndex]].currentGridId;
                newEvent.utility = true;

                events.insert(events.begin() + eventIndex + 1, newEvent);
            }

            currentAmbulanceIndex++;
        }
    }

    // set the type to none so it doesn't trigger again
    events[eventIndex].type = EventType::NONE;
}

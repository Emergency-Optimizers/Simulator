/**
 * @file ClosestDispatchEngineStrategy.cpp
 *
 * @copyright Copyright (c) 2024 Emergency-Optimizers
 */

/* external libraries */
#include <limits>
#include <algorithm>
#include <numeric>
/* internal libraries */
#include "simulator/strategies/ClosestDispatchEngineStrategy.hpp"
#include "Utils.hpp"
#include "file-reader/Stations.hpp"
#include "file-reader/Settings.hpp"
#include "file-reader/ODMatrix.hpp"

bool ClosestDispatchEngineStrategy::run(
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

bool ClosestDispatchEngineStrategy::assigningAmbulance(
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
    /// TODO: Add some time before checking again (maybe 1 second after next event
    /// so we constantly check for available ambulances) or tell the simulator to make an ambulance available.
    if (availableAmbulanceIndicies.empty()) {
        events[eventIndex].updateTimer(60, "duration_resource_appointment");

        return sortAllEvents;
    }

    // find closest ambulance
    int closestAmbulanceIndex = -1;
    int64_t closestAmbulanceGridId = -1;
    int closestAmbulanceTravelTime = std::numeric_limits<int>::max();
    int64_t eventGridId = events[eventIndex].gridId;
    // std::pair<int, int> utm1 = idToUtm(eventGridId);
    for (int i = 0; i < availableAmbulanceIndicies.size(); i++) {
        int64_t ambulanceGridId;

        if (ambulances[availableAmbulanceIndicies[i]].assignedEventId != -1) {
            int currentAmbulanceEventIndex = findEventIndexFromId(events, ambulances[availableAmbulanceIndicies[i]].assignedEventId);

            ambulanceGridId = approximateLocation(
                ambulances[availableAmbulanceIndicies[i]].currentGridId,
                events[currentAmbulanceEventIndex].gridId,
                events[currentAmbulanceEventIndex].prevTimer,
                events[eventIndex].timer,
                events[currentAmbulanceEventIndex].triageImpression
            );

            if (!ODMatrix::getInstance().gridIdExists(ambulanceGridId)) {
                continue;
            }
        } else {
            ambulanceGridId = ambulances[availableAmbulanceIndicies[i]].currentGridId;
        }

        // std::pair<int, int> utm2 = idToUtm(ambulanceGridId);
        // int travelTime = calculateEuclideanDistance(utm1.first, utm1.second, utm2.first, utm2.second);
        int travelTime = ODMatrix::getInstance().getTravelTime(
            ambulanceGridId,
            eventGridId,
            false,
            events[eventIndex].triageImpression,
            events[eventIndex].timer
        );

        if (travelTime < closestAmbulanceTravelTime) {
            closestAmbulanceIndex = availableAmbulanceIndicies[i];
            closestAmbulanceGridId = ambulanceGridId;
            closestAmbulanceTravelTime = travelTime;
        }
    }

    if (closestAmbulanceIndex == -1) {
        events[eventIndex].updateTimer(60, "duration_resource_appointment");

        return sortAllEvents;
    }

    if (ambulances[closestAmbulanceIndex].assignedEventId != -1) {
        int currentAmbulanceEventIndex = findEventIndexFromId(events, ambulances[closestAmbulanceIndex].assignedEventId);
        int incrementSeconds;

        if (events[currentAmbulanceEventIndex].type == EventType::DISPATCHING_TO_DEPOT) {
            incrementSeconds = ODMatrix::getInstance().getTravelTime(
                ambulances[closestAmbulanceIndex].currentGridId,
                closestAmbulanceGridId,
                true,
                events[currentAmbulanceEventIndex].triageImpression,
                events[currentAmbulanceEventIndex].prevTimer
            );
            events[currentAmbulanceEventIndex].metrics["duration_dispatching_to_depot"] += incrementSeconds;

            events[currentAmbulanceEventIndex].gridId = closestAmbulanceGridId;
            events[currentAmbulanceEventIndex].type = EventType::NONE;
        } else if (events[currentAmbulanceEventIndex].type == EventType::DISPATCHING_TO_SCENE) {
            incrementSeconds = ODMatrix::getInstance().getTravelTime(
                ambulances[closestAmbulanceIndex].currentGridId,
                closestAmbulanceGridId,
                false,
                events[currentAmbulanceEventIndex].triageImpression,
                events[currentAmbulanceEventIndex].prevTimer
            );

            // set old event metrics to resource appointment resetting the event
            int oldMetrics = events[currentAmbulanceEventIndex].metrics["duration_resource_preparing_departure"];
            events[currentAmbulanceEventIndex].metrics["duration_resource_preparing_departure"] = 0;

            events[currentAmbulanceEventIndex].metrics["duration_resource_appointment"] += incrementSeconds + oldMetrics;

            events[currentAmbulanceEventIndex].type = EventType::RESOURCE_APPOINTMENT;

            // reset timer
            int oldEventTravelTime = ODMatrix::getInstance().getTravelTime(
                ambulances[closestAmbulanceIndex].currentGridId,
                events[currentAmbulanceEventIndex].gridId,
                false,
                events[currentAmbulanceEventIndex].triageImpression,
                events[currentAmbulanceEventIndex].prevTimer
            );

            events[currentAmbulanceEventIndex].timer -= oldEventTravelTime;
            events[currentAmbulanceEventIndex].timer += incrementSeconds;

            sortAllEvents = true;
        }

        events[currentAmbulanceEventIndex].assignedAmbulanceIndex = -1;

        ambulances[closestAmbulanceIndex].timeUnavailable += incrementSeconds;
        ambulances[closestAmbulanceIndex].currentGridId = closestAmbulanceGridId;
    }

    events[eventIndex].assignedAmbulanceIndex = closestAmbulanceIndex;
    ambulances[closestAmbulanceIndex].assignedEventId = events[eventIndex].id;
    events[eventIndex].type = EventType::PREPARING_DISPATCH_TO_SCENE;
    events[eventIndex].updateTimer(
        events[eventIndex].secondsWaitResourcePreparingDeparture,
        "duration_resource_preparing_departure"
    );
    ambulances[events[eventIndex].assignedAmbulanceIndex].timeUnavailable += events[eventIndex].secondsWaitResourcePreparingDeparture;

    return sortAllEvents;
}

void ClosestDispatchEngineStrategy::dispatchingToHospital(
    std::mt19937& rnd,
    std::vector<Ambulance>& ambulances,
    std::vector<Event>& events,
    const int eventIndex
) {
    // find closest hospital
    int closestHospitalIndex = -1;
    int closestHospitalTravelTime = std::numeric_limits<int>::max();
    int64_t eventGridId = events[eventIndex].gridId;
    std::vector<unsigned int> hospitals = Stations::getInstance().getHospitalIndices();
    for (int i = 0; i < hospitals.size(); i++) {
        int64_t hospitalGridId = Stations::getInstance().get<int64_t>(
            "grid_id",
            hospitals[i]
        );
        int travelTime = ODMatrix::getInstance().getTravelTime(
            hospitalGridId,
            eventGridId,
            false,
            events[eventIndex].triageImpression,
            events[eventIndex].timer
        );
        if (travelTime < closestHospitalTravelTime) {
            closestHospitalIndex = i;
            closestHospitalTravelTime = travelTime;
        }
    }

    events[eventIndex].gridId = Stations::getInstance().get<int64_t>(
        "grid_id",
        hospitals[closestHospitalIndex]
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

void ClosestDispatchEngineStrategy::reallocating(
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

    // sort the indicies to adhere to the closest strategy
    std::vector<int> sortedAmbulanceIndices;
    for (size_t depotIndex = 0; depotIndex < depotIndices.size(); depotIndex++) {
        unsigned int allocatedToDepot = allocation[depotIndex];
        for (unsigned int i = 0; i < allocatedToDepot; i++) {
            int closestAmbulanceIndex = 0;
            int closestTravelTime = std::numeric_limits<int>::max();
            for (int ambulanceIndex = 0; ambulanceIndex < ambulanceIndices.size(); ambulanceIndex++) {
                int64_t depotGridId = Stations::getInstance().get<int64_t>(
                    "grid_id",
                    depotIndices[depotIndex]
                );

                int travelTime = ODMatrix::getInstance().getTravelTime(
                    ambulances[ambulanceIndices[ambulanceIndex]].currentGridId,
                    depotGridId,
                    true,
                    "V1",
                    events[eventIndex].timer
                );

                if (travelTime < closestTravelTime) {
                    closestAmbulanceIndex = ambulanceIndex;
                    closestTravelTime = travelTime;

                    if (closestTravelTime == 0) {
                        break;
                    }
                }
            }

            sortedAmbulanceIndices.push_back(ambulanceIndices[closestAmbulanceIndex]);
            ambulanceIndices.erase(ambulanceIndices.begin() + closestAmbulanceIndex);
        }
    }

    // reallocate by assigning the ambulance indicies to the depot according to allocation vector
    size_t currentAmbulanceIndex = 0;
    for (size_t depotIndex = 0; depotIndex < depotIndices.size() && currentAmbulanceIndex < sortedAmbulanceIndices.size(); depotIndex++) {
        unsigned int allocatedToDepot = allocation[depotIndex];
        for (unsigned int i = 0; i < allocatedToDepot && currentAmbulanceIndex < sortedAmbulanceIndices.size(); i++) {
            /*std::cout << "Ambulance " << ambulances[sortedAmbulanceIndices[currentAmbulanceIndex]].id << ": "
                << ambulances[sortedAmbulanceIndices[currentAmbulanceIndex]].allocatedDepotIndex
                << " -> " << depotIndices[depotIndex] << std::endl;*/

            // allocate ambulance to new depot
            ambulances[sortedAmbulanceIndices[currentAmbulanceIndex]].allocatedDepotIndex = depotIndices[depotIndex];

            // branch if it isn't responding to an incident and create an event that transfers the ambulance to the new depot
            if (ambulances[sortedAmbulanceIndices[currentAmbulanceIndex]].assignedEventId == -1) {
                Event newEvent;
                newEvent.id = events.size();
                newEvent.type = EventType::PREPARING_DISPATCH_TO_DEPOT;
                newEvent.timer = events[eventIndex].timer;
                newEvent.prevTimer = events[eventIndex].timer;
                newEvent.assignedAmbulanceIndex = sortedAmbulanceIndices[currentAmbulanceIndex];
                newEvent.triageImpression = "V1";
                newEvent.gridId = ambulances[sortedAmbulanceIndices[currentAmbulanceIndex]].currentGridId;
                newEvent.utility = true;

                ambulances[sortedAmbulanceIndices[currentAmbulanceIndex]].assignedEventId = newEvent.id;

                events.insert(events.begin() + eventIndex + 1, newEvent);
            }

            currentAmbulanceIndex++;
        }
    }

    // set the type to none so it doesn't trigger again
    events[eventIndex].type = EventType::NONE;
}

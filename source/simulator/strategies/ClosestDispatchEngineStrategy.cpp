/**
 * @file ClosestDispatchEngineStrategy.cpp
 *
 * @copyright Copyright (c) 2024 Emergency-Optimizers
 */

/* external libraries */
#include <limits>
/* internal libraries */
#include "simulator/strategies/ClosestDispatchEngineStrategy.hpp"
#include "Utils.hpp"
#include "file-reader/Stations.hpp"
#include "file-reader/ODMatrix.hpp"

void ClosestDispatchEngineStrategy::run(
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

void ClosestDispatchEngineStrategy::assigningAmbulance(
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
    /// TODO: Add some time before checking again (maybe 1 second after next event
    /// so we constantly check for available ambulances) or tell the simulator to make an ambulance available.
    if (availableAmbulanceIndicies.empty()) {
        events[eventIndex].updateTimer(60, "duration_resource_appointment");

        return;
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

    if (ambulances[closestAmbulanceIndex].assignedEventId != -1) {
        int currentAmbulanceEventIndex = findEventIndexFromId(events, ambulances[closestAmbulanceIndex].assignedEventId);

        int incrementSeconds = ODMatrix::getInstance().getTravelTime(
            ambulances[closestAmbulanceIndex].currentGridId,
            closestAmbulanceGridId,
            true,
            events[currentAmbulanceEventIndex].triageImpression,
            events[currentAmbulanceEventIndex].prevTimer
        );
        events[currentAmbulanceEventIndex].metrics["duration_dispatching_to_depot"] += incrementSeconds;
        ambulances[closestAmbulanceIndex].timeUnavailable += incrementSeconds;

        events[currentAmbulanceEventIndex].gridId = closestAmbulanceGridId;
        events[currentAmbulanceEventIndex].assignedAmbulanceIndex = -1;
        events[currentAmbulanceEventIndex].type = EventType::NONE;

        ambulances[closestAmbulanceIndex].currentGridId = closestAmbulanceGridId;
    }

    events[eventIndex].assignedAmbulanceIndex = closestAmbulanceIndex;
    ambulances[closestAmbulanceIndex].assignedEventId = events[eventIndex].id;
    events[eventIndex].type = EventType::DISPATCHING_TO_SCENE;
    events[eventIndex].updateTimer(
        events[eventIndex].secondsWaitResourcePreparingDeparture,
        "duration_resource_preparing_departure"
    );
    ambulances[events[eventIndex].assignedAmbulanceIndex].timeUnavailable += events[eventIndex].secondsWaitResourcePreparingDeparture;
}

void ClosestDispatchEngineStrategy::dispatchingToHospital(
    std::mt19937& rng,
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

    events[eventIndex].type = EventType::DISPATCHING_TO_DEPOT;
}

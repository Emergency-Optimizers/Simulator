/**
 * @file Event.cpp
 *
 * @copyright Copyright (c) 2024 Emergency-Optimizers
 */

/* internal libraries */
#include "simulator/Event.hpp"
#include "simulator/Ambulance.hpp"

void Event::updateTimer(const int increment, const std::string& metric, const bool dontUpdateTimer) {
    if (!dontUpdateTimer) {
        // set previous timer to handle reassignment events
        prevTimer = timer;
        timer += increment;
    }

    // update metrics if defined, used when writing events.csv file for analysing
    if (!metric.empty()) {
        if (metrics[metric] == -1) {
            metrics[metric] = increment;
        } else {
            metrics[metric] += increment;
        }

        // update ambulance UHU if applicable
        bool updateAmbulance = metric == "duration_resource_preparing_departure";
        updateAmbulance |= metric == "duration_dispatching_to_scene";
        updateAmbulance |= metric == "duration_at_scene";
        updateAmbulance |= metric == "duration_dispatching_to_hospital";
        updateAmbulance |= metric == "duration_at_hospital";
        updateAmbulance |= metric == "duration_dispatching_to_depot";

        if (assignedAmbulance != nullptr && updateAmbulance) {
            assignedAmbulance->timeUnavailable += increment;
        }
    }
}

int Event::getResponseTime() {
    // calculate response time for event
    int responseTime = metrics["duration_incident_creation"];
    responseTime += metrics["duration_resource_appointment"];
    responseTime += metrics["duration_resource_preparing_departure"];
    responseTime += metrics["duration_dispatching_to_scene"];

    return responseTime;
}


void Event::removeAssignedAmbulance() {
    if (assignedAmbulance != nullptr) {
        assignedAmbulance->assignedEventId = -1;
        assignedAmbulance = nullptr;
    }
}

void Event::assignAmbulance(Ambulance& ambulance) {
    // make sure to deassign any old ambulances if needed
    if (assignedAmbulance != nullptr) {
        assignedAmbulance->assignedEventId = -1;
    }

    assignedAmbulance = &ambulance;
    assignedAmbulance->assignedEventId = id;
    depotIndexResponsible = assignedAmbulance->allocatedDepotIndex;
    ambulanceIdResponsible = assignedAmbulance->id;
    allocationIndex = assignedAmbulance->currentAllocationIndex;
}

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
        prevTimer = timer;
        timer += increment;
    }

    if (!metric.empty()) {
        metrics[metric] += increment;

        bool updateAmbulance = metric == "duration_resource_preparing_departure";
        updateAmbulance |= metric == "duration_dispatching_to_scene";
        updateAmbulance |= metric == "duration_at_scene";
        updateAmbulance |= metric == "duration_dispatching_to_hospital";
        updateAmbulance |= metric == "duration_at_hospital";

        if (updateAmbulance) {
            assignedAmbulance->timeUnavailable += increment;
        }
    }
}

int Event::getResponseTime() {
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
    if (assignedAmbulance != nullptr) {
        assignedAmbulance->assignedEventId = -1;
    }

    assignedAmbulance = &ambulance;
    assignedAmbulance->assignedEventId = id;
    depotIndexResponsible = assignedAmbulance->allocatedDepotIndex;
}

/**
 * @file Event.hpp
 *
 * @copyright Copyright (c) 2024 Emergency-Optimizers
 */

#pragma once

/* external libraries */
#include <ctime>
#include <iostream>
#include <iomanip>
#include <sstream>
#include <string>
#include <map>
#include <vector>
/* internal libraries */
#include "simulator/EventType.hpp"

struct Event {
    int id = -1;
    EventType type = EventType::ASSIGNING_AMBULANCE;
    std::time_t timer;
    std::time_t prevTimer = 0;
    int assignedAmbulanceIndex = -1;
    std::map<std::string, int> metrics = {
        {"duration_incident_creation", 0},
        {"duration_resource_appointment", 0},
        {"duration_resource_preparing_departure", 0},
        {"duration_dispatching_to_scene", 0},
        {"duration_at_scene", 0},
        {"duration_dispatching_to_hospital", 0},
        {"duration_at_hospital", 0},
        {"duration_dispatching_to_depot", 0},
    };
    std::string triageImpression;
    std::tm callReceived;
    float secondsWaitCallAnswered = -1.0f;
    float secondsWaitAppointingResource = -1.0f;
    float secondsWaitResourcePreparingDeparture = -1.0f;
    float secondsWaitDepartureScene = -1.0f;
    float secondsWaitAvailable = -1.0f;
    int64_t gridId = -1;
    int64_t incidentGridId = -1;

    void updateTimer(const int increment, const std::string& metric = "") {
        prevTimer = timer;
        timer += increment;

        if (!metric.empty()) {
            metrics[metric] += increment;
        }
    }

    float getResponseTime() {
        float responseTime = metrics["duration_incident_creation"];
        responseTime += metrics["duration_resource_appointment"];
        responseTime += metrics["duration_resource_preparing_departure"];
        responseTime += metrics["duration_dispatching_to_scene"];

        return responseTime;
    }
};

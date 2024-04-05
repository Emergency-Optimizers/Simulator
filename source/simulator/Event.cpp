/**
 * @file Event.cpp
 *
 * @copyright Copyright (c) 2024 Emergency-Optimizers
 */

/* internal libraries */
#include "simulator/Event.hpp"

void Event::updateTimer(const int increment, const std::string& metric) {
    prevTimer = timer;
    timer += increment;

    if (!metric.empty()) {
        metrics[metric] += increment;
    }
}

float Event::getResponseTime() {
    float responseTime = metrics["duration_incident_creation"];
    responseTime += metrics["duration_resource_appointment"];
    responseTime += metrics["duration_resource_preparing_departure"];
    responseTime += metrics["duration_dispatching_to_scene"];

    return responseTime;
}

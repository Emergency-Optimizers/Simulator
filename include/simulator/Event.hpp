/**
 * @file Event.hpp
 *
 * @copyright Copyright (c) 2024 Emergency-Optimizers
 */

#pragma once

/* external libraries */
#include <string>
#include <map>
#include <vector>
#include <ctime>
/* internal libraries */
#include "simulator/EventType.hpp"

struct Ambulance;

struct Event {
    int id = -1;
    EventType type = EventType::RESOURCE_APPOINTMENT;
    std::time_t timer;
    std::time_t prevTimer = 0;
    Ambulance* assignedAmbulance = nullptr;
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
    double secondsWaitCallAnswered = -1.0;
    double secondsWaitAppointingResource = -1.0;
    double secondsWaitResourcePreparingDeparture = -1.0;
    double secondsWaitDepartureScene = -1.0;
    double secondsWaitAvailable = -1.0;
    int64_t gridId = -1LL;
    int64_t incidentGridId = -1LL;
    int depotIndexResponsible = -1;
    std::vector<int> reallocation;
    bool utility = false;

    void updateTimer(const int increment, const std::string& metric = "", const bool dontUpdateTimer = false);
    int getResponseTime();
    void removeAssignedAmbulance();
    void assignAmbulance(Ambulance& ambulance);
};

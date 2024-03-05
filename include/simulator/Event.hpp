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

/* internal libraries */
#include "simulator/EventPerformanceMetrics.hpp"
#include "simulator/EventType.hpp"

struct Event {
    EventType type = EventType::ASSIGNING_AMBULANCE;
    std::time_t timer;
    int assignedAmbulanceIndex = -1;
    EventPerformanceMetrics metrics;
    std::string triageImpression;
    std::tm callReceived;
    float secondsWaitCallAnswered = -1;
    float secondsWaitDepartureScene = -1;
    float secondsWaitAvailable = -1;
    int64_t gridId = -1;

    void print() {
        std::cout << "triageImpression: " << triageImpression << std::endl;
        std::cout << "callReceived: " << tmToString(callReceived) << std::endl;
        std::cout << "secondsWaitCallAnswered: " << secondsWaitCallAnswered << std::endl;
        std::cout << "secondsWaitDepartureScene: " << secondsWaitDepartureScene << std::endl;
        std::cout << "secondsWaitAvailable: " << secondsWaitAvailable << std::endl;
        std::cout << "gridId: " << gridId << std::endl;
    }

    std::string tmToString(const std::tm& time) const {
        std::stringstream ss;
        ss << std::put_time(&time, "%Y-%m-%d %H:%M:%S");
        return ss.str();
    }
};

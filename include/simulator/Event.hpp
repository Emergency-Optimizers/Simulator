/**
 * @file Event.hpp
 *
 * @copyright Copyright (c) 2024 Emergency-Optimizers
 */

#pragma once

/* external libraries */
#include <string>
#include <ctime>
#include <iostream>
/* internal libraries */
#include "Utils.hpp"

struct Event {
    std::string triageImpression;
    std::tm callReceived;
    float secondsWaitCallAnswered = -1;
    float secondsWaitAmbulanceNotified = -1;
    float secondsWaitAmbulanceDispatch = -1;
    float secondsWaitDepartureScene = -1;
    float secondsWaitAvailable = -1;
    int64_t gridId = -1;

    void print() {
        std::cout << "triageImpression: " << triageImpression << std::endl;
        std::cout << "callReceived: " << Utils::tmToString(callReceived) << std::endl;
        std::cout << "secondsWaitCallAnswered: " << secondsWaitCallAnswered << std::endl;
        std::cout << "secondsWaitAmbulanceNotified: " << secondsWaitAmbulanceNotified << std::endl;
        std::cout << "secondsWaitAmbulanceDispatch: " << secondsWaitAmbulanceDispatch << std::endl;
        std::cout << "secondsWaitDepartureScene: " << secondsWaitDepartureScene << std::endl;
        std::cout << "secondsWaitAvailable: " << secondsWaitAvailable << std::endl;
        std::cout << "gridId: " << gridId << std::endl;
    }
};

/**
 * @file DispatchEngineStrategy.hpp
 *
 * @copyright Copyright (c) 2024 Emergency-Optimizers
 */

#pragma once

/* external libraries */
#include <vector>
#include <random>
/* internal libraries */
#include "simulator/Ambulance.hpp"
#include "simulator/Event.hpp"

class DispatchEngineStrategy {
 protected:
    static bool assigningAmbulance(
        std::mt19937& rnd,
        std::vector<Ambulance>& ambulances,
        std::vector<Event>& events,
        const int eventIndex
    );
    static void preparingToDispatchToScene(
        std::mt19937& rnd,
        std::vector<Ambulance>& ambulances,
        std::vector<Event>& events,
        const int eventIndex
    );
    static void dispatchingToScene(
        std::mt19937& rnd,
        std::vector<Ambulance>& ambulances,
        std::vector<Event>& events,
        const int eventIndex
    );
    static void dispatchingToHospital(
        std::mt19937& rnd,
        std::vector<Ambulance>& ambulances,
        std::vector<Event>& events,
        const int eventIndex
    );
    static void dispatchingToDepot(
        std::mt19937& rnd,
        std::vector<Ambulance>& ambulances,
        std::vector<Event>& events,
        const int eventIndex
    );
    static void finishingEvent(
        std::mt19937& rnd,
        std::vector<Ambulance>& ambulances,
        std::vector<Event>& events,
        const int eventIndex
    );
    static void reallocating(
        std::mt19937& rnd,
        std::vector<Ambulance>& ambulances,
        std::vector<Event>& events,
        const int eventIndex
    );

 public:
    static bool run(
        std::mt19937& rnd,
        std::vector<Ambulance>& ambulances,
        std::vector<Event>& events,
        const int eventIndex
    );
};

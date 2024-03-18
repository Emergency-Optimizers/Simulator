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
#include "simulator/Incidents.hpp"
#include "simulator/Stations.hpp"
#include "simulator/ODMatrix.hpp"

class DispatchEngineStrategy {
 protected:
    static void assigningAmbulance(
        std::mt19937& rng,
        std::vector<Ambulance>& ambulances,
        std::vector<Event>& events,
        const int eventIndex
    );
    static void dispatchingToScene(
        std::mt19937& rng,
        std::vector<Ambulance>& ambulances,
        std::vector<Event>& events,
        const int eventIndex
    );
    static void dispatchingToHospital(
        std::mt19937& rng,
        std::vector<Ambulance>& ambulances,
        std::vector<Event>& events,
        const int eventIndex
    );
    static void dispatchingToDepot(
        std::mt19937& rng,
        std::vector<Ambulance>& ambulances,
        std::vector<Event>& events,
        const int eventIndex
    );
    static void finishingEvent(
        std::mt19937& rng,
        std::vector<Ambulance>& ambulances,
        std::vector<Event>& events,
        const int eventIndex
    );

 public:
    static void run(
        std::mt19937& rng,
        std::vector<Ambulance>& ambulances,
        std::vector<Event>& events,
        const int eventIndex
    );
};

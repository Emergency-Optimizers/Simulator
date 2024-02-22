/**
 * @file RandomDispatchEngineStrategy.hpp
 *
 * @copyright Copyright (c) 2024 Emergency-Optimizers
 */

#pragma once

/* external libraries */
#include <vector>
#include <random>
/* internal libraries */
#include "simulator/strategies/DispatchEngineStrategy.hpp"

class RandomDispatchEngineStrategy : public DispatchEngineStrategy {
 protected:
    static bool assignAmbulance(
        std::mt19937& rng,
        Incidents& incidents,
        Stations& stations,
        ODMatrix& odMatrix,
        std::vector<Ambulance>& ambulances,
        std::vector<Event>& events,
        const int eventIndex
    );
    static void callProcessed(
        std::mt19937& rng,
        Incidents& incidents,
        Stations& stations,
        ODMatrix& odMatrix,
        std::vector<Ambulance>& ambulances,
        std::vector<Event>& events,
        const int eventIndex
    );
    static void dispatchToScene(
        std::mt19937& rng,
        Incidents& incidents,
        Stations& stations,
        ODMatrix& odMatrix,
        std::vector<Ambulance>& ambulances,
        std::vector<Event>& events,
        const int eventIndex
    );
    static void arrivedAtScene(
        std::mt19937& rng,
        Incidents& incidents,
        Stations& stations,
        ODMatrix& odMatrix,
        std::vector<Ambulance>& ambulances,
        std::vector<Event>& events,
        const int eventIndex
    );
    static void dispatchToHospital(
        std::mt19937& rng,
        Incidents& incidents,
        Stations& stations,
        ODMatrix& odMatrix,
        std::vector<Ambulance>& ambulances,
        std::vector<Event>& events,
        const int eventIndex
    );
    static void arrivedAtHospital(
        std::mt19937& rng,
        Incidents& incidents,
        Stations& stations,
        ODMatrix& odMatrix,
        std::vector<Ambulance>& ambulances,
        std::vector<Event>& events,
        const int eventIndex
    );
    static void dispatchToDepot(
        std::mt19937& rng,
        Incidents& incidents,
        Stations& stations,
        ODMatrix& odMatrix,
        std::vector<Ambulance>& ambulances,
        std::vector<Event>& events,
        const int eventIndex
    );

 public:
    static void run(
        std::mt19937& rng,
        Incidents& incidents,
        Stations& stations,
        ODMatrix& odMatrix,
        std::vector<Ambulance>& ambulances,
        std::vector<Event>& events,
        const int eventIndex
    );
};

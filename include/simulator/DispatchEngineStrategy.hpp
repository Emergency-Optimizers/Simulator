/**
 * @file DispatchEngineStrategy.hpp
 * @author Sindre Eiklid
 * @version 1.0
 * @date 2024-01-18
 *
 * @copyright Copyright (c) 2024 Sindre Eiklid
 */

#pragma once

/* external libraries */
#include <vector>
#include <random>
/* internal libraries */
#include "simulator/Ambulance.hpp"
#include "simulator/EventOld.hpp"
#include "simulator/Incidents.hpp"
#include "simulator/Stations.hpp"
#include "simulator/ODMatrix.hpp"

class DispatchEngineStrategy {
 protected:
    static int assignAmbulance(
        std::mt19937& rng,
        Incidents& incidents,
        Stations& stations,
        ODMatrix& odMatrix,
        std::vector<Ambulance>& ambulances,
        EventOld& event
    );
    static void callProcessed(
        std::mt19937& rng,
        Incidents& incidents,
        Stations& stations,
        ODMatrix& odMatrix,
        std::vector<Ambulance>& ambulances,
        EventOld& event
    );
    static void dispatchToScene(
        std::mt19937& rng,
        Incidents& incidents,
        Stations& stations,
        ODMatrix& odMatrix,
        std::vector<Ambulance>& ambulances,
        EventOld& event
    );
    static void arrivedAtScene(
        std::mt19937& rng,
        Incidents& incidents,
        Stations& stations,
        ODMatrix& odMatrix,
        std::vector<Ambulance>& ambulances,
        EventOld& event
    );
    static void dispatchToHospital(
        std::mt19937& rng,
        Incidents& incidents,
        Stations& stations,
        ODMatrix& odMatrix,
        std::vector<Ambulance>& ambulances,
        EventOld& event
    );
    static void arrivedAtHospital(
        std::mt19937& rng,
        Incidents& incidents,
        Stations& stations,
        ODMatrix& odMatrix,
        std::vector<Ambulance>& ambulances,
        EventOld& event
    );
    static void dispatchToDepot(
        std::mt19937& rng,
        Incidents& incidents,
        Stations& stations,
        ODMatrix& odMatrix,
        std::vector<Ambulance>& ambulances,
        EventOld& event
    );

 public:
    static void run(
        std::mt19937& rng,
        Incidents& incidents,
        Stations& stations,
        ODMatrix& odMatrix,
        std::vector<Ambulance>& ambulances,
        EventOld& event
    );
};

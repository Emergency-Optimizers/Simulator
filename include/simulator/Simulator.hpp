/**
 * @file Simulator.hpp
 * @author Sindre Eiklid
 * @version 1.0
 * @date 2024-01-06
 *
 * @copyright Copyright (c) 2023 Sindre Eiklid
 */

#pragma once

/* external libraries */
#include <random>
#include <vector>
#include <string>
/* internal libraries */
#include "simulator/Incidents.hpp"
#include "simulator/Stations.hpp"
#include "simulator/AmbulanceAllocator.hpp"
#include "simulator/EventHandler.hpp"
#include "simulator/DispatchEngineStrategyType.hpp"
#include "simulator/ODMatrix.hpp"

class Simulator {
 private:
    std::mt19937& rng;
    Incidents& incidents;
    Stations& stations;
    ODMatrix& odMatrix;
    AmbulanceAllocator& ambulanceAllocator;
    DispatchEngineStrategyType dispatchStrategy;
    EventHandler eventHandler;

 public:
    Simulator(
        std::mt19937& rng,
        Incidents& incidents,
        Stations& stations,
        ODMatrix& odMatrix,
        AmbulanceAllocator& ambulanceAllocator,
        DispatchEngineStrategyType dispatchStrategy,
        std::vector<Event> events
    );
    void run();
    double getResponseTime();
    void printAverageEventPerformanceMetrics();
};

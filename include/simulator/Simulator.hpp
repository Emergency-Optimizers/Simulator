/**
 * @file Simulator.hpp
 *
 * @copyright Copyright (c) 2024 Emergency-Optimizers
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
    void run(bool saveMetricsToFile = false);
    double getResponseTime();
    void printAverageEventPerformanceMetrics();
};

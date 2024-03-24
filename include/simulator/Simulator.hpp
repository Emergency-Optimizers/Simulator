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
#include "file-reader/Incidents.hpp"
#include "file-reader/Stations.hpp"
#include "file-reader/ODMatrix.hpp"
#include "simulator/AmbulanceAllocator.hpp"
#include "simulator/EventHandler.hpp"
#include "simulator/strategies/DispatchEngineStrategyType.hpp"

class Simulator {
 private:
    std::mt19937& rng;
    AmbulanceAllocator& ambulanceAllocator;
    EventHandler eventHandler;
    DispatchEngineStrategyType dispatchStrategy;

 public:
    Simulator(
        std::mt19937& rng,
        AmbulanceAllocator& ambulanceAllocator,
        DispatchEngineStrategyType dispatchStrategy,
        std::vector<Event> events

    );
    void run(bool saveMetricsToFile = false);
    double averageResponseTime(const std::string& triageImpression, bool urban);
    double responseTimeViolations();
    void printAverageEventPerformanceMetrics();
};

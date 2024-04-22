/**
 * @file Simulator.hpp
 *
 * @copyright Copyright (c) 2024 Emergency-Optimizers
 */

#pragma once

/* external libraries */
#include <random>
#include <vector>
/* internal libraries */
#include "file-reader/Settings.hpp"
#include "simulator/AmbulanceAllocator.hpp"
#include "simulator/EventHandler.hpp"
#include "simulator/strategies/DispatchEngineStrategyType.hpp"

class Simulator {
 private:
    std::mt19937 rnd = std::mt19937(Settings::get<int>("SEED"));
    AmbulanceAllocator& ambulanceAllocator;
    EventHandler eventHandler;
    DispatchEngineStrategyType dispatchStrategy;

 public:
    Simulator(
        AmbulanceAllocator& ambulanceAllocator,
        DispatchEngineStrategyType dispatchStrategy,
        std::vector<Event> events
    );
    std::vector<Event> run();
};

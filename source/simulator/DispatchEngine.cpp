/**
 * @file DispatchEngine.cpp
 *
 * @copyright Copyright (c) 2024 Emergency-Optimizers
 */

/* internal libraries */
#include "simulator/DispatchEngine.hpp"
#include "simulator/strategies/RandomDispatchEngineStrategy.hpp"

void DispatchEngine::dispatch(
    const DispatchEngineStrategyType strategy,
    std::mt19937& rng,
    Incidents& incidents,
    Stations& stations,
    ODMatrix& odMatrix,
    std::vector<Ambulance>& ambulances,
    std::vector<Event>& events,
    const int eventIndex
) {
    switch (strategy) {
        default:
            RandomDispatchEngineStrategy::run(
                rng,
                incidents,
                stations,
                odMatrix,
                ambulances,
                events,
                eventIndex
            );
    }
}

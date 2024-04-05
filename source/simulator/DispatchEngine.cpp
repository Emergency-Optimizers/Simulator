/**
 * @file DispatchEngine.cpp
 *
 * @copyright Copyright (c) 2024 Emergency-Optimizers
 */

/* internal libraries */
#include "simulator/DispatchEngine.hpp"
#include "simulator/strategies/RandomDispatchEngineStrategy.hpp"
#include "simulator/strategies/ClosestDispatchEngineStrategy.hpp"

void DispatchEngine::dispatch(
    const DispatchEngineStrategyType strategy,
    std::mt19937& rng,
    std::vector<Ambulance>& ambulances,
    std::vector<Event>& events,
    const int eventIndex
) {
    switch (strategy) {
        case DispatchEngineStrategyType::CLOSEST:
            ClosestDispatchEngineStrategy::run(
                rng,
                ambulances,
                events,
                eventIndex
            );
            break;
        default:
            RandomDispatchEngineStrategy::run(
                rng,
                ambulances,
                events,
                eventIndex
            );
            break;
    }
}

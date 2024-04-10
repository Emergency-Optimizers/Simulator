/**
 * @file DispatchEngine.cpp
 *
 * @copyright Copyright (c) 2024 Emergency-Optimizers
 */

/* internal libraries */
#include "simulator/DispatchEngine.hpp"
#include "simulator/strategies/RandomDispatchEngineStrategy.hpp"
#include "simulator/strategies/ClosestDispatchEngineStrategy.hpp"

bool DispatchEngine::dispatch(
    const DispatchEngineStrategyType strategy,
    std::mt19937& rnd,
    std::vector<Ambulance>& ambulances,
    std::vector<Event>& events,
    const int eventIndex
) {
    switch (strategy) {
        case DispatchEngineStrategyType::CLOSEST:
            return ClosestDispatchEngineStrategy::run(
                rnd,
                ambulances,
                events,
                eventIndex
            );
        default:
            return RandomDispatchEngineStrategy::run(
                rnd,
                ambulances,
                events,
                eventIndex
            );
    }
}

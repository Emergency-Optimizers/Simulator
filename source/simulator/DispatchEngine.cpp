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
    std::mt19937& rnd,
    std::vector<Ambulance>& ambulances,
    std::vector<Event>& events,
    const int eventIndex
) {
    switch (strategy) {
        case DispatchEngineStrategyType::CLOSEST:
            ClosestDispatchEngineStrategy::run(
                rnd,
                ambulances,
                events,
                eventIndex
            );
            break;
        default:
            RandomDispatchEngineStrategy::run(
                rnd,
                ambulances,
                events,
                eventIndex
            );
            break;
    }
}

/**
 * @file Simulator.cpp
 *
 * @copyright Copyright (c) 2024 Emergency-Optimizers
 */

/* external libraries */
#include <chrono>
#include <iomanip>
/* internal libraries */
#include "Utils.hpp"
#include "simulator/Simulator.hpp"
#include "simulator/DispatchEngine.hpp"

Simulator::Simulator(
    std::mt19937& rng,
    AmbulanceAllocator& ambulanceAllocator,
    DispatchEngineStrategyType dispatchStrategy,
    std::vector<Event> events
) : rng(rng), ambulanceAllocator(ambulanceAllocator), dispatchStrategy(dispatchStrategy) {
    eventHandler.populate(events);

    eventHandler.sortEvents();
}

std::vector<Event> Simulator::run(bool saveMetricsToFile) {
    // std::cout << "\nSimulator started. Total events to simulate: " << eventHandler.events.size() << std::endl;
    auto start = std::chrono::steady_clock::now();

    int eventIndex = eventHandler.getNextEventIndex();

    while (eventIndex != -1) {
        DispatchEngine::dispatch(
            dispatchStrategy,
            rng,
            ambulanceAllocator.ambulances,
            eventHandler.events,
            eventIndex
        );

        eventHandler.sortEvent(eventIndex);

        eventIndex = eventHandler.getNextEventIndex();
    }

    auto stop = std::chrono::steady_clock::now();
    std::chrono::duration<double> duration = stop - start;
    // std::cout << "\nSimulator finished. Time taken by process: " << duration.count() << " seconds" << std::endl;

    if (saveMetricsToFile) {
        writeMetrics(eventHandler.events);
    }

    return eventHandler.events;
}

/**
 * @file Simulator.cpp
 * @author Sindre Eiklid
 * @version 1.0
 * @date 2024-01-06
 *
 * @copyright Copyright (c) 2023 Sindre Eiklid
 */

#include <chrono>
/* internal libraries */
#include "Simulator.hpp"
#include "DispatchEngine.hpp"

Simulator::Simulator(
    const unsigned seed,
    Incidents& incidents,
    Stations& stations,
    ODMatrix& odMatrix,
    AmbulanceAllocator& ambulanceAllocator,
    DispatchEngineStrategy dispatchStrategy,
    const std::string& start,
    const std::string& end
) : incidents(incidents), stations(stations), odMatrix(odMatrix), ambulanceAllocator(ambulanceAllocator), dispatchStrategy(dispatchStrategy) {
    std::mt19937 rng(seed);
    
    eventHandler.populate(incidents, start, end);
}

void Simulator::run() {
    std::cout << "Simulator starded. Total events to simulate: " << eventHandler.events.size() << std::endl;
    std::chrono::steady_clock::time_point start = std::chrono::high_resolution_clock::now();

    int eventIndex = eventHandler.getNextEventIndex();

    while (eventIndex != -1) {
        Event& event = eventHandler.events[eventIndex];
        DispatchEngine::dispatch(
            dispatchStrategy,
            rng,
            incidents,
            stations,
            odMatrix,
            ambulanceAllocator.ambulances,
            event
        );

        eventHandler.sort(eventIndex);

        eventIndex = eventHandler.getNextEventIndex();
    }

    std::chrono::steady_clock::time_point stop = std::chrono::high_resolution_clock::now();
    std::chrono::duration<double> duration = std::chrono::duration<double>(stop - start);
    std::cout << std::endl << "Simulator finished. Time taken by process: " << duration.count() << " seconds" << std::endl;
}

/**
 * @file Simulator.cpp
 * @author Sindre Eiklid
 * @version 1.0
 * @date 2024-01-06
 *
 * @copyright Copyright (c) 2023 Sindre Eiklid
 */

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
    int eventIndex = eventHandler.getNextEventIndex();
    std::cout << "eventIndex: " << eventIndex << std::endl;
    std::cout <<std::endl;

    Event& currentEvent = eventHandler.events[eventIndex];
    currentEvent.print();
    std::cout <<std::endl;

    DispatchEngine::dispatch(
        dispatchStrategy,
        rng,
        incidents,
        stations,
        odMatrix,
        ambulanceAllocator.ambulances,
        currentEvent
    );
    currentEvent.print();
    std::cout <<std::endl;

    DispatchEngine::dispatch(
        dispatchStrategy,
        rng,
        incidents,
        stations,
        odMatrix,
        ambulanceAllocator.ambulances,
        currentEvent
    );
    currentEvent.print();
    std::cout <<std::endl;

    DispatchEngine::dispatch(
        dispatchStrategy,
        rng,
        incidents,
        stations,
        odMatrix,
        ambulanceAllocator.ambulances,
        currentEvent
    );
    currentEvent.print();
    std::cout <<std::endl;

    DispatchEngine::dispatch(
        dispatchStrategy,
        rng,
        incidents,
        stations,
        odMatrix,
        ambulanceAllocator.ambulances,
        currentEvent
    );
    currentEvent.print();
    std::cout <<std::endl;

    DispatchEngine::dispatch(
        dispatchStrategy,
        rng,
        incidents,
        stations,
        odMatrix,
        ambulanceAllocator.ambulances,
        currentEvent
    );
    currentEvent.print();
    std::cout <<std::endl;

    DispatchEngine::dispatch(
        dispatchStrategy,
        rng,
        incidents,
        stations,
        odMatrix,
        ambulanceAllocator.ambulances,
        currentEvent
    );
    currentEvent.print();
    std::cout <<std::endl;

    currentEvent.metrics.print();
    std::cout <<std::endl;

    incidents.printRow(currentEvent.incidentIndex);
}

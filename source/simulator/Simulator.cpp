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
    std::mt19937 rnd,
    AmbulanceAllocator& ambulanceAllocator,
    DispatchEngineStrategyType dispatchStrategy,
    std::vector<Event>& events
) : rnd(rnd),
    ambulanceAllocator(ambulanceAllocator),
    dispatchStrategy(dispatchStrategy) {
    // populate and sort event handler
    eventHandler.populate(events);

    eventHandler.sortEvents();
}

std::vector<Event> Simulator::run() {
    int eventIndex = eventHandler.getNextEventIndex();

    while (eventIndex != -1) {
        DispatchEngine::dispatch(
            dispatchStrategy,
            rnd,
            ambulanceAllocator.ambulances,
            eventHandler.events,
            eventIndex
        );

        eventHandler.sortEvent(eventIndex);

        eventIndex = eventHandler.getNextEventIndex();
    }

    return eventHandler.events;
}

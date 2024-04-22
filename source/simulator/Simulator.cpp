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
    AmbulanceAllocator& ambulanceAllocator,
    DispatchEngineStrategyType dispatchStrategy,
    std::vector<Event> events
) : ambulanceAllocator(ambulanceAllocator),
    dispatchStrategy(dispatchStrategy),
    eventHandler(events) { }

std::vector<Event> Simulator::run() {
    int eventIndex = eventHandler.getNextEventIndex();

    while (eventIndex != -1) {
        const bool sortAllEvents = DispatchEngine::dispatch(
            dispatchStrategy,
            rnd,
            ambulanceAllocator.ambulances,
            eventHandler.events,
            eventIndex
        );

        if (sortAllEvents) {
            eventHandler.sortEvents();
        } else {
            eventHandler.sortEvent(eventIndex);
        }

        eventIndex = eventHandler.getNextEventIndex();
    }

    return eventHandler.events;
}

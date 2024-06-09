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
    // get first event to process
    int eventIndex = eventHandler.getNextEventIndex();

    // continue until all events are processed
    while (eventIndex != -1) {
        // process events
        const bool sortAllEvents = DispatchEngine::dispatch(
            dispatchStrategy,
            rnd,
            ambulanceAllocator.ambulances,
            eventHandler.events,
            eventIndex
        );

        // sort events
        if (sortAllEvents) {
            eventHandler.sortEvents();
        } else {
            eventHandler.sortEvent(eventIndex);
        }

        // get next scheduled event
        eventIndex = eventHandler.getNextEventIndex();
    }

    // return processed events for evaluation of simulation
    return eventHandler.events;
}

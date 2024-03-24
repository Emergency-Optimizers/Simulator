/**
 * @file Simulator.cpp
 *
 * @copyright Copyright (c) 2024 Emergency-Optimizers
 */

/* external libraries */
#include <chrono>
#include <iomanip>
/* internal libraries */
#include "simulator/Simulator.hpp"
#include "simulator/DispatchEngine.hpp"

Simulator::Simulator(
    std::mt19937& rng,
    AmbulanceAllocator& ambulanceAllocator,
    DispatchEngineStrategyType dispatchStrategy,
    std::vector<Event> events
) : rng(rng), ambulanceAllocator(ambulanceAllocator), dispatchStrategy(dispatchStrategy) {
    eventHandler.populate(events);
}

void Simulator::run(bool saveMetricsToFile) {
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
        Utils::saveMetricsToFile(eventHandler.events);
    }
}

double Simulator::averageResponseTime(const std::string& triageImpression, bool urban) {
    int totalEvents = 0;
    float totalResponseTime = 0;
    for (int i = 0; i < eventHandler.events.size(); i++) {
        Event event = eventHandler.events[i];

        if (event.triageImpression != triageImpression || Incidents::getInstance().gridIdUrban[event.incidentGridId] != urban) continue;

        totalResponseTime += event.getResponseTime();
        totalEvents++;
    }

    if (totalEvents == 0) return 0;

    return static_cast<double>(totalResponseTime) / static_cast<double>(totalEvents);
}

double Simulator::responseTimeViolations() {
    int totalViolations = 0;

    for (int i = 0; i < eventHandler.events.size(); i++) {
        int responseTime = eventHandler.events[i].getResponseTime();

        bool urban = Incidents::getInstance().gridIdUrban[eventHandler.events[i].incidentGridId];
        std::string triage = eventHandler.events[i].triageImpression;

        if (triage == "A") {
            if (urban && responseTime > 720) {
                totalViolations++;
            } else if (!urban && responseTime > 1500) {
                totalViolations++;
            }
        } else if (triage == "H") {
            if (urban && responseTime > 1800) {
                totalViolations++;
            } else if (!urban && responseTime > 2400) {
                totalViolations++;
            }
        }
    }

    return static_cast<double>(totalViolations);
}

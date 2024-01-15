/**
 * @file Simulator.cpp
 * @author Sindre Eiklid
 * @version 1.0
 * @date 2024-01-06
 *
 * @copyright Copyright (c) 2023 Sindre Eiklid
 */

/* external libraries */
#include <chrono>
#include <iomanip>
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
    std::cout << "\nSimulator started. Total events to simulate: " << eventHandler.events.size() << std::endl;
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
    std::cout << "\nSimulator finished. Time taken by process: " << duration.count() << " seconds" << std::endl;
}

void Simulator::printAverageEventPerformanceMetrics() {
    EventPerformanceMetrics totalMetrics;
    int totalEvents = eventHandler.events.size();

    if (totalEvents == 0) return;

    for (int i = 0; i < totalEvents; i++) {
        totalMetrics.callProcessedTime += eventHandler.events[i].metrics.callProcessedTime;
        totalMetrics.dispatchToSceneTime += eventHandler.events[i].metrics.dispatchToSceneTime;
        totalMetrics.arrivalAtSceneTime += eventHandler.events[i].metrics.arrivalAtSceneTime;
        totalMetrics.dispatchToHospitalTime += eventHandler.events[i].metrics.dispatchToHospitalTime;
        totalMetrics.arrivalAtHospitalTime += eventHandler.events[i].metrics.arrivalAtHospitalTime;
        totalMetrics.dispatchToDepotTime += eventHandler.events[i].metrics.dispatchToDepotTime;
        totalMetrics.waitingForAmbulanceTime += eventHandler.events[i].metrics.waitingForAmbulanceTime;
    }

    std::cout << std::fixed << std::setprecision(2);
    std::cout << "\nAverage time spent on processing call: " << static_cast<double>(totalMetrics.callProcessedTime) / totalEvents << " seconds\n";
    std::cout << "Average time spent on dispatching to scene: " << static_cast<double>(totalMetrics.dispatchToSceneTime) / totalEvents << " seconds\n";
    std::cout << "Average time spent on arrival at scene: " << static_cast<double>(totalMetrics.arrivalAtSceneTime) / totalEvents << " seconds\n";
    std::cout << "Average time spent on dispatching to hospital: " << static_cast<double>(totalMetrics.dispatchToHospitalTime) / totalEvents << " seconds\n";
    std::cout << "Average time spent on arrival at hospital: " << static_cast<double>(totalMetrics.arrivalAtHospitalTime) / totalEvents << " seconds\n";
    std::cout << "Average time spent on dispatching to depot: " << static_cast<double>(totalMetrics.dispatchToDepotTime) / totalEvents << " seconds\n";
    std::cout << "Average time spent on waiting for ambulance: " << static_cast<double>(totalMetrics.waitingForAmbulanceTime) / totalEvents << " seconds\n";
}

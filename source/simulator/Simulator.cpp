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
    Incidents& incidents,
    Stations& stations,
    ODMatrix& odMatrix,
    AmbulanceAllocator& ambulanceAllocator,
    DispatchEngineStrategyType dispatchStrategy,
    std::vector<Event> events
) : rng(rng), incidents(incidents), stations(stations), odMatrix(odMatrix), ambulanceAllocator(ambulanceAllocator), dispatchStrategy(dispatchStrategy) {
    eventHandler.populate(events);
}

void Simulator::run(bool saveMetricsToFile) {
    // std::cout << "\nSimulator started. Total events to simulate: " << eventHandler.events.size() << std::endl;
    auto start = std::chrono::steady_clock::now();

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
            event,
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


double Simulator::getResponseTime() {
    EventPerformanceMetrics totalMetrics;
    int totalEvents = eventHandler.events.size();

    if (totalEvents == 0) return 0;

    for (int i = 0; i < totalEvents; i++) {
        totalMetrics.callProcessedTime += eventHandler.events[i].metrics.callProcessedTime;
        totalMetrics.dispatchToSceneTime += eventHandler.events[i].metrics.dispatchToSceneTime;
        totalMetrics.arrivalAtSceneTime += eventHandler.events[i].metrics.arrivalAtSceneTime;
        totalMetrics.dispatchToHospitalTime += eventHandler.events[i].metrics.dispatchToHospitalTime;
        totalMetrics.arrivalAtHospitalTime += eventHandler.events[i].metrics.arrivalAtHospitalTime;
        totalMetrics.dispatchToDepotTime += eventHandler.events[i].metrics.dispatchToDepotTime;
        totalMetrics.waitingForAmbulanceTime += eventHandler.events[i].metrics.waitingForAmbulanceTime;
    }

    double averageResponseTime = static_cast<double>(totalMetrics.callProcessedTime + totalMetrics.dispatchToSceneTime + totalMetrics.waitingForAmbulanceTime) / totalEvents;

    return averageResponseTime;
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
    double averageTimeSpentProcessingCall = static_cast<double>(totalMetrics.callProcessedTime) / totalEvents;
    std::cout << "\nAverage time spent on processing call: "
        << averageTimeSpentProcessingCall << " seconds (" << averageTimeSpentProcessingCall / 60 << " min)\n";

    double averageTimeSpentDispatchToScene = static_cast<double>(totalMetrics.dispatchToSceneTime) / totalEvents;
    std::cout << "Average time spent on dispatching to scene: "
        << averageTimeSpentDispatchToScene << " seconds (" << averageTimeSpentDispatchToScene / 60 << " min)\n";

    double averageTimeSpentArrivalAtScene = static_cast<double>(totalMetrics.arrivalAtSceneTime) / totalEvents;
    std::cout << "Average time spent on arrival at scene: "
        << averageTimeSpentArrivalAtScene << " seconds (" << averageTimeSpentArrivalAtScene / 60 << " min)\n";

    double averageTimeSpentDispatchToHospital = static_cast<double>(totalMetrics.dispatchToHospitalTime) / totalEvents;
    std::cout << "Average time spent on dispatching to hospital: "
        << averageTimeSpentDispatchToHospital << " seconds (" << averageTimeSpentDispatchToHospital / 60 << " min)\n";

    double averageTimeSpentArrivalAtHospital = static_cast<double>(totalMetrics.arrivalAtHospitalTime) / totalEvents;
    std::cout << "Average time spent on arrival at hospital: "
        << averageTimeSpentArrivalAtHospital << " seconds (" << averageTimeSpentArrivalAtHospital / 60 << " min)\n";

    double averageTimeSpentDispatchToDepot = static_cast<double>(totalMetrics.dispatchToDepotTime) / totalEvents;
    std::cout << "Average time spent on dispatching to depot: "
        << averageTimeSpentDispatchToDepot << " seconds (" << averageTimeSpentDispatchToDepot / 60 << " min)\n";

    double averageTimeSpentWaitingForAmbulance = static_cast<double>(totalMetrics.waitingForAmbulanceTime) / totalEvents;
    std::cout << "Average time spent on waiting for ambulance: "
        << averageTimeSpentWaitingForAmbulance << " seconds (" << averageTimeSpentWaitingForAmbulance / 60 << " min)\n";

    double averageResponseTime = static_cast<double>(totalMetrics.callProcessedTime + totalMetrics.dispatchToSceneTime) / totalEvents;
    std::cout << "Average response time: "
        << averageResponseTime << " seconds (" << averageResponseTime / 60 << " min)\n";
}

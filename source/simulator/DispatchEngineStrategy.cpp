/**
 * @file DispatchEngineStrategy.cpp
 * @author Sindre Eiklid
 * @version 1.0
 * @date 2024-01-18
 *
 * @copyright Copyright (c) 2024 Sindre Eiklid
 */

/* internal libraries */
#include "simulator/DispatchEngineStrategy.hpp"

void DispatchEngineStrategy::run(
    std::mt19937& rng,
    Incidents& incidents,
    Stations& stations,
    ODMatrix& odMatrix,
    std::vector<Ambulance>& ambulances,
    Event& event
) {
    if (event.assignedAmbulanceIndex == -1) {
        if (assignAmbulance(rng, incidents, stations, odMatrix, ambulances, event) == 0) return;
    }

    switch (event.type) {
        case EventType::CALL_PROCESSED:
            callProcessed(rng, incidents, stations, odMatrix, ambulances, event);
            break;
        case EventType::DISPATCH_TO_SCENE:
            dispatchToScene(rng, incidents, stations, odMatrix, ambulances, event);
            break;
        case EventType::ARRIVED_AT_SCENE:
            arrivedAtScene(rng, incidents, stations, odMatrix, ambulances, event);
            break;
        case EventType::DISPATCH_TO_HOSPITAL:
            dispatchToHospital(rng, incidents, stations, odMatrix, ambulances, event);
            break;
        case EventType::ARRIVED_AT_HOSPITAL:
            arrivedAtHospital(rng, incidents, stations, odMatrix, ambulances, event);
            break;
        case EventType::DISPATCH_TO_DEPOT:
            dispatchToDepot(rng, incidents, stations, odMatrix, ambulances, event);
            break;
    }
}

int DispatchEngineStrategy::assignAmbulance(
    std::mt19937& rng,
    Incidents& incidents,
    Stations& stations,
    ODMatrix& odMatrix,
    std::vector<Ambulance>& ambulances,
    Event& event
) {
    /// TODO: code here
    return 0;
}

void DispatchEngineStrategy::callProcessed(
    std::mt19937& rng,
    Incidents& incidents,
    Stations& stations,
    ODMatrix& odMatrix,
    std::vector<Ambulance>& ambulances,
    Event& event
) {
    /// TODO: code here
}

void DispatchEngineStrategy::dispatchToScene(
    std::mt19937& rng,
    Incidents& incidents,
    Stations& stations,
    ODMatrix& odMatrix,
    std::vector<Ambulance>& ambulances,
    Event& event
) {
    /// TODO: code here
}

void DispatchEngineStrategy::arrivedAtScene(
    std::mt19937& rng,
    Incidents& incidents,
    Stations& stations,
    ODMatrix& odMatrix,
    std::vector<Ambulance>& ambulances,
    Event& event
) {
    /// TODO: code here
}

void DispatchEngineStrategy::dispatchToHospital(
    std::mt19937& rng,
    Incidents& incidents,
    Stations& stations,
    ODMatrix& odMatrix,
    std::vector<Ambulance>& ambulances,
    Event& event
) {
    /// TODO: code here
}

void DispatchEngineStrategy::arrivedAtHospital(
    std::mt19937& rng,
    Incidents& incidents,
    Stations& stations,
    ODMatrix& odMatrix,
    std::vector<Ambulance>& ambulances,
    Event& event
) {
    /// TODO: code here
}

void DispatchEngineStrategy::dispatchToDepot(
    std::mt19937& rng,
    Incidents& incidents,
    Stations& stations,
    ODMatrix& odMatrix,
    std::vector<Ambulance>& ambulances,
    Event& event
) {
    /// TODO: code here
}

/**
 * @file MonteCarloSimulator.cpp
 *
 * @copyright Copyright (c) 2024 Emergency-Optimizers
 */

/* internal libraries */
#include "simulator/MonteCarloSimulator.hpp"

MonteCarloSimulator::MonteCarloSimulator(
    Incidents& incidents,
    const int month,
    const int day,
    const unsigned windowSize
) : windowSize(windowSize), month(month), day(day) {
    filteredIncidents = incidents.rowsWithinTimeFrame(month, day, windowSize);

    generateIncidentProbabilityDistribution();
}

void MonteCarloSimulator::generateIncidentProbabilityDistribution() {
    std::vector<int> totalIncidentsPerHour(24, 0);
    int totalIncidents = 0;

    // get total incidents per hour for each row in the filtered dataset
    for (int i = 0; i < filteredIncidents.size(); i++) {
        std::tm timeCallReceived = filteredIncidents.get<std::optional<std::tm>>("time_call_received", i).value();
        totalIncidentsPerHour[timeCallReceived.tm_hour]++;
        totalIncidents++;
    }

    // get the probability per hour
    for (int i = 0; i < totalIncidentsPerHour.size(); i++) {
        incidentProbabilityDistribution.push_back((float) totalIncidentsPerHour[i] / (float) totalIncidents);
    }
}

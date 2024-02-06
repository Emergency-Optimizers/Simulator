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

    std::tm targetTm = {};
    targetTm.tm_year = 120;  // uear 2020, a leap year
    targetTm.tm_mon = month - 1;
    targetTm.tm_mday = day;
    targetTm.tm_hour = 0;
    targetTm.tm_min = 0;
    targetTm.tm_sec = 0;
    targetTm.tm_isdst = -1;  // prevent DST affecting the calculation
    mktime(&targetTm);

    dayOfYear = targetTm.tm_yday;

    generateIncidentProbabilityDistribution();
}

void MonteCarloSimulator::generateIncidentProbabilityDistribution() {
    std::vector<float> totalIncidentsPerHour(24, 0);
    int totalIncidents = 0;

    std::vector<double> weights = generateWeights(windowSize);

    // get total incidents per hour for each row in the filtered dataset
    for (int i = 0; i < filteredIncidents.size(); i++) {
        std::tm timeCallReceived = filteredIncidents.get<std::optional<std::tm>>("time_call_received", i).value();

        timeCallReceived.tm_year = 120;  // normalize year for comparison
        timeCallReceived.tm_isdst = -1;  // prevent DST affecting the calculation
        mktime(&timeCallReceived);

        // calculate weight based on how far away the incident is from the target
        int incidentDayOfYear = timeCallReceived.tm_yday;
        int dayDiff = abs(incidentDayOfYear - dayOfYear);
        double weight = weights[dayDiff];

        totalIncidentsPerHour[timeCallReceived.tm_hour] += weight;
        totalIncidents += weight;
    }

    // get the probability per hour
    for (int i = 0; i < totalIncidentsPerHour.size(); i++) {
        incidentProbabilityDistribution.push_back(totalIncidentsPerHour[i] / totalIncidents);
    }
}

std::vector<double> MonteCarloSimulator::generateWeights(int windowSize, double sigma) {
    std::vector<double> weights;
    double centralWeight = std::exp(0);

    for (int i = 0; i <= windowSize; ++i) {
        double weight = std::exp(-i * i / (2 * sigma * sigma)) / centralWeight;
        weights.push_back(weight);
    }

    // normalize weights so the central day has a weight of 1
    double normalizationFactor = weights[0];
    for (double& weight : weights) {
        weight /= normalizationFactor;
    }

    return weights;
}

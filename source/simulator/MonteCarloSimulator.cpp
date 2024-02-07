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
) : incidents(incidents), windowSize(windowSize), month(month), day(day) {
    filteredIncidents = incidents.rowsWithinTimeFrame(month, day, windowSize);

    weights = generateWeights(windowSize);

    generateHourlyIncidentProbabilityDistribution();
    generateMinuteIncidentProbabilityDistribution();
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

void MonteCarloSimulator::generateHourlyIncidentProbabilityDistribution() {
    std::vector<float> newHourlyIncidentProbabilityDistribution(24, 0);

    std::vector<float> totalIncidentsPerHour(24, 0);
    int totalIncidents = 0;

    // get total incidents per hour for each row in the filtered dataset
    for (int i = 0; i < filteredIncidents.size(); i++) {
        std::tm timeCallReceived = filteredIncidents.get<std::optional<std::tm>>("time_call_received", i).value();

        // calculate weight based on how far away the incident is from the target
        int dayDiff = Utils::calculateDayDifference(timeCallReceived, month, day);
        double weight = weights[dayDiff];

        totalIncidentsPerHour[timeCallReceived.tm_hour] += weight;
        totalIncidents += weight;
    }

    // get the probability per hour
    for (int i = 0; i < totalIncidentsPerHour.size(); i++) {
        newHourlyIncidentProbabilityDistribution[i] = totalIncidentsPerHour[i] / totalIncidents;
    }

    hourlyIncidentProbabilityDistribution = newHourlyIncidentProbabilityDistribution;
}

void MonteCarloSimulator::generateMinuteIncidentProbabilityDistribution() {
    std::vector<std::vector<float>> newMinuteIncidentProbabilityDistribution(24, std::vector<float>(60, 0.0));

    std::vector<std::vector<float>> totalIncidentsPerMinute(24, std::vector<float>(60, 0.0));
    std::vector<float> totalIncidents(24, 0.0);

    for (int i = 0; i < filteredIncidents.size(); i++) {
        std::tm timeCallReceived = filteredIncidents.get<std::optional<std::tm>>("time_call_received", i).value();

        int dayDiff = Utils::calculateDayDifference(timeCallReceived, month, day);
        double weight = weights[dayDiff];

        totalIncidentsPerMinute[timeCallReceived.tm_hour][timeCallReceived.tm_min] += weight;
        totalIncidents[timeCallReceived.tm_hour] += weight;
    }

    for (int indexHour = 0; indexHour < 24; indexHour++) {
        for (int indexMinute = 0; indexMinute < 60; indexMinute++) {
            float minuteIncidentProbability = totalIncidentsPerMinute[indexHour][indexMinute] / totalIncidents[indexHour];
            newMinuteIncidentProbabilityDistribution[indexHour][indexMinute] = minuteIncidentProbability;

            if (indexHour == 11) std::cout << minuteIncidentProbability << ", ";
        }
    }

    minuteIncidentProbabilityDistribution = newMinuteIncidentProbabilityDistribution;
}

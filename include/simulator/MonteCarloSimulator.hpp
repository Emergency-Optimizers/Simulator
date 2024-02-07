/**
 * @file MonteCarloSimulator.hpp
 *
 * @copyright Copyright (c) 2024 Emergency-Optimizers
 */

#pragma once

/* external libraries */
#include <random>
#include <vector>
/* internal libraries */
#include "simulator/Incidents.hpp"

class MonteCarloSimulator {
 private:
    Incidents& incidents;
    Incidents filteredIncidents;
    const int windowSize;
    const int month;
    const int day;
    std::vector<double> weights;
    void generateHourlyIncidentProbabilityDistribution();
    void generateMinuteIncidentProbabilityDistribution();
 public:
    std::vector<float> hourlyIncidentProbabilityDistribution;
    std::vector<std::vector<float>> minuteIncidentProbabilityDistribution;
    MonteCarloSimulator(Incidents& incidents, const int month, const int day, const unsigned windowSize);
    std::vector<double> generateWeights(int windowSize, double sigma = 1.0);
};

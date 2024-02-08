/**
 * @file MonteCarloSimulator.hpp
 *
 * @copyright Copyright (c) 2024 Emergency-Optimizers
 */

#pragma once

/* external libraries */
#include <random>
#include <vector>
#include <map>
#include <utility>
/* internal libraries */
#include "simulator/Incidents.hpp"

class MonteCarloSimulator {
 private:
    std::mt19937& rnd;
    Incidents& incidents;
    Incidents filteredIncidents;
    const int windowSize;
    const int month;
    const int day;
    std::vector<double> weights;
    void generateHourlyIncidentProbabilityDistribution();
    void generateMinuteIncidentProbabilityDistribution();
    std::map<std::pair<float, float>, float> createHistogram(const std::vector<float>& data, int numBins);
    float generateRandomFromHistogram(const std::map<std::pair<float, float>, float>& histogram);
 public:
    std::vector<float> hourlyIncidentProbabilityDistribution;
    std::vector<std::vector<float>> minuteIncidentProbabilityDistribution;
    MonteCarloSimulator(std::mt19937& rnd, Incidents& incidents, const int month, const int day, const unsigned windowSize);
    std::vector<double> generateWeights(int windowSize, double sigma = 1.0);
};

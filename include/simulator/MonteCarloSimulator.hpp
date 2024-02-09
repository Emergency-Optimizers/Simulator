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
#include <string>
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
    void generateWaitTimeHistograms();
    void generateWaitTimeHistogram(
        const std::string fromEventColumn,
        const std::string toEventColumn,
        const int binSize
    );
    std::map<std::pair<float, float>, float> createHistogram(const std::vector<float>& data, int numBins);
    float generateRandomWaitTimeFromHistogram(const std::map<std::pair<float, float>, float>& histogram);

 public:
    std::vector<float> hourlyIncidentProbabilityDistribution;
    std::vector<std::vector<float>> minuteIncidentProbabilityDistribution;
    std::map<std::pair<std::string, std::string>, std::map<std::string, std::map<std::pair<float, float>, float>>> waitTimesHistograms;
    MonteCarloSimulator(std::mt19937& rnd, Incidents& incidents, const int month, const int day, const unsigned windowSize);
    std::vector<double> generateWeights(int windowSize, double sigma = 1.0);
};

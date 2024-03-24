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
#include "simulator/Event.hpp"

class MonteCarloSimulator {
 private:
    std::mt19937& rnd;
    std::vector<int> filteredIncidents;
    const int windowSize;
    const int year;
    const int month;
    const int day;
    const bool dayShift;
    std::vector<double> weights;

    void generateHourlyIncidentProbabilityDistribution();
    void generateMinuteIncidentProbabilityDistribution();
    void generateTriageProbabilityDistribution();
    void generateCanceledProbabilityDistribution();
    void generateLocationProbabilityDistribution();
    void generateWaitTimeHistograms();
    void generateWaitTimeHistogram(
        const std::string fromEventColumn,
        const std::string toEventColumn,
        const int binSize
    );
    std::map<std::pair<float, float>, double> createHistogram(const std::vector<float>& data, int numBins);
    int getTotalIncidentsToGenerate();
    float generateRandomWaitTimeFromHistogram(const std::map<std::pair<float, float>, double>& histogram);

 public:
    std::vector<double> hourlyIncidentProbabilityDistribution;
    std::vector<std::vector<double>> minuteIncidentProbabilityDistribution;
    std::map<std::pair<std::string, std::string>, std::map<std::string, std::map<std::pair<float, float>, double>>> waitTimesHistograms;
    std::vector<std::vector<double>> triageProbabilityDistribution;
    std::vector<std::vector<double>> canceledProbability;
    std::map<int, int64_t> indexToGridIdMapping;
    std::map<int64_t, int> gridIdToIndexMapping;
    std::vector<std::vector<std::vector<double>>> locationProbabilityDistribution;

    MonteCarloSimulator(
        std::mt19937& rnd,
        const int year,
        const int month,
        const int day,
        const bool dayShift,
        const unsigned windowSize
    );
    std::vector<double> generateWeights(int windowSize, double sigma = 1.0);
    std::vector<Event> generateEvents();
};

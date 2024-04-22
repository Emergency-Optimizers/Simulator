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
#include "file-reader/Settings.hpp"
#include "simulator/Event.hpp"
#include "simulator/KDEData.hpp"

class MonteCarloSimulator {
 private:
    std::mt19937 rnd = std::mt19937(Settings::get<int>("SEED"));
    std::vector<int> filteredIncidents;
    const int windowSize = Settings::get<int>("SIMULATION_GENERATION_WINDOW_SIZE");
    const int year = Settings::get<int>("SIMULATE_YEAR");
    const int month = Settings::get<int>("SIMULATE_MONTH");
    const int day = Settings::get<int>("SIMULATE_DAY");
    const bool dayShift = Settings::get<bool>("SIMULATE_DAY_SHIFT");
    std::vector<double> weights;

    void generateHourlyIncidentProbabilityDistribution();
    void generateTriageProbabilityDistribution();
    void generateCanceledProbabilityDistribution();
    void generateLocationProbabilityDistribution();
    int getTotalIncidentsToGenerate();
    void generateDurationsData(
        const std::string& fromEventColumn,
        const std::string& toEventColumn,
        const bool filterToCancelledEvents = false
    );
    void precomputeKDE(KDEData& kdeData);
    double sampleFromData(const KDEData& kdeData);

 public:
    std::vector<double> hourlyIncidentProbabilityDistribution;
    std::map<std::pair<std::string, std::string>, std::vector<std::vector<KDEData>>> preProcessedKDEData;
    std::vector<std::vector<double>> triageProbabilityDistribution;
    std::vector<std::vector<double>> canceledProbability;
    std::map<int, int64_t> indexToGridIdMapping;
    std::map<int64_t, int> gridIdToIndexMapping;
    std::vector<std::vector<std::vector<double>>> locationProbabilityDistribution;

    MonteCarloSimulator();
    std::vector<double> generateWeights(int weigthSize, double sigma = 1.0);
    std::vector<Event> generateEvents();
};

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
    Incidents filteredIncidents;
    const int windowSize;
    const int month;
    const int day;
    void generateIncidentProbabilityDistribution();
 public:
    std::vector<float> incidentProbabilityDistribution;
    MonteCarloSimulator(Incidents& incidents, const int month, const int day, const unsigned windowSize);
};

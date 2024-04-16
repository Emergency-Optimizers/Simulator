/**
 * @file PopulationMA.hpp
 *
 * @copyright Copyright (c) 2024 Emergency-Optimizers
 */

#pragma once

/* external libraries */
#include <vector>
#include <string>
#include <map>
/* internal libraries */
#include "heuristics/PopulationGA.hpp"

class PopulationMA : public PopulationGA {
 private:
    void localSearch(Individual& individual);

 protected:
    const std::string heuristicName = "MA";

    std::vector<Individual> createOffspring() override;
    const std::string getHeuristicName() const override;

 public:
    PopulationMA(
        std::mt19937& rnd,
        const std::vector<Event>& events,
        const bool dayShift,
        const DispatchEngineStrategyType dispatchStrategy,
        const int numAmbulancesDuringDay,
        const int numAmbulancesDuringNight,
        const int populationSize,
        const double mutationProbability,
        const double crossoverProbability,
        const int numTimeSegments
    );
};

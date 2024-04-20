/**
 * @file PopulationMemeticNSGA2.hpp
 *
 * @copyright Copyright (c) 2024 Emergency-Optimizers
 */

#pragma once

/* external libraries */
#include <vector>
#include <string>
#include <map>
/* internal libraries */
#include "heuristics/PopulationNSGA2.hpp"
#include "heuristics/PopulationMA.hpp"

class PopulationMemeticNSGA2 : public PopulationNSGA2, public PopulationMA {
 protected:
    const std::string heuristicName = "MemeticNSGA2";

    using PopulationMA::createOffspring;
    using PopulationNSGA2::parentSelection;
    using PopulationNSGA2::survivorSelection;
    using PopulationNSGA2::sortIndividuals;
    using PopulationNSGA2::getProgressBarPostfix;
    const std::string getHeuristicName() const override;
    using PopulationNSGA2::storeGenerationMetrics;

 public:
    PopulationMemeticNSGA2(
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
    using PopulationNSGA2::evolve;
};

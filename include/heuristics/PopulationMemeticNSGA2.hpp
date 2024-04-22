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
    explicit PopulationMemeticNSGA2(const std::vector<Event>& events);
    using PopulationNSGA2::evolve;
};

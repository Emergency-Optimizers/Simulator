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

class PopulationMA : virtual public PopulationGA {
 private:
    void localSearch(Individual& individual);

 protected:
    const std::string heuristicName = "MA";
    const double localSearchProbability = Settings::get<float>("LOCAL_SEARCH_PROBABILITY");

    std::vector<Individual> createOffspring() override;
    const std::string getHeuristicName() const override;

 public:
    explicit PopulationMA(const std::vector<Event>& events);
};

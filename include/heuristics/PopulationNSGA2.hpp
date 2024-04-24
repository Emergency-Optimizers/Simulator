/**
 * @file PopulationNSGA2.hpp
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

class PopulationNSGA2 : virtual public PopulationGA {
 private:
    std::vector<std::vector<Individual*>> fronts;

    void nonDominatedSort();
    void calculateCrowdingDistance(std::vector<Individual*>& front);
    std::vector<Individual> tournamentSelection(const int k, const int tournamentSize);
    Individual tournamentWinner(Individual& individual1, Individual& individual2);

 protected:
    const std::string heuristicName = "NSGA2";
    std::map<std::string, std::vector<std::vector<double>>> metrics = {
        {"diversity", {}},
        {"avg_response_time_urban_a", {}},
        {"avg_response_time_urban_h", {}},
        {"avg_response_time_urban_v1", {}},
        {"avg_response_time_rural_a", {}},
        {"avg_response_time_rural_h", {}},
        {"avg_response_time_rural_v1", {}},
        {"percentage_violations", {}},
        {"percentage_violations_urban", {}},
        {"percentage_violations_rural", {}},
        {"front_number", {}},
        {"crowding_distance", {}},
    };

    std::vector<Individual> parentSelection() override;
    std::vector<Individual> survivorSelection() override;
    void sortIndividuals() override;
    const std::string getProgressBarPostfix() const override;
    void storeGenerationMetrics() override;
    const std::string getHeuristicName() const override;

 public:
    explicit PopulationNSGA2(const std::vector<Event>& events);
    void evolve(const bool verbose = true) override;
};

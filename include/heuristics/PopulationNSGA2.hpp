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

class PopulationNSGA2 : public PopulationGA {
 private:
    std::vector<std::vector<Individual*>> fronts;

    void nonDominatedSort();
    void calculateCrowdingDistance(std::vector<Individual*>& front);
    std::vector<Individual> selectNextGeneration(const int selectionSize);
    std::vector<Individual> tournamentSelection(const int k, const int tournamentSize);
    Individual tournamentWinner(Individual& individual1, Individual& individual2);

 protected:
    const std::string heuristicName = "NSGA-II";
    const std::string progressBarPrefix = "Running NSGA-II";
    std::map<std::string, std::vector<std::vector<double>>> metrics = {
        {"diversity", {}},
        {"avg_response_time_urban_a", {}},
        {"avg_response_time_urban_h", {}},
        {"avg_response_time_urban_v1", {}},
        {"avg_response_time_rural_a", {}},
        {"avg_response_time_rural_h", {}},
        {"avg_response_time_rural_v1", {}},
        {"percentage_violations", {}},
        {"front_number", {}},
        {"crowding_distance", {}},
    };

    void sortIndividuals() override;
    const std::string getProgressBarPostfix() const override;
    void storeGenerationMetrics() override;

 public:
    PopulationNSGA2::PopulationNSGA2(
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
    void evolve(int generations) override;
};

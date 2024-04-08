/**
 * @file PopulationGA.hpp
 *
 * @copyright Copyright (c) 2024 Emergency-Optimizers
 */

#pragma once

/* external libraries */
#include <vector>
#include <numeric>
#include <random>
#include <string>
/* internal libraries */
#include "heuristics/IndividualGA.hpp"
#include "file-reader/Incidents.hpp"
#include "file-reader/Stations.hpp"
#include "file-reader/ODMatrix.hpp"
#include "simulator/Event.hpp"
#include "heuristics/GenotypeInitType.hpp"
#include "heuristics/MutationType.hpp"

class PopulationGA {
 private:
    std::mt19937& rnd;
    const std::vector<Event>& events;
    const bool dayShift;
    const DispatchEngineStrategyType dispatchStrategy;
    const int populationSize;
    const int numDepots;
    const int numAmbulances;
    const double mutationProbability;
    const double crossoverProbability;
    const int numTimeSegments;
    std::vector<IndividualGA> individuals;
    std::vector<GenotypeInitType> genotypeInits;
    std::vector<double> genotypeInitsTickets;
    std::vector<MutationType> mutations;
    std::vector<double> mutationsTickets;

    void getPossibleGenotypeInits();
    void getPossibleMutations();
    std::vector<IndividualGA> parentSelection(int tournamentSize);
    std::vector<IndividualGA> survivorSelection(int numSurvivors);
    std::vector<IndividualGA> crossover(const IndividualGA& parent1, const IndividualGA& parent2);
    std::vector<std::vector<std::vector<int>>> singlePointCrossover(
        const std::vector<std::vector<int>>& parent1Genotype,
        const std::vector<std::vector<int>>& parent2Genotype
    );
    IndividualGA createIndividual(const bool child);
    void evaluateFitness();
    void sortIndividuals();
    const std::string getProgressBarPostfix() const;
    const IndividualGA getFittest() const;
    const int countUnique() const;

 protected:
    const std::string progressBarPrefix = "Running GA";

 public:
    PopulationGA::PopulationGA(
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
    void evolve(int generations);
};

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
    std::vector<GenotypeInitType> genotypeInitTypes;
    std::vector<double> genotypeInitTypeWeights;
    std::vector<MutationType> mutationTypes;
    std::vector<double> mutationTypeWeights;
    const std::string progressBarPrefix = "Running GA";

    void getPossibleGenotypeInits();
    void getPossibleMutations();
    std::vector<IndividualGA> parentSelection(int tournamentSize);
    std::vector<IndividualGA> survivorSelection(int numSurvivors);
    std::vector<IndividualGA> crossover(const IndividualGA& parent1, const IndividualGA& parent2);
    std::vector<IndividualGA> singlePointCrossover(const IndividualGA& parent1, const IndividualGA& parent2);
    const int countUnique() const;
    const IndividualGA getFittest() const;
    IndividualGA createIndividual(const bool child);

 protected:
    virtual void evaluateFitness();
    virtual const std::string getProgressBarPostfix() const;

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

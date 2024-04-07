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

    void getPossibleGenotypeInits();
    void getPossibleMutations();

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
    void evaluateFitness();
    std::vector<IndividualGA> parentSelection(int tournamentSize);
    std::vector<IndividualGA> survivorSelection(int numSurvivors);
    void addChildren(const std::vector<IndividualGA>& children);
    std::vector<IndividualGA> PopulationGA::crossover(const IndividualGA& parent1, const IndividualGA& parent2);
    std::vector<IndividualGA> PopulationGA::singlePointCrossover(const IndividualGA& parent1, const IndividualGA& parent2);
    void evolve(int generations);
    int PopulationGA::countUnique();
    const IndividualGA findFittest();
};

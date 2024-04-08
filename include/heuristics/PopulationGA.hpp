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
#include <utility>
/* internal libraries */
#include "heuristics/IndividualGA.hpp"
#include "file-reader/Incidents.hpp"
#include "file-reader/Stations.hpp"
#include "file-reader/ODMatrix.hpp"
#include "simulator/Event.hpp"
#include "heuristics/GenotypeInitType.hpp"
#include "heuristics/MutationType.hpp"
#include "heuristics/CrossoverType.hpp"
#include "heuristics/SelectionType.hpp"

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
    std::vector<CrossoverType> crossovers;
    std::vector<double> crossoversTickets;
    std::vector<SelectionType> parentSelections;
    std::vector<double> parentSelectionsTickets;

    void getPossibleGenotypeInits();
    void getPossibleMutations();
    void getPossibleCrossovers();
    void getPossibleParentSelections();
    std::vector<IndividualGA> parentSelection();
    std::vector<IndividualGA> survivorSelection(int numSurvivors);
    std::vector<int> tournamentSelection(
        const std::vector<std::pair<int, double>>& population,
        const int k,
        const int tournamentSize
    );
    std::vector<int> rouletteWheelSelection(
        const std::vector<std::pair<int, double>>& population,
        const int k
    );
    std::vector<int> elitismSelection(
        const std::vector<std::pair<int, double>>& population,
        const int k
    );
    std::vector<IndividualGA> crossover(const IndividualGA& parent1, const IndividualGA& parent2);
    std::vector<std::vector<std::vector<int>>> singlePointCrossover(
        const std::vector<std::vector<int>>& parent1Genotype,
        const std::vector<std::vector<int>>& parent2Genotype
    );
    IndividualGA createIndividual(const bool child);
    const IndividualGA getFittest() const;
    const int countUnique() const;

 protected:
    const std::string progressBarPrefix = "Running GA";

    virtual void generatePopulation();
    std::vector<std::pair<int, double>> generateIndexFitnessPair();
    virtual void evaluateFitness();
    virtual void sortIndividuals();
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
    virtual void evolve(int generations);
};

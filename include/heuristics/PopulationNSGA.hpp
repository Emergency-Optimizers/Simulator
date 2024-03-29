/**
 * @file PopulationNSGA.hpp
 *
 * @copyright Copyright (c) 2024 Emergency-Optimizers
 */

#pragma once

/* external libraries */
#include <vector>
#include <iostream>
#include <numeric>
#include <random>
/* internal libraries */
#include "heuristics/IndividualNSGA.hpp"
#include "simulator/Event.hpp"

class PopulationNSGA {
 private:
    std::mt19937& rnd;
    std::vector<Event> events = { };
    bool useFronts;
    std::vector<float> objectiveWeights;
    std::vector<IndividualNSGA> individuals;
    int populationSize;
    int numDepots;
    int numAmbulances;
    int numObjectives;
    std::vector<std::vector<IndividualNSGA*>> fronts;
    double mutationProbability;
    double crossoverProbability;
    const bool dayShift;

 public:
    PopulationNSGA::PopulationNSGA(
        std::mt19937& rnd,
        bool useFronts,
        std::vector<float> objectiveWeights,
        int populationSize,
        double mutationProbability,
        double crossoverProbability,
        const bool dayShift
    );
    void evaluateObjectives();
    std::vector<IndividualNSGA> parentSelection(int tournamentSize);
    std::vector<IndividualNSGA> survivorSelection(int numSurvivors);
    void addChildren(const std::vector<IndividualNSGA>& children);
    IndividualNSGA crossover(const IndividualNSGA& parent1, const IndividualNSGA& parent2);
    void calculateCrowdingDistance(std::vector<IndividualNSGA*>& front);
    void fastNonDominatedSort();
    void evolve(int generations);
    int countUnique(const std::vector<IndividualNSGA>& population);
    const IndividualNSGA& findFittest() const;
    const IndividualNSGA findLeastFit();
    void checkEmptyGenotypes();
    void printPopulationInfo();
    void printBestScoresForEachObjective() const;
};

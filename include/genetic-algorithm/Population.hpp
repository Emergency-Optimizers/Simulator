/**
 * @file Population.hpp
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
#include "genetic-algorithm/Individual.hpp"
#include "file-reader/Incidents.hpp"
#include "file-reader/Stations.hpp"
#include "file-reader/ODMatrix.hpp"
#include "simulator/Event.hpp"

class Population {
 private:
    std::mt19937& rnd;
    std::vector<Event> events = { };
    std::vector<Individual> individuals;
    int populationSize;
    int numDepots;
    int numAmbulances;
    double mutationProbability;
    const bool dayShift;

 public:
    Population::Population(
        std::mt19937& rnd,
        int populationSize,
        double mutationProbability,
        const bool dayShift
    );
    void evaluateFitness();
    std::vector<Individual> parentSelection(int numParents, int tournamentSize);
    std::vector<Individual> survivorSelection(int numSurvivors);
    void addChildren(const std::vector<Individual>& children);
    Individual crossover(const Individual& parent1, const Individual& parent2);
    void evolve(int generations);
    int Population::countUnique(const std::vector<Individual>& population);
    const Individual findFittest();
    const Individual findLeastFit();
    const double averageFitness();
};

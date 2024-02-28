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
#include "simulator/Incidents.hpp"
#include "simulator/Stations.hpp"
#include "simulator/ODMatrix.hpp"
#include "simulator/Event.hpp"

class Population {
 private:
    std::mt19937& rnd;
    Incidents& incidents;
    Stations& stations;
    ODMatrix& odMatrix;
    std::vector<Event> events = { };
    std::vector<Individual> individuals;
    int populationSize;
    int numDepots;
    int numAmbulances;
    double mutationProbability;

 public:
    Population::Population(
        std::mt19937& rnd,
        Incidents& incidents,
        Stations& stations,
        ODMatrix& odMatrix,
        int populationSize,
        int numDepots,
        int numAmbulances,
        double mutationProbability,
        bool saveEventsToCSV
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

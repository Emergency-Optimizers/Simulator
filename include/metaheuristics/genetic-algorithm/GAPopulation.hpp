/**
 * @file GAPopulation.hpp
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
#include "metaheuristics/genetic-algorithm/GAIndividual.hpp"
#include "simulator/Incidents.hpp"
#include "simulator/Stations.hpp"
#include "simulator/ODMatrix.hpp"
#include "simulator/Event.hpp"

class GAPopulation {
 private:
    std::mt19937& rnd;
    Incidents& incidents;
    Stations& stations;
    ODMatrix& odMatrix;
    std::vector<Event> events = { };
    std::vector<GAIndividual> individuals;
    int populationSize;
    int numDepots;
    int numAmbulances;
    double mutationProbability;

 public:
    GAPopulation::GAPopulation(
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
    std::vector<GAIndividual> parentSelection(int numParents, int tournamentSize);
    std::vector<GAIndividual> survivorSelection(int numSurvivors);
    void addChildren(const std::vector<GAIndividual>& children);
    GAIndividual crossover(const GAIndividual& parent1, const GAIndividual& parent2);
    void evolve(int generations);
    int countUnique(const std::vector<GAIndividual>& population);
    const GAIndividual findFittest();
    const GAIndividual findLeastFit();
    const double averageFitness();
};

/**
 * @file PopulationGA.hpp
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
#include "metaheuristics/genetic-algorithm/IndividualGA.hpp"
#include "simulator/Incidents.hpp"
#include "simulator/Stations.hpp"
#include "simulator/ODMatrix.hpp"
#include "simulator/Event.hpp"

class PopulationGA {
 private:
    std::mt19937& rnd;
    Incidents& incidents;
    Stations& stations;
    ODMatrix& odMatrix;
    std::vector<Event> events = { };
    std::vector<IndividualGA> individuals;
    int populationSize;
    int numDepots;
    int numAmbulances;
    double mutationProbability;

 public:
    PopulationGA::PopulationGA(
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
    std::vector<IndividualGA> parentSelection(int numParents, int tournamentSize);
    std::vector<IndividualGA> survivorSelection(int numSurvivors);
    void addChildren(const std::vector<IndividualGA>& children);
    IndividualGA crossover(const IndividualGA& parent1, const IndividualGA& parent2);
    void evolve(int generations);
    int countUnique(const std::vector<IndividualGA>& population);
    const IndividualGA findFittest();
    const IndividualGA findLeastFit();
    const double averageFitness();
};

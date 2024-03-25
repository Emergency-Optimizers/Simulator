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
#include "metaheuristics/nsga2/IndividualNSGA.hpp"
#include "simulator/Incidents.hpp"
#include "simulator/Stations.hpp"
#include "simulator/ODMatrix.hpp"
#include "simulator/Event.hpp"

class PopulationNSGA {
 private:
    std::mt19937& rnd;
    Incidents& incidents;
    Stations& stations;
    ODMatrix& odMatrix;
    std::vector<Event> events = { };
    std::vector<IndividualNSGA> individuals;
    int populationSize;
    int numDepots;
    int numAmbulances;
    int numObjectives;
    std::vector<std::vector<IndividualNSGA*>> fronts;
    double mutationProbability;

 public:
    PopulationNSGA::PopulationNSGA(
        std::mt19937& rnd,
        Incidents& incidents,
        Stations& stations,
        ODMatrix& odMatrix,
        int populationSize,
        int numDepots,
        int numAmbulances,
        int numObjectives,
        double mutationProbability,
        bool saveEventsToCSV
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

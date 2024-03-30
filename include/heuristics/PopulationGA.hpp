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
#include "heuristics/IndividualGA.hpp"
#include "file-reader/Incidents.hpp"
#include "file-reader/Stations.hpp"
#include "file-reader/ODMatrix.hpp"
#include "simulator/Event.hpp"

class PopulationGA {
 private:
    std::mt19937& rnd;
    std::vector<Event> events = { };
    std::vector<IndividualGA> individuals;
    int populationSize;
    int numDepots;
    int numAmbulances;
    double mutationProbability;
    double crossoverProbability;
    const bool dayShift;
    int numTimeSegments;

 public:
    PopulationGA::PopulationGA(
        std::mt19937& rnd,
        int populationSize,
        double mutationProbability,
        double crossoverProbability,
        const bool dayShift,
        int numTimeSegments
    );
    void evaluateFitness();
    std::vector<IndividualGA> parentSelection(int tournamentSize);
    std::vector<IndividualGA> survivorSelection(int numSurvivors);
    void addChildren(const std::vector<IndividualGA>& children);
    std::vector<IndividualGA> crossover(const IndividualGA& parent1, const IndividualGA& parent2);
    void evolve(int generations);
    int PopulationGA::countUnique();
    const IndividualGA findFittest();
};

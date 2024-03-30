/**
 * @file PopulationTSGA.hpp
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
#include "heuristics/IndividualTSGA.hpp"
#include "file-reader/Incidents.hpp"
#include "file-reader/Stations.hpp"
#include "file-reader/ODMatrix.hpp"
#include "simulator/Event.hpp"

class PopulationTSGA {
 private:
    std::mt19937& rnd;
    std::vector<Event> events = { };
    std::vector<IndividualTSGA> individuals;
    int populationSize;
    int numDepots;
    int numAmbulances;
    double mutationProbability;
    double crossoverProbability;
    const bool dayShift;
    int numTimeSegments;

 public:
    PopulationTSGA::PopulationTSGA(
        std::mt19937& rnd,
        int populationSize,
        double mutationProbability,
        double crossoverProbability,
        const bool dayShift,
        int numTimeSegments
    );
    void evaluateFitness();
    std::vector<IndividualTSGA> parentSelection(int tournamentSize);
    std::vector<IndividualTSGA> survivorSelection(int numSurvivors);
    void addChildren(const std::vector<IndividualTSGA>& children);
    std::vector<IndividualTSGA> crossover(const IndividualTSGA& parent1, const IndividualTSGA& parent2);
    void evolve(int generations);
    int PopulationTSGA::countUnique();
    const IndividualTSGA findFittest();
};

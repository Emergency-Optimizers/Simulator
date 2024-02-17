/**
 * @file Main.cpp
 *
 * @copyright Copyright (c) 2024 Emergency-Optimizers
 */

#include <iostream>
/* internal libraries */
#include "simulator/Incidents.hpp"
#include "simulator/Stations.hpp"
#include "simulator/ODMatrix.hpp"
#include "simulator/AmbulanceAllocator.hpp"
#include "simulator/Simulator.hpp"
#include "simulator/MonteCarloSimulator.hpp"
#include "genetic-algorithm/Individual.hpp"
#include "genetic-algorithm/Population.hpp"

int main() {
    std::mt19937 rnd(0);

    Incidents incidents;
    incidents.loadFromFile("../../Data-Processing/data/enhanced/oslo/incidents.csv");

    Stations stations;
    stations.loadFromFile("../../Data-Processing/data/enhanced/oslo/depots.csv");

    ODMatrix odMatrix;
    odMatrix.loadFromFile("../../Data-Processing/data/oslo/od_matrix.txt");

    int populationSize = 50;
    int numDepots = 19;
    int numAmbulances = 45;
    double mutationProbability = 0.05;
    int generations = 15;

    std::cout << "Before evolve" << std::endl;
    Population population(rnd, incidents, stations, odMatrix, populationSize, numDepots, numAmbulances, mutationProbability);
    // Run the genetic algorithm for the specified number of generations
    population.evolve(generations);
    // Find and print the fittest individual after the final generation
    Individual fittest = population.findFittest();
    std::cout << "Fittest Individual After Evolution: " << std::endl;
    std::cout << "Valid: " << fittest.isValid() << std::endl;
    fittest.printChromosome();

    return 0;
}

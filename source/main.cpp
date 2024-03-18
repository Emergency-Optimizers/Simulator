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
#include "metaheuristics/nsga2/Individual.hpp"
#include "metaheuristics/nsga2/Population.hpp"

int main() {
    std::mt19937 rnd(0);
    std::cout << "Loading incidents..." << std::endl;
    Incidents incidents;
    incidents.loadFromFile("../../Data-Processing/data/enhanced/oslo/incidents.csv");
/*
    std::cout << "Loading stations..." << std::endl;
    Stations stations;
    stations.loadFromFile("../../Data-Processing/data/enhanced/oslo/depots.csv");

    ODMatrix odMatrix;
    odMatrix.loadFromFile("../../Data-Processing/data/oslo/od_matrix.txt");


 Incidents incidents("../../Data-Processing/data/enhanced/oslo/incidents.csv");
    Stations stations("../../Data-Processing/data/enhanced/oslo/depots.csv");
    ODMatrix odMatrix("../../Data-Processing/data/oslo/od_matrix.txt");
    
    int numObjectives = 3;
    int populationSize = 10;
    int numDepots = 19;
    int numAmbulances = 45;
    double mutationProbability = 0.05;
    int generations = 3;
    bool saveEventsToCSV = true;

    std::cout << "Starting GA..." << std::endl;
    Population population(rnd, incidents, stations, odMatrix, populationSize, numDepots, numAmbulances, numObjectives, mutationProbability, saveEventsToCSV);
    
    // run the genetic algorithm for the specified number of generations
    population.evolve(generations);

    std::cout << "Got out! " << std::endl;

    // find and print the fittest individual after the final generation
    Individual fittest = population.findFittest();
    std::cout << "Fittest Individual After Evolution: " << std::endl;
    std::cout << "Valid: " << fittest.isValid() << std::endl;
    fittest.printChromosome();
*/
    return 0;
}

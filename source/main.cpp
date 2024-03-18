/**
 * @file Main.cpp
 *
 * @copyright Copyright (c) 2024 Emergency-Optimizers
 */

#include <iostream>
#include <chrono>
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
    

    Incidents incidents("../../Data-Processing/data/enhanced/oslo/incidents.csv");
    Stations stations("../../Data-Processing/data/enhanced/oslo/depots.csv");
    ODMatrix odMatrix("../../Data-Processing/data/oslo/od_matrix.txt");

    int populationSize = 50;
    int numDepots = 19;
    int numAmbulances = 45;
    double mutationProbability = 0.05;
    int generations = 50;
    bool saveEventsToCSV = true;

    std::cout << "Starting GA..." << std::endl;
    Population population(rnd, incidents, stations, odMatrix, populationSize, numDepots, numAmbulances, mutationProbability, saveEventsToCSV);

    // run the genetic algorithm for the specified number of generations
    auto start = std::chrono::high_resolution_clock::now();
    population.evolve(generations);
    auto end = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count();

    std::cout << "Got out! " << std::endl;

    // find and print the fittest individual after the final generation
    std::cout << "\n[--- GA completed in " << duration / 1000 << " seconds ---]" << std::endl;
    std::cout << "Generations: " << generations << ", population size: " << populationSize << std::endl;
    Individual fittest = population.findFittest();
    std::cout << "Fittest Individual: " << fittest.getFitness();
    std::cout << (fittest.isValid() ? " [valid]\n" : " [invalid]\n")  << std::endl;

    fittest.printChromosome();
*/
    return 0;
}

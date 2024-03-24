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
#include "metaheuristics/genetic-algorithm/GAIndividual.hpp"
#include "metaheuristics/genetic-algorithm/GAPopulation.hpp"

int main() {
    std::mt19937 rnd(0);

    Incidents incidents("../../Data-Processing/data/enhanced/oslo/incidents.csv");
    std::cout << "[1/3] Incidents loaded" << std::endl;
    Stations stations("../../Data-Processing/data/enhanced/oslo/depots.csv");
    std::cout << "[2/3] Stations loaded" << std::endl;
    ODMatrix odMatrix("../../Data-Processing/data/oslo/od_matrix.txt");
    std::cout << "[3/3] OD matrix loaded" << std::endl;

    int numObjectives = 7;
    int populationSize = 30;
    int numDepots = 19;
    int numAmbulances = 45;
    double mutationProbability = 0.05;
    int generations = 30;
    bool saveEventsToCSV = true;
    std::string simulator = "GA";

    GAPopulation GApopulation(rnd, incidents, stations, odMatrix, populationSize, numDepots, numAmbulances, mutationProbability, saveEventsToCSV);
    Population NSGApopulation(rnd, incidents, stations, odMatrix, populationSize, numDepots, numAmbulances, numObjectives, mutationProbability, saveEventsToCSV);

    // run the algorithm for the specified number of generations
    auto start = std::chrono::high_resolution_clock::now();
    if (simulator == "GA") {
        std::cout << "\nStarting GA..." << std::endl;
        GApopulation.evolve(generations);
    } else if (simulator == "NSGA") {
        std::cout << "\nStarting NSGA..." << std::endl;
        NSGApopulation.evolve(generations);
    }
    auto end = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count();

    return 0;
}

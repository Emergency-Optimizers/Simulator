#include <iostream>
#include "Individual.hpp"
#include "Population.hpp"

int main() {
    // Define the parameters for the population
    int populationSize = 50;  // Total number of individuals in the population
    int numDepots = 19;        // Number of depots
    int numAmbulances = 45;    // Total number of ambulances
    double mutationProbability = 0.05;  // Probability of mutation
    int generations = 100;      // Number of generations to evolve

    // Create a population with the specified parameters
    unsigned int seed = 12345; // Example seed
    Population population(populationSize, numDepots, numAmbulances, mutationProbability);
    std::cout << "Before evolve" << std::endl;
    // Run the genetic algorithm for the specified number of generations
    population.evolve(generations);
    // Find and print the fittest individual after the final generation
    Individual fittest = population.findFittest();
    std::cout << "Fittest Individual After Evolution: " << std::endl;
    std::cout << "Valid: " << fittest.isValid() << std::endl;
    fittest.printChromosome();

    return 0;
}

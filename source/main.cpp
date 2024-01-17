#include <iostream>
#include "Individual.hpp"
#include "Population.hpp"

int main() {
    const int numDepots = 7;
    const int numAmbulances = 15;
    const double mutationProbability = 0.02; // Example mutation probability

    // Create an individual with a specific number of depots and ambulances
    Individual individual(numDepots, numAmbulances, mutationProbability);

    // Print the initial state of the individual
    std::cout << "Initial State:" << std::endl;
    individual.printChromosome();

    // Apply mutation
    individual.mutate();

    // Print the mutated state of the individual
    std::cout << "\nAfter Mutation:" << std::endl;
    individual.printChromosome();

    return 0;
}
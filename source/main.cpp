// main.cpp

#include <iostream>
#include "Individual.hpp"
#include "GAUtils.hpp"

int main() {
    // Create an Individual with a specified number of depots
    int numDepots = 10;
    Individual individual(numDepots);

    // Set a known state of ambulances for testing
    individual.setNumAmbulances(5);
    std::vector<int> testGenotype(numDepots, 1);
    individual.setGenotype(testGenotype);

    // Test the randomizeAmbulances method
    individual.randomizeAmbulances();
    std::cout << "Randomized Ambulances:" << std::endl;
    individual.printChromosome();
    std::cout << "Is valid: " << (individual.isValid() ? "Yes" : "No") << std::endl;

    // Test setting ambulances at a specific depot
    individual.setAmbulancesAtDepot(2, 3);
    std::cout << "\nSet 3 ambulances at depot 2:" << std::endl;
    individual.printChromosome();
    std::cout << "Is valid: " << (individual.isValid() ? "Yes" : "No") << std::endl;

    // Test getting the number of ambulances at a specific depot
    int ambulancesAtDepot = individual.getAmbulancesAtDepot(2);
    std::cout << "\nNumber of ambulances at depot 2: " << ambulancesAtDepot << std::endl;

    // Test getting the total number of ambulances
    int totalAmbulances = individual.getNumAmbulances();
    std::cout << "Total number of ambulances: " << totalAmbulances << std::endl;

    return 0;
}

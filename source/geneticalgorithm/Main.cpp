#include <iostream>
#include "Individual.cpp"  // Assuming Individual class is defined in Individual.cpp

int main() {
    // Create an instance of Individual with a specific number of depots
    int numDepots = 19;
    int totalAmbulances = 45;

    Individual individual(numDepots);
    individual.setNumAmbulances(totalAmbulances);

    // Randomize ambulances distribution
    individual.randomizeAmbulances();

    // Print the chromosome
    std::cout << "Initial Random Distribution:" << std::endl;
    individual.printChromosome();

    // Print the chromosome after setting ambulances at depot 2
    std::cout << "\nDistribution after setting ambulances at depot 2:" << std::endl;
    individual.printChromosome();

    std::cout << "\nValid: " << (individual.isValid() ? "true" : "false") << std::endl;

    return 0;
}
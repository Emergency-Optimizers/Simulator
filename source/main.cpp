// main.cpp

#include <iostream>
#include "Individual.hpp"

int main() {
    // Constants for the number of depots and ambulances
    const int numDepots = 10;
    const int numAmbulances = 15;

    // Create three Individual objects
    Individual individual1(numDepots, numAmbulances);
    Individual individual2(numDepots, numAmbulances);
    Individual individual3(numDepots, numAmbulances);

    // Array to easily iterate over the individuals
    Individual individuals[3] = {individual1, individual2, individual3};

    // Test validity and print chromosome for each individual
    for (int i = 0; i < 3; ++i) {
        std::cout << "Individual " << (i + 1) << " chromosome:" << std::endl;
        individuals[i].printChromosome();
        std::cout << "Is valid: " << (individuals[i].isValid() ? "Yes" : "No") << "\n" << std::endl;
    }

    return 0;
}

#include <iostream>
#include "Individual.hpp"

int main() {
    const int numDepots = 10; // Number of depots
    const int totalAmbulances = 45; // Total ambulances for medium and low fitness individuals

    // High Fitness Individual
    Individual highFitnessIndividual(numDepots, totalAmbulances);
    std::vector<int> highDistribution(numDepots, 2); // Close to but not exceeding the ideal max
    highFitnessIndividual.setGenotype(highDistribution);
    highFitnessIndividual.evaluateFitness();
    std::cout << "High Fitness Individual: " << highFitnessIndividual.getFitness() << std::endl;

    // Medium Fitness Individual
    Individual mediumFitnessIndividual(numDepots, totalAmbulances);
    std::vector<int> mediumDistribution(numDepots, 1); // Some depots underutilized
    mediumFitnessIndividual.setGenotype(mediumDistribution);
    mediumFitnessIndividual.evaluateFitness();
    std::cout << "Medium Fitness Individual: " << mediumFitnessIndividual.getFitness() << std::endl;

    // Low Fitness Individual
    Individual lowFitnessIndividual(numDepots, totalAmbulances);
    std::vector<int> lowDistribution(numDepots, 7); // Most depots exceeding the ideal max
    lowFitnessIndividual.setGenotype(lowDistribution);
    lowFitnessIndividual.evaluateFitness();
    std::cout << "Low Fitness Individual: " << lowFitnessIndividual.getFitness() << std::endl;

    return 0;
}

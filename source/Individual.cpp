// Individual.cpp

#include "Individual.hpp"
#include "GAUtils.hpp"

/**
 * @brief Construct a new Individual object with specified number of depots.
 * @param numDepots Number of depots in the genotype.
 */
Individual::Individual(int numDepots) : genotype(numDepots, 0) {}

/**
 * @brief Randomizes the number of ambulances at each depot.
 */
void Individual::randomizeAmbulances() {
    std::fill(genotype.begin(), genotype.end(), 0); // Reset all depots to 0 ambulances

    for (int i = 0; i < numAmbulances; ++i) {
        int depotIndex = RandomUtil::randomInt(0, genotype.size() - 1);
        genotype[depotIndex]++;
    }
}

/**
 * @brief Checks if the individual's genotype is valid.
 * @return true if the total number of ambulances matches numAmbulances, otherwise false.
 */
bool Individual::isValid() const {
    int totalAmbulances = std::accumulate(genotype.begin(), genotype.end(), 0);
    return totalAmbulances == numAmbulances;
    }

/**
 * @brief Prints the chromosome (genotype) of the individual.
 */
void Individual::printChromosome() const {
    for (int i = 0; i < genotype.size(); ++i) {
        std::cout << "Depot " << i << ": " << genotype[i] << " ambulances" << std::endl;
    }
}

const std::vector<int>& Individual::getGenotype() const {
    return genotype;
}

void Individual::setGenotype(const std::vector<int>& newGenotype) {
    genotype = newGenotype;
}

void Individual::setAmbulancesAtDepot(int depotIndex, int count) {
    if (depotIndex >= 0 && depotIndex < genotype.size()) {
        genotype[depotIndex] = count;
    }
}

int Individual::getAmbulancesAtDepot(int depotIndex) const {
    return (depotIndex >= 0 && depotIndex < genotype.size()) ? genotype[depotIndex] : -1;
}

int Individual::getNumAmbulances() const {
    return numAmbulances;
}

void Individual::setNumAmbulances(int newNumAmbulances) {
    numAmbulances = newNumAmbulances;
}

int Individual::getNumDepots() const {
    return numDepots;
}

void Individual::setNumDepots(int newNumDepots) {
    numDepots = newNumDepots;
};
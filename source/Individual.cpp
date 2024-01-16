// Individual.cpp

#include "Individual.hpp"
#include "Utils.hpp"

/**
 * @brief Construct a new Individual object with a specified number of depots.
 *        Randomizes the number of ambulances at each depot immediately after construction.
 * @param numDepots Number of depots in the genotype.
 * @param numAmbulances Number of ambulances to distribute across the depots.
 */
Individual::Individual(int numDepots, int numAmbulances) : genotype(numDepots, 0), numAmbulances(numAmbulances) {
    randomizeAmbulances(); // Randomize ambulances as part of the construction process.
}

/**
 * @brief Randomizes the number of ambulances at each depot.
 */
void Individual::randomizeAmbulances() {
    std::fill(genotype.begin(), genotype.end(), 0); // Reset all depots to 0 ambulances

    for (int i = 0; i < numAmbulances; ++i) {
        int depotIndex = Utils::randomInt(0, genotype.size() - 1);
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
 * @brief Evaluates the fitness of an individual
 * @return the fitness of the individual.
 */
 void Individual::evaluateFitness() const {
        const double ideal = static_cast<double>(numAmbulances) / numDepots;

        for (int ambulancesInDepot : genotype) {
            // Penalize deviation from the ideal number of ambulances per depot
            fitness += 1.0 - abs(ambulancesInDepot - ideal) / ideal;
        }

        // Normalize fitness to a value between 0 and 1
        fitness = std::max(0.0, fitness / numDepots);
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

double Individual::getFitness() const {
    return fitness;
};

void Individual::setFitness(double newFitness) {
    fitness = newFitness;
};

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
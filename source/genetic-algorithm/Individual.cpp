/**
 * @file Individual.hpp
 *
 * @copyright Copyright (c) 2024 Emergency-Optimizers
 */

/* internal libraries */
#include "genetic-algorithm/Individual.hpp"
#include "Utils.hpp"

Individual::Individual() { }

Individual::Individual(int numDepots, int numAmbulances, double mutationProbability, bool child)
: genotype(numDepots, 0), numDepots(numDepots), numAmbulances(numAmbulances), fitness(0.0), mutationProbability(mutationProbability), child(child) {
    if (!child) {
        randomizeAmbulances();  // Randomize ambulances as part of the construction process.
    }
    evaluateFitness();
}

void Individual::randomizeAmbulances() {
    std::fill(genotype.begin(), genotype.end(), 0);  // Reset all depots to 0 ambulances

    for (int i = 0; i < numAmbulances; ++i) {
        int depotIndex = Utils::randomInt(0, genotype.size() - 1);
        genotype[depotIndex]++;
    }
}

bool Individual::isValid() const {
    int totalAmbulances = std::accumulate(genotype.begin(), genotype.end(), 0);
    return totalAmbulances == numAmbulances;
    }

void Individual::evaluateFitness() const {
    fitness = 0.0;

    for (size_t i = 0; i < genotype.size(); ++i) {
        if (i == 0) {  // Check if it's depot 1
            fitness += genotype[i];  // Add points for ambulances in depot 1
        }
        // No action for other depots as they contribute 0 points
    }

    fitness = std::max(0.0, fitness);  // Ensure fitness is non-negative
}

void Individual::mutate() {
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_real_distribution<> probDist(0.0, 1.0);
    std::uniform_int_distribution<> depotDist(0, genotype.size() - 1);

    for (int depot = 0; depot < genotype.size(); ++depot) {
        if (probDist(gen) < mutationProbability && genotype[depot] > 0) {  // Only mutate if depot has at least one ambulance
            int otherDepot = depotDist(gen);
            while (otherDepot == depot) {
                otherDepot = depotDist(gen);  // Find a different depot
            }

            // Transfer an ambulance from current depot to another depot
            genotype[depot]--;
            genotype[otherDepot]++;
        }
    }

    // Ensure total number of ambulances remains constant
    if (!isValid()) {
        throw std::runtime_error("Total number of ambulances changed during mutation.");
    }
}

void Individual::repair() {
    int totalAmbulances = std::accumulate(genotype.begin(), genotype.end(), 0);

    while (totalAmbulances != numAmbulances) {
        if (totalAmbulances < numAmbulances) {
            addAmbulances(numAmbulances - totalAmbulances);
        } else {
            removeAmbulances(totalAmbulances - numAmbulances);
        }

        totalAmbulances = std::accumulate(genotype.begin(), genotype.end(), 0);  // Update the total after modifications
    }

    // Ensure total number of ambulances remains constant
    if (!isValid()) {
        throw std::runtime_error("Total number of ambulances changed during mutation.");
    }
}

void Individual::addAmbulances(int ambulancesToAdd) {
    std::random_device rd;
    std::mt19937 gen(rd());

    for (int i = 0; i < ambulancesToAdd; ++i) {
        std::uniform_int_distribution<> dis(0, genotype.size() - 1);
        int depotIndex = dis(gen);

        genotype[depotIndex]++;
    }
}

void Individual::removeAmbulances(int ambulancesToRemove) {
    std::random_device rd;
    std::mt19937 gen(rd());

    for (int i = 0; i < ambulancesToRemove; ++i) {
        bool ambulanceRemoved = false;
        while (!ambulanceRemoved) {
            std::uniform_int_distribution<> dis(0, genotype.size() - 1);
            int depotIndex = dis(gen);

            // Check if the selected depot has an ambulance
            if (genotype[depotIndex] > 0) {
                genotype[depotIndex]--;
                ambulanceRemoved = true;  // Ambulance removed, exit the while loop
            }
        }
    }
}

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
}

void Individual::setFitness(double newFitness) {
    fitness = newFitness;
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
}

// Individual.cpp

#include "Individual.hpp"
#include "Utils.hpp"


// Default constructor, not to be used but necessary to use resize() method
Individual::Individual() {
    // Print a debug message or throw an exception
    std::cerr << "Warning: Default constructor of Individual called!" << std::endl;
    throw std::runtime_error("Default constructor of Individual should not be used.");
}

/**
 * @brief Construct a new Individual object with a specified number of depots.
 *        Randomizes the number of ambulances at each depot immediately after construction.
 * @param numDepots Number of depots in the genotype.
 * @param numAmbulances Number of ambulances to distribute across the depots.
 */
Individual::Individual(int numDepots, int numAmbulances, double mutationProbability) : genotype(numDepots, 0), numAmbulances(numAmbulances), fitness(0.0), mutationProbability(mutationProbability) {
    randomizeAmbulances(); // Randomize ambulances as part of the construction process.
    evaluateFitness();
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
    fitness = 0.0;
    const int maxAmbulancesPerDepot = 2;  // Maximum ideal ambulances per depot, 45/19 ~= 2.3

    for (int ambulancesInDepot : genotype) {
        if (ambulancesInDepot <= maxAmbulancesPerDepot) {
            fitness += ambulancesInDepot; // Add to fitness if within limit
        } else {
            fitness -= (ambulancesInDepot - maxAmbulancesPerDepot); // Penalize if over the limit
        }
    }

    fitness = std::max(0.0, fitness); // Ensure fitness is non-negative
}

/**
 * @brief Mutates the individual by randomly redistributing ambulances between depots.
 * 
 * This method iterates over each depot in the genotype and, with a probability
 * equal to the mutation probability, transfers an ambulance from one depot to another.
 * It ensures that the total number of ambulances remains constant and that no ambulance
 * is removed from an empty depot. If the final state is not valid, an exception is thrown.
 */
void Individual::mutate() {
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_real_distribution<> probDist(0.0, 1.0);
    std::uniform_int_distribution<> depotDist(0, genotype.size() - 1);

    for (int depot = 0; depot < genotype.size(); ++depot) {
        if (probDist(gen) < mutationProbability) {
            int otherDepot = depotDist(gen);
            while (otherDepot == depot || genotype[otherDepot] == 0) {
                otherDepot = depotDist(gen); // Find a different depot with at least one ambulance
            }

            // Increment the current depot and decrement the other depot
            genotype[depot]++;
            genotype[otherDepot]--;

            std::cout << "Mutation occured between " << depot << " and " << otherDepot << std::endl;

            // Ensure total number of ambulances remains constant
            if (!isValid()) {
                throw std::runtime_error("Total number of ambulances changed during mutation.");
            }
        }
    }
}

/**
 * @brief Adds a specified number of ambulances to random depots in the genotype.
 * 
 * This method uniformly selects random depots and increments their ambulance count.
 * The default number of ambulances to add is 1, but this can be overridden by the caller.
 * 
 * @param ambulancesToAdd Number of ambulances to add. Default is 1.
 */
void Individual::addAmbulance(int ambulancesToAdd = 1) {
    std::random_device rd;
    std::mt19937 gen(rd());

    for (int i = 0; i < ambulancesToAdd; ++i) {
        std::uniform_int_distribution<> dis(0, genotype.size() - 1);
        int depotIndex = dis(gen);

        genotype[depotIndex]++;
    }
}

/**
 * @brief Removes a specified number of ambulances from random depots in the genotype.
 * 
 * This method uniformly selects random depots and decrements their ambulance count,
 * ensuring that no ambulance is removed from an empty depot. The default number of
 * ambulances to remove is 1, but this can be overridden by the caller.
 * 
 * @param ambulancesToRemove Number of ambulances to remove. Default is 1.
 */
void Individual::removeAmbulance(int ambulancesToRemove = 1) {
    std::random_device rd;
    std::mt19937 gen(rd());

    for (int i = 0; i < ambulancesToRemove; ++i) {
        std::uniform_int_distribution<> dis(0, genotype.size() - 1);
        int depotIndex = dis(gen);

        // Ensure we don't remove an ambulance from an empty depot
        if (genotype[depotIndex] > 0) {
            genotype[depotIndex]--;
        } else {
            // Optional: Try another depot or handle the case when there are no ambulances left
        }
    }
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
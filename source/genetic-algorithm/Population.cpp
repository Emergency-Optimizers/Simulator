/**
 * @file Population.cpp
 *
 * @copyright Copyright (c) 2024 Emergency-Optimizers
 */

/* internal libraries */
#include "genetic-algorithm/Population.hpp"

/**
 * @brief Construct a new Population object.
 * @param size Size of the population.
 */
Population::Population(int populationSize, int numDepots, int numAmbulances, double mutationProbability)
: populationSize(populationSize), numDepots(numDepots), numAmbulances(numAmbulances), mutationProbability(mutationProbability) {
    for (int i = 0; i < populationSize; ++i) {
        individuals.push_back(Individual(numDepots, numAmbulances, mutationProbability, false));
    }
}

/**
 * @brief Evaluates the fitness of each individual in the population.
 */
void Population::evaluateFitness() {
    for (Individual& individual : individuals) {
        individual.evaluateFitness();
    }
}

/**
 * @brief Selects individuals for reproduction based on their fitness.
 * @return std::vector<Individual> A vector of selected individuals.
 */
std::vector<Individual> Population::parentSelection(int numParents, int tournamentSize) {
    std::vector<Individual> selectedParents;
    std::random_device rd;
    std::mt19937 gen(rd());

    for (int i = 0; i < numParents; ++i) {
        std::vector<Individual> tournament;
        for (int j = 0; j < tournamentSize; ++j) {
            std::uniform_int_distribution<> dis(0, individuals.size() - 1);
            int randomIndex = dis(gen);
            tournament.push_back(individuals[randomIndex]);
        }

        // Select the individual with the highest fitness in the tournament
        auto best = std::max_element(tournament.begin(), tournament.end(),
            [](const Individual &a, const Individual &b) {
                return a.getFitness() < b.getFitness();
            });

        selectedParents.push_back(*best);
    }

    return selectedParents;
}

/**
 * @brief Performs survivor selection for the genetic algorithm.
 * 
 * This method sorts the current population based on fitness in descending order
 * and then resizes the population to keep only the top 'numSurvivors' individuals.
 * This ensures that only the fittest individuals are carried over to the next generation.
 *
 * @param numSurvivors The number of individuals to survive into the next generation.
 * @return std::vector<Individual> The vector of survivors after selection.
 */
std::vector<Individual> Population::survivorSelection(int numSurvivors) {
    std::sort(individuals.begin(), individuals.end(),
        [](const Individual &a, const Individual &b) { return a.getFitness() > b.getFitness(); });

    if (numSurvivors < individuals.size()) {
        individuals.resize(numSurvivors);  // Keep only the top individuals
    }
    return individuals;  // Return the resized population
}

/**
 * @brief Adds a vector of new children (offspring) to the existing population.
 * 
 * This method is used to insert a new generation of individuals (usually offspring
 * created through crossover and mutation) into the current population. The children
 * are added to the end of the population vector.
 *
 * @param children A vector of Individual objects representing the children to be added.
 */
void Population::addChildren(const std::vector<Individual>& children) {
    individuals.insert(individuals.end(), children.begin(), children.end());
}

/**
 * @brief Applies crossover operation to create new individuals.
 * @param parents A vector of selected individuals for reproduction.
 * @return Individual The offspring of parent 1 and 2.
 */
Individual Population::crossover(const Individual& parent1, const Individual& parent2) {
    if (parent1.getGenotype().size() != parent2.getGenotype().size()) {
        throw std::invalid_argument("Genotypes of parents must be of the same size.");
    }

    std::vector<int> offspringGenotype;
    std::random_device rd;
    std::mt19937 gen(rd());
    offspringGenotype.reserve(parent1.getGenotype().size());

    std::uniform_real_distribution<> dist(0.0, 1.0);

    // Perform crossover
    for (size_t i = 0; i < parent1.getGenotype().size(); ++i) {
        double alpha = dist(gen);  // Weight for averaging
        int gene = static_cast<int>(alpha * parent1.getGenotype()[i] + (1 - alpha) * parent2.getGenotype()[i]);
        offspringGenotype.push_back(gene);
    }
    Individual offspring = Individual(numDepots, numAmbulances, mutationProbability);
    offspring.setGenotype(offspringGenotype);
    offspring.repair();
    return offspring;
}


/**
 * @brief Runs the genetic algorithm for a specified number of generations.
 * @param generations Number of generations to evolve.
 */
void Population::evolve(int generations) {
    for (int gen = 0; gen < generations; ++gen) {
        std::cout << "Generation " << gen  << ": " << std::endl;

        // Step 1: Parent Selection
        int numParents = 2;  // For simplicity, selecting two parents for crossover
        int tournamentSize = 5;  // Tournament size for parent selection
        std::vector<Individual> parents = parentSelection(numParents, tournamentSize);


        // Step 2: Crossover to create offspring
        std::vector<Individual> children;
        children.reserve(populationSize);  // Assuming one child per pair of parents
        for (int i = 0; i < populationSize; i += 2) {
            Individual offspring = crossover(parents[i % numParents], parents[(i+1) % numParents]);
            offspring.mutate();  // Apply mutation to the offspring
            offspring.evaluateFitness();  // Evaluate the fitness of the offspring
            children.push_back(offspring);
        }

        // Step 3: Survivor Selection
        // Combining existing population with children
        addChildren(children);
        survivorSelection(populationSize);  // Keep only the top individuals

        // Optional: Find and print the fittest individual of this generation
        Individual fittest = findFittest();
      }
}

/**
 * @brief Finds the fittest individual in the population.
 * @return Individual The fittest individual.
 */
Individual Population::findFittest() const {
    auto fittest = std::max_element(individuals.begin(), individuals.end(),
        [](const Individual &a, const Individual &b) {
            return a.getFitness() < b.getFitness();
        });
    return *fittest;
}


Individual Population::findLeastFit() const {
    auto leastFit = std::min_element(individuals.begin(), individuals.end(),
    [](const Individual &a, const Individual &b) {
        return a.getFitness() < b.getFitness();
    });

    return *leastFit;
}

double Population::averageFitness() const {
    if (individuals.empty()) {
        return 0.0;  // Avoid division by zero
    }

    double totalFitness = std::accumulate(individuals.begin(), individuals.end(), 0.0,
        [](double sum, const Individual& individual) {
            return sum + individual.getFitness();
        });

    return totalFitness / individuals.size();
}

/**
 * @brief Sets the population size.
 * @param size New population size.
 */
void setPopulationSize(int size);


std::vector<Individual> Population::getIndividuals() const {
    return individuals;
}

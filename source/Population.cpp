#include <vector>

#include "Population.hpp"
#include "Individual.hpp"

/**
 * @brief Construct a new Population object.
 * @param size Size of the population.
 */
Population::Population(int populationSize, int numDepots, int numAmbulances, double mutationProbability) {
    for (int i = 0; i < populationSize; ++i) {
        individuals.push_back(Individual(numDepots, numAmbulances, mutationProbability));
    }
}

/**
 * @brief Evaluates the fitness of each individual in the population.
 */
void Population::evaluateFitness() {
    for (Individual& individual : individuals)
    {
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
    return individuals; // Return the resized population
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
 * @return std::vector<Individual> A vector of offspring individuals.
 */
std::vector<Individual> crossover(const std::vector<Individual>& parents);

/**
 * @brief Applies mutation to individuals in the population.
 */
void mutate();

/**
 * @brief Replaces the least fit individuals with new offspring.
 * @param offspring A vector of offspring individuals.
 */
void replace(const std::vector<Individual>& offspring);

/**
 * @brief Runs the genetic algorithm for a specified number of generations.
 * @param generations Number of generations to evolve.
 */
void evolve(int generations);

/**
 * @brief Finds the fittest individual in the population.
 * @return Individual The fittest individual.
 */
Individual findFittest();

/**
 * @brief Gets the current population.
 * @return std::vector<Individual> The current population.
 */
/// std::vector<Individual> getPopulation() const;

/**
 * @brief Sets the population size.
 * @param size New population size.
 */
void setPopulationSize(int size);


std::vector<Individual> Population::getIndividuals() const {
    return individuals;
}
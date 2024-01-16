#include <vector>

#include "Population.hpp"
#include "Individual.hpp"


/**
 * @brief Construct a new Population object.
 * @param size Size of the population.
 */
Population::Population(int populationSize, int numDepots, int numAmbulances) {
    for (int i = 0; i < populationSize; ++i) {
        individuals.push_back(Individual(numDepots, numAmbulances));
    }
}

/**
 * @brief Initializes the population with random individuals.
 */
void initialize();

/**
 * @brief Evaluates the fitness of each individual in the population.
 */
void evaluateFitness();

/**
 * @brief Selects individuals for reproduction based on their fitness.
 * @return std::vector<Individual> A vector of selected individuals.
 */
std::vector<Individual> selectForReproduction();

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
std::vector<Individual> getPopulation() const;

/**
 * @brief Sets the population size.
 * @param size New population size.
 */
void setPopulationSize(int size);

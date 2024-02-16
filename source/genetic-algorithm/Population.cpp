/**
 * @file Population.cpp
 *
 * @copyright Copyright (c) 2024 Emergency-Optimizers
 */

/* internal libraries */
#include "genetic-algorithm/Population.hpp"
#include "Utils.hpp"

Population::Population(std::mt19937& rnd, int populationSize, int numDepots, int numAmbulances, double mutationProbability)
: rnd(rnd), populationSize(populationSize), numDepots(numDepots), numAmbulances(numAmbulances), mutationProbability(mutationProbability) {
    for (int i = 0; i < populationSize; i++) {
        individuals.push_back(Individual(numDepots, numAmbulances, mutationProbability, false));
    }
}

void Population::evaluateFitness() {
    for (Individual& individual : individuals) {
        individual.evaluateFitness();
    }
}

std::vector<Individual> Population::parentSelection(int numParents, int tournamentSize) {
    std::vector<Individual> selectedParents;

    for (int i = 0; i < numParents; i++) {
        std::vector<Individual> tournament;
        for (int j = 0; j < tournamentSize; j++) {
            tournament.push_back(Utils::getRandomElement(rnd, individuals));
        }

        // select the individual with the highest fitness in the tournament
        auto best = std::max_element(
            tournament.begin(),
            tournament.end(),
            [](const Individual &a, const Individual &b) {
                return a.getFitness() < b.getFitness();
            }
        );

        selectedParents.push_back(*best);
    }

    return selectedParents;
}

std::vector<Individual> Population::survivorSelection(int numSurvivors) {
    std::sort(
        individuals.begin(),
        individuals.end(),
        [](const Individual &a, const Individual &b) { return a.getFitness() > b.getFitness(); }
    );

    if (numSurvivors < individuals.size()) {
        // keep only the top individuals
        individuals.resize(numSurvivors);
    }
    return individuals;
}

void Population::addChildren(const std::vector<Individual>& children) {
    individuals.insert(individuals.end(), children.begin(), children.end());
}

Individual Population::crossover(const Individual& parent1, const Individual& parent2) {
    std::vector<int> offspringGenotype;
    offspringGenotype.reserve(parent1.getGenotype().size());

    std::uniform_real_distribution<> dist(0, 1);

    for (size_t i = 0; i < parent1.getGenotype().size(); i++) {
        double alpha = dist(rnd);
        int gene = static_cast<int>(alpha * parent1.getGenotype()[i] + (1 - alpha) * parent2.getGenotype()[i]);
        offspringGenotype.push_back(gene);
    }

    Individual offspring = Individual(numDepots, numAmbulances, mutationProbability);
    offspring.setGenotype(offspringGenotype);
    offspring.repair();

    return offspring;
}

void Population::evolve(int generations) {
    for (int gen = 0; gen < generations; gen++) {
        // step 1: parent Selection
        int numParents = 2;
        int tournamentSize = 5;
        std::vector<Individual> parents = parentSelection(numParents, tournamentSize);

        // step 2: crossover to create offspring
        std::vector<Individual> children;
        children.reserve(populationSize);

        for (int i = 0; i < populationSize; i += 2) {
            Individual offspring = crossover(parents[i % numParents], parents[(i + 1) % numParents]);
            offspring.mutate();
            offspring.evaluateFitness();
            children.push_back(offspring);
        }

        // step 3: survivor Selection
        // combining existing population with children
        addChildren(children);
        survivorSelection(populationSize);

        Individual fittest = findFittest();
        std::cout << "Generation " << gen  << ": " << fittest.getFitness() << std::endl;
      }
}

const Individual Population::findFittest() {
    auto fittest = std::max_element(
        individuals.begin(),
        individuals.end(),
        [](const Individual &a, const Individual &b) {
            return a.getFitness() < b.getFitness();
        }
    );

    return *fittest;
}

const Individual Population::findLeastFit() {
    auto leastFit = std::min_element(
        individuals.begin(),
        individuals.end(),
        [](const Individual &a, const Individual &b) {
            return a.getFitness() < b.getFitness();
        }
    );

    return *leastFit;
}

const double Population::averageFitness() {
    if (individuals.empty()) {
        return 0.0;
    }

    double totalFitness = std::accumulate(
        individuals.begin(),
        individuals.end(),
        0.0,
        [](double sum, const Individual& individual) {
            return sum + individual.getFitness();
        }
    );

    return totalFitness / individuals.size();
}

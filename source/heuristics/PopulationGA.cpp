/**
 * @file PopulationGA.cpp
 *
 * @copyright Copyright (c) 2024 Emergency-Optimizers
 */

/* external libraries */
#include <sstream>
#include <iomanip>
/* internal libraries */
#include "ProgressBar.hpp"
#include "heuristics/PopulationGA.hpp"
#include "file-reader/Settings.hpp"
#include "Utils.hpp"
#include "simulator/MonteCarloSimulator.hpp"

PopulationGA::PopulationGA(
    std::mt19937& rnd,
    int populationSize,
    double mutationProbability,
    double crossoverProbability,
    const bool dayShift
) : rnd(rnd), populationSize(populationSize), mutationProbability(mutationProbability), crossoverProbability(crossoverProbability), dayShift(dayShift) {
    MonteCarloSimulator monteCarloSim(
        rnd,
        Settings::get<int>("SIMULATE_YEAR"),
        Settings::get<int>("SIMULATE_MONTH"),
        Settings::get<int>("SIMULATE_DAY"),
        dayShift,
        Settings::get<int>("SIMULATION_GENERATION_WINDOW_SIZE")
    );

    numDepots = Stations::getInstance().getDepotIndices(dayShift).size();
    numAmbulances = dayShift ? Settings::get<int>("TOTAL_AMBULANCES_DURING_DAY") : Settings::get<int>("TOTAL_AMBULANCES_DURING_NIGHT");

    events = monteCarloSim.generateEvents();

    for (int i = 0; i < populationSize; i++) {
        IndividualGA individual = IndividualGA(rnd, numDepots, numAmbulances, mutationProbability, dayShift, false);
        individuals.push_back(individual);
    }

    evaluateFitness();
}

void PopulationGA::evaluateFitness() {
    for (IndividualGA& individual : individuals) {
        individual.evaluateFitness(events);
    }
}

std::vector<IndividualGA> PopulationGA::parentSelection(int tournamentSize) {
    std::vector<IndividualGA> selectedParents;

    for (int i = 0; i < 2; i++) {
        std::vector<IndividualGA> tournament;
        for (int j = 0; j < tournamentSize; j++) {
            tournament.push_back(getRandomElement(rnd, individuals));
        }

        // select the individual with the highest fitness in the tournament
        auto best = std::max_element(
            tournament.begin(),
            tournament.end(),
            [](const IndividualGA &a, const IndividualGA &b) {
                return a.getFitness() > b.getFitness();
            }
        );
        selectedParents.push_back(*best);
    }
    return selectedParents;
}

std::vector<IndividualGA> PopulationGA::survivorSelection(int numSurvivors) {
    // sort the current population based on fitness in descending order
    std::sort(
        individuals.begin(),
        individuals.end(),
        [](const IndividualGA &a, const IndividualGA &b) { return a.getFitness() < b.getFitness(); }
    );

    std::vector<IndividualGA> survivors;

    // calculate the actual number of survivors to keep, which is the minimum
    // of numSurvivors and the current population size to avoid out-of-bounds access
    int actualNumSurvivors = std::min(numSurvivors, static_cast<int>(individuals.size()));

    // copy the top 'actualNumSurvivors' individuals to the survivors vector
    for (int i = 0; i < actualNumSurvivors; i++) {
        survivors.push_back(individuals[i]);
    }

    return survivors;
}

void PopulationGA::addChildren(const std::vector<IndividualGA>& children) {
    for (int i = 0; i < children.size(); i++) individuals.push_back(children[i]);
}

IndividualGA PopulationGA::crossover(const IndividualGA& parent1, const IndividualGA& parent2) {
    // Initialize the offspring genotype with the same size as the parents
    std::vector<int> offspringGenotype(parent1.getGenotype().size());

    std::uniform_real_distribution<> alphaDist(0.0, 1.0); // For deciding on crossover action
    std::uniform_int_distribution<> geneDist(0, 1); // For deciding from which parent the gene is taken

    // Decide if crossover should happen based on crossoverProbability
    if (alphaDist(rnd) <= crossoverProbability) {
        for (size_t i = 0; i < parent1.getGenotype().size(); i++) {
            // Decide from which parent to take the gene
            if (geneDist(rnd) == 1) {
                offspringGenotype[i] = parent1.getGenotype()[i];
            } else {
                offspringGenotype[i] = parent2.getGenotype()[i];
            }
        }
    } else {
        offspringGenotype = parent2.getGenotype();
    }

    // Create the offspring IndividualGA with the combined or inherited genotype
    IndividualGA offspring(rnd, numDepots, numAmbulances, mutationProbability, dayShift);
    offspring.setGenotype(offspringGenotype);
    offspring.repair();
    offspring.evaluateFitness(events);

    return offspring;
}

void PopulationGA::evolve(int generations) {
    ProgressBar progressBar(generations, "Running GA");

    for (int gen = 0; gen < generations; gen++) {
        int numParents = populationSize / 2;

        // don't branch if population size is set to 1
        // this is for debugging when we only want to simulate once
        if (numParents > 1) {
            // step 1: parent selection
            std::vector<IndividualGA> offspring;
            int tournamentSize = 3;
            std::uniform_real_distribution<> shouldCrossover(0.0, 1.0);

            while (offspring.size() < populationSize) {
                if (shouldCrossover(rnd) < crossoverProbability) {           
                    std::vector<IndividualGA> parents = parentSelection(tournamentSize);
                    IndividualGA child = crossover(parents[0], parents[1]);
                    child.mutate();
                    child.evaluateFitness(events);
                    offspring.push_back(child);
                }
            }
            // step 3: survivor selection
            // combining existing population with children
            addChildren(offspring);

            individuals = survivorSelection(populationSize);
        }

        IndividualGA fittest = findFittest();

        std::ostringstream postfix;
        postfix
            << "Best fitness: " << std::fixed << std::setprecision(2) << std::setw(6) << fittest.getFitness()
            << ", Valid: " << (fittest.isValid() ? "true " : "false");

        progressBar.update(gen + 1, postfix.str());
    }

    // run one last time to print metrics
    IndividualGA finalIndividual = findFittest();
    bool saveMetricsToFile = true;
    finalIndividual.evaluateFitness(events, saveMetricsToFile);

    std::cout
        << "Fittest IndividualGA: " << finalIndividual.getFitness()
        << (finalIndividual.isValid() ? " [valid]\n" : " [invalid]\n")
        << std::endl;
    finalIndividual.printChromosome();
}

int PopulationGA::countUnique(const std::vector<IndividualGA>& population) {
    std::vector<std::vector<int>> genotypes;
    genotypes.reserve(population.size());

    for (const auto& individual : population) {
        genotypes.push_back(individual.getGenotype());
    }

    // sort genotypes to bring identical ones together
    std::sort(genotypes.begin(), genotypes.end());

    // remove consecutive duplicates
    auto lastUnique = std::unique(genotypes.begin(), genotypes.end());

    // calculate the distance between the beginning and the point of last unique element
    return std::distance(genotypes.begin(), lastUnique);
}

const IndividualGA PopulationGA::findFittest() {
    auto fittest = std::max_element(
        individuals.begin(),
        individuals.end(),
        [](const IndividualGA &a, const IndividualGA &b) {
            return a.getFitness() > b.getFitness();
        }
    );

    return *fittest;
}

const IndividualGA PopulationGA::findLeastFit() {
    auto leastFit = std::min_element(
        individuals.begin(),
        individuals.end(),
        [](const IndividualGA &a, const IndividualGA &b) {
            return a.getFitness() > b.getFitness();
        }
    );

    return *leastFit;
}

const double PopulationGA::averageFitness() {
    if (individuals.empty()) {
        return 0.0;
    }

    double totalFitness = std::accumulate(
        individuals.begin(),
        individuals.end(),
        0.0,
        [](double sum, const IndividualGA& individual) {
            return sum + individual.getFitness();
        }
    );

    return totalFitness / individuals.size();
}

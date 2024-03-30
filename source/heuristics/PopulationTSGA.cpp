/**
 * @file PopulationTSGA.cpp
 *
 * @copyright Copyright (c) 2024 Emergency-Optimizers
 */

/* external libraries */
#include <sstream>
#include <iomanip>
/* internal libraries */
#include "ProgressBar.hpp"
#include "heuristics/PopulationTSGA.hpp"
#include "file-reader/Settings.hpp"
#include "Utils.hpp"
#include "simulator/MonteCarloSimulator.hpp"

PopulationTSGA::PopulationTSGA(
    std::mt19937& rnd,
    int populationSize,
    double mutationProbability,
    double crossoverProbability,
    const bool dayShift,
    int numTimeSegments
) : rnd(rnd), populationSize(populationSize), mutationProbability(mutationProbability), crossoverProbability(crossoverProbability), dayShift(dayShift), numTimeSegments(numTimeSegments) {
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
        IndividualTSGA individual = IndividualTSGA(rnd, numDepots, numAmbulances, numTimeSegments, mutationProbability, dayShift, false);
        individuals.push_back(individual);
    }

    evaluateFitness();
}

void PopulationTSGA::evaluateFitness() {
    for (IndividualTSGA& individual : individuals) {
        individual.evaluateFitness(events);
        }
}

std::vector<IndividualTSGA> PopulationTSGA::parentSelection(int tournamentSize) {
    std::vector<IndividualTSGA> selectedParents;

    for (int i = 0; i < 2; i++) {
        std::vector<IndividualTSGA> tournament;
        for (int j = 0; j < tournamentSize; j++) {
            tournament.push_back(getRandomElement(rnd, individuals));
        }

        // select the individual with the highest fitness in the tournament
        auto best = std::max_element(
            tournament.begin(),
            tournament.end(),
            [](const IndividualTSGA &a, const IndividualTSGA &b) {
                return a.getFitness() > b.getFitness();
            }
        );

        selectedParents.push_back(*best);
    }

    return selectedParents;
}

std::vector<IndividualTSGA> PopulationTSGA::survivorSelection(int numSurvivors) {
    // sort the current population based on fitness in descending order
    std::sort(
        individuals.begin(),
        individuals.end(),
        [](const IndividualTSGA &a, const IndividualTSGA &b) { return a.getFitness() < b.getFitness(); }
    );

    std::vector<IndividualTSGA> survivors;

    // calculate the actual number of survivors to keep, which is the minimum
    // of numSurvivors and the current population size to avoid out-of-bounds access
    int actualNumSurvivors = std::min(numSurvivors, static_cast<int>(individuals.size()));

    // copy the top 'actualNumSurvivors' individuals to the survivors vector
    for (int i = 0; i < actualNumSurvivors; i++) {
        survivors.push_back(individuals[i]);
    }

    return survivors;
}

void PopulationTSGA::addChildren(const std::vector<IndividualTSGA>& children) {
    for (int i = 0; i < children.size(); i++) individuals.push_back(children[i]);
}

std::vector<IndividualTSGA> PopulationTSGA::crossover(const IndividualTSGA& parent1, const IndividualTSGA& parent2) {
    // initialize offspring genotypes to respective parents
    std::vector<std::vector<int>> offspring1Genotype = parent1.getGenotype();
    std::vector<std::vector<int>> offspring2Genotype = parent2.getGenotype();

    // iterate over each time segment
    for (size_t t = 0; t < offspring1Genotype.size(); ++t) {
        // generate random midpoint for the current time segment's allocation
        std::uniform_int_distribution<> midpointDist(1, offspring1Genotype[t].size() - 2);
        size_t midpoint = midpointDist(rnd);

        // perform crossover around this randomly chosen midpoint for the current time segment
        for (size_t i = 0; i < offspring1Genotype[t].size(); ++i) {
            if (i <= midpoint) {
                offspring2Genotype[t][i] = parent1.getGenotype()[t][i];
            } else {
                offspring1Genotype[t][i] = parent2.getGenotype()[t][i];
            }
        }
    }

    IndividualTSGA offspring1 = IndividualTSGA(rnd, numDepots, numAmbulances, numTimeSegments, mutationProbability, dayShift, true);
    IndividualTSGA offspring2 = IndividualTSGA(rnd, numDepots, numAmbulances, numTimeSegments, mutationProbability, dayShift, true);

    offspring1.setGenotype(offspring1Genotype);
    offspring2.setGenotype(offspring2Genotype);

    // repair, mutate, and evaluate fitness for each offspring
    std::vector<IndividualTSGA> offspring = {offspring1, offspring2};
    for (auto& child : offspring) {
        child.repair();
        child.mutate();
        child.evaluateFitness(events);
    }

    return offspring;
}

void PopulationTSGA::evolve(int generations) {
    ProgressBar progressBar(generations, "Running TSGA");

    for (int gen = 0; gen < generations; gen++) {
        int numParents = populationSize / 2;

        // don't branch if population size is set to 1
        // this is for debugging when we only want to simulate once
        if (numParents > 1) {
            // step 1: parent selection
            std::vector<IndividualTSGA> offspring;
            int tournamentSize = 3;
            std::uniform_real_distribution<> shouldCrossover(0.0, 1.0);

            while (offspring.size() < populationSize) {
                if (shouldCrossover(rnd) < crossoverProbability) {
                    std::vector<IndividualTSGA> parents = parentSelection(tournamentSize);
                    std::vector<IndividualTSGA> children = crossover(parents[0], parents[1]);
                    
                    // calculate how many children can be added without exceeding populationSize
                    size_t spaceLeft = populationSize - offspring.size();
                    size_t childrenToAdd = std::min(children.size(), spaceLeft);
                    
                    // add children directly to offspring, ensuring not to exceed populationSize
                    offspring.insert(offspring.end(), children.begin(), children.begin() + childrenToAdd);
                }
            }
            // step 3: survivor selection
            // combining existing population with children
            addChildren(offspring);
            individuals = survivorSelection(populationSize);
        }

        IndividualTSGA fittest = findFittest();

        std::ostringstream postfix;
        postfix
            << "Best fitness: " << std::fixed << std::setprecision(2) << std::setw(6) << fittest.getFitness()
            << ", Valid: " << (fittest.isValid() ? "true" : "false") << ", Unique: " << countUnique();

        progressBar.update(gen + 1, postfix.str());
    }

    // run one last time to print metrics
    IndividualTSGA finalIndividual = findFittest();
    bool saveMetricsToFile = true;
    finalIndividual.evaluateFitness(events, saveMetricsToFile);

    std::cout
        << "Fittest IndividualGA: " << finalIndividual.getFitness()
        << (finalIndividual.isValid() ? " [valid]\n" : " [invalid]\n")
        << std::endl;
    finalIndividual.printTimeSegmentedChromosome();
}

int PopulationTSGA::countUnique() {
    std::vector<std::string> genotypeStrings;
    genotypeStrings.reserve(individuals.size());

    for (const auto& individual : individuals) {
        std::ostringstream genotypeStream;
        for (const auto& segment : individual.getGenotype()) {
            for (const auto& depotAllocation : segment) {
                genotypeStream << depotAllocation << ",";
            }
            genotypeStream << ";";
        }
        genotypeStrings.push_back(genotypeStream.str());
    }

    std::sort(genotypeStrings.begin(), genotypeStrings.end());

    auto lastUnique = std::unique(genotypeStrings.begin(), genotypeStrings.end());

    return std::distance(genotypeStrings.begin(), lastUnique);
}

const IndividualTSGA PopulationTSGA::findFittest() {
    auto fittest = std::max_element(
        individuals.begin(),
        individuals.end(),
        [](const IndividualTSGA &a, const IndividualTSGA &b) {
            return a.getFitness() > b.getFitness();
        }
    );

    return *fittest;
}

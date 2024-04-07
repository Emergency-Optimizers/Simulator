/**
 * @file PopulationGA.cpp
 *
 * @copyright Copyright (c) 2024 Emergency-Optimizers
 */

/* external libraries */
#include <sstream>
#include <iomanip>
#include <string>
#include <iostream>
/* internal libraries */
#include "ProgressBar.hpp"
#include "heuristics/PopulationGA.hpp"
#include "file-reader/Settings.hpp"
#include "Utils.hpp"
#include "simulator/MonteCarloSimulator.hpp"

PopulationGA::PopulationGA(
    std::mt19937& rnd,
    const std::vector<Event>& events,
    const bool dayShift,
    const DispatchEngineStrategyType dispatchStrategy,
    const int numAmbulancesDuringDay,
    const int numAmbulancesDuringNight,
    const int populationSize,
    const double mutationProbability,
    const double crossoverProbability,
    const int numTimeSegments
) : rnd(rnd),
    events(events),
    dayShift(dayShift),
    dispatchStrategy(dispatchStrategy),
    populationSize(populationSize),
    numDepots(Stations::getInstance().getDepotIndices(dayShift).size()),
    numAmbulances(dayShift ? numAmbulancesDuringDay : numAmbulancesDuringNight),
    mutationProbability(mutationProbability),
    crossoverProbability(crossoverProbability),
    numTimeSegments(numTimeSegments) {
    // generate initial generation of individuals
    const bool child = false;
    for (int i = 0; i < populationSize; i++) {
        IndividualGA individual = IndividualGA(
            rnd,
            numDepots,
            numAmbulances,
            numTimeSegments,
            mutationProbability,
            child
        );
        individuals.push_back(individual);
    }

    evaluateFitness();
}

void PopulationGA::evaluateFitness() {
    for (IndividualGA& individual : individuals) {
        individual.evaluate(events, dayShift, dispatchStrategy);
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
                return a.fitness > b.fitness;
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
        [](const IndividualGA &a, const IndividualGA &b) { return a.fitness < b.fitness; }
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

std::vector<IndividualGA> PopulationGA::crossover(const IndividualGA& parent1, const IndividualGA& parent2) {
    CrossoverType crossoverType = Settings::get<CrossoverType>("CROSSOVER");
    switch(crossoverType) {
        case CrossoverType::SINGLE_POINT:
          return singlePointCrossover(parent1, parent2);
        default:
           return singlePointCrossover(parent1, parent2);
    }
}

std::vector<IndividualGA> PopulationGA::singlePointCrossover(const IndividualGA& parent1, const IndividualGA& parent2) {
    // initialize offspring genotypes to respective parents
    std::vector<std::vector<int>> offspring1Genotype = parent1.genotype;
    std::vector<std::vector<int>> offspring2Genotype = parent2.genotype;

    // iterate over each time segment
    for (size_t t = 0; t < offspring1Genotype.size(); ++t) {
        // generate random midpoint for the current time segment's allocation
        std::uniform_int_distribution<> midpointDist(1, offspring1Genotype[t].size() - 2);
        size_t midpoint = midpointDist(rnd);

        // perform crossover around this randomly chosen midpoint for the current time segment
        for (size_t i = 0; i < offspring1Genotype[t].size(); ++i) {
            if (i <= midpoint) {
                offspring2Genotype[t][i] = parent1.genotype[t][i];
            } else {
                offspring1Genotype[t][i] = parent2.genotype[t][i];
            }
        }
    }

    IndividualGA offspring1 = IndividualGA(rnd, numDepots, numAmbulances, numTimeSegments, mutationProbability, true);
    IndividualGA offspring2 = IndividualGA(rnd, numDepots, numAmbulances, numTimeSegments, mutationProbability, true);

    offspring1.genotype = offspring1Genotype;
    offspring2.genotype = offspring2Genotype;

    // repair, mutate, and evaluate fitness for each offspring
    std::vector<IndividualGA> offspring = {offspring1, offspring2};
    for (auto& child : offspring) {
        child.repair();
        child.mutate();
        child.evaluate(events, dayShift, dispatchStrategy);
    }

    return offspring;
}

void PopulationGA::evolve(int generations) {
    ProgressBar progressBar(generations, "Running GA");

    for (int gen = 0; gen < generations; gen++) {
        // step 1: parent selection
        std::vector<IndividualGA> offspring;
        int tournamentSize = 3;
        std::uniform_real_distribution<> shouldCrossover(0.0, 1.0);

        while (offspring.size() < populationSize) {
            if (shouldCrossover(rnd) < crossoverProbability) {
                std::vector<IndividualGA> parents = parentSelection(tournamentSize);
                std::vector<IndividualGA> children = crossover(parents[0], parents[1]);

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

        IndividualGA fittest = findFittest();

        std::ostringstream postfix;
        postfix
            << "Best: " << std::fixed << std::setprecision(2) << std::setw(6) << fittest.fitness
            << ", Unique: " << std::setw(std::to_string(populationSize).size()) << countUnique();

        progressBar.update(gen + 1, postfix.str());
    }

    // get best individual
    IndividualGA finalIndividual = findFittest();

    // write metrics to file
    writeMetrics(finalIndividual.simulatedEvents);

    printTimeSegmentedAllocationTable(dayShift, numTimeSegments, finalIndividual.genotype);

    printAmbulanceWorkload(finalIndividual.simulatedAmbulances);
}

int PopulationGA::countUnique() {
    std::vector<std::string> genotypeStrings;
    genotypeStrings.reserve(individuals.size());

    for (const auto& individual : individuals) {
        std::ostringstream genotypeStream;
        for (const auto& segment : individual.genotype) {
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

const IndividualGA PopulationGA::findFittest() {
    auto fittest = std::max_element(
        individuals.begin(),
        individuals.end(),
        [](const IndividualGA &a, const IndividualGA &b) {
            return a.fitness > b.fitness;
        }
    );

    return *fittest;
}

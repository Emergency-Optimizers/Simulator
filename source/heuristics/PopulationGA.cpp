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
    // generate list of possible genotype inits, mutations, crossovers
    getPossibleGenotypeInits();
    getPossibleMutations();

    // generate initial generation of individuals
    const bool isChild = false;
    for (int i = 0; i < populationSize; i++) {
        individuals.push_back(createIndividual(isChild));
    }

    evaluateFitness();
}

void PopulationGA::evolve(int generations) {
    ProgressBar progressBar(generations, progressBarPrefix);

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
        for (int i = 0; i < offspring.size(); i++) {
            individuals.push_back(offspring[i]);
        }

        evaluateFitness();

        individuals = survivorSelection(populationSize);

        // update progress bar
        progressBar.update(gen + 1, getProgressBarPostfix());
    }

    // get best individual
    IndividualGA finalIndividual = getFittest();

    // write metrics to file
    writeMetrics(finalIndividual.simulatedEvents);

    printTimeSegmentedAllocationTable(dayShift, numTimeSegments, finalIndividual.genotype);

    printAmbulanceWorkload(finalIndividual.simulatedAmbulances);
}

void PopulationGA::getPossibleGenotypeInits() {
    // clear lists
    genotypeInitTypes.clear();
    genotypeInitTypeWeights.clear();

    // add types and weights if applicable
    double weight = 0.0;

    weight = Settings::get<double>("GENOTYPE_INIT_WEIGHT_RANDOM");
    if (weight > 0.0) {
        genotypeInitTypes.push_back(GenotypeInitType::RANDOM);
        genotypeInitTypeWeights.push_back(weight);
    }

    weight = Settings::get<double>("GENOTYPE_INIT_WEIGHT_EVEN");
    if (weight > 0.0) {
        genotypeInitTypes.push_back(GenotypeInitType::EVEN);
        genotypeInitTypeWeights.push_back(weight);
    }

    // check if valid
    if (genotypeInitTypes.empty()) {
        throwError("No applicable genotype init types.");
    }
}

void PopulationGA::getPossibleMutations() {
    // clear lists
    mutationTypes.clear();
    mutationTypeWeights.clear();

    // add types and weights if applicable
    double weight = 0.0;

    weight = Settings::get<double>("MUTATION_WEIGHT_REDISTRIBUTE");
    if (weight > 0.0) {
        mutationTypes.push_back(MutationType::REDISTRIBUTE);
        mutationTypeWeights.push_back(weight);
    }

    // check if valid
    if (mutationTypes.empty()) {
        throwError("No applicable mutation types.");
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

std::vector<IndividualGA> PopulationGA::crossover(const IndividualGA& parent1, const IndividualGA& parent2) {
    std::vector<std::vector<std::vector<int>>> offspringGenotypes;

    CrossoverType crossoverType = Settings::get<CrossoverType>("CROSSOVER");
    switch(crossoverType) {
        case CrossoverType::SINGLE_POINT:
            offspringGenotypes = singlePointCrossover(parent1.genotype, parent2.genotype);
            break;
        default:
            offspringGenotypes = singlePointCrossover(parent1.genotype, parent2.genotype);
            break;
    }

    const bool isChild = true;
    std::vector<IndividualGA> offspring;
    for (int i = 0; i < offspringGenotypes.size(); i++) {
        IndividualGA child = createIndividual(isChild);
        child.genotype = offspringGenotypes[i];

        child.repair();
        child.mutate(mutationTypes, mutationTypeWeights);

        offspring.push_back(child);
    }

    return offspring;
}

std::vector<std::vector<std::vector<int>>> PopulationGA::singlePointCrossover(
    const std::vector<std::vector<int>>& parent1Genotype,
    const std::vector<std::vector<int>>& parent2Genotype
) {
    // initialize offspring genotypes to respective parents
    std::vector<std::vector<int>> offspring1Genotype = parent1Genotype;
    std::vector<std::vector<int>> offspring2Genotype = parent2Genotype;

    // iterate over each time segment
    for (size_t t = 0; t < numTimeSegments; t++) {
        // generate random midpoint for the current time segment's allocation
        size_t midpoint = getRandomInt(rnd, 1, offspring1Genotype[t].size() - 2);

        // perform crossover around this randomly chosen midpoint for the current time segment
        for (size_t i = 0; i < numDepots; i++) {
            if (i <= midpoint) {
                offspring2Genotype[t][i] = parent1Genotype[t][i];
            } else {
                offspring1Genotype[t][i] = parent2Genotype[t][i];
            }
        }
    }

    std::vector<std::vector<std::vector<int>>> offspring = {offspring1Genotype, offspring2Genotype};

    return offspring;
}

IndividualGA PopulationGA::createIndividual(const bool child) {
    IndividualGA individual = IndividualGA(
        rnd,
        numDepots,
        numAmbulances,
        numTimeSegments,
        mutationProbability,
        child,
        genotypeInitTypes,
        genotypeInitTypeWeights
    );

    return individual;
}

void PopulationGA::evaluateFitness() {
    for (IndividualGA& individual : individuals) {
        individual.evaluate(events, dayShift, dispatchStrategy);
    }

    sortIndividuals();
}

void PopulationGA::sortIndividuals() {
    std::sort(
        individuals.begin(),
        individuals.end(),
        [](const IndividualGA &a, const IndividualGA &b) { return a.fitness < b.fitness; }
    );
}

const std::string PopulationGA::getProgressBarPostfix() const {
    IndividualGA fittest = getFittest();

    std::ostringstream postfix;
    postfix
        << "Best: " << std::fixed << std::setprecision(2) << std::setw(6) << fittest.fitness
        << ", Unique: " << std::setw(std::to_string(populationSize).size()) << countUnique();

    return postfix.str();
}

const IndividualGA PopulationGA::getFittest() const {
    return individuals[0];
}

const int PopulationGA::countUnique() const {
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

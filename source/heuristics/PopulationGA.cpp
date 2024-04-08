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
    getPossibleCrossovers();
    getPossibleParentSelections();

    // init population
    generatePopulation();
    evaluateFitness();
}

void PopulationGA::generatePopulation() {
    individuals.clear();

    const bool isChild = false;
    for (int i = 0; i < populationSize; i++) {
        individuals.push_back(createIndividual(isChild));
    }
}

void PopulationGA::evolve(int generations) {
    ProgressBar progressBar(generations, progressBarPrefix);

    for (int gen = 0; gen < generations; gen++) {
        // step 1: parent selection
        std::vector<IndividualGA> offspring;
        std::uniform_real_distribution<> shouldCrossover(0.0, 1.0);

        while (offspring.size() < populationSize) {
            if (shouldCrossover(rnd) < crossoverProbability) {
                std::vector<IndividualGA> parents = parentSelection();
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
    genotypeInits.clear();
    genotypeInitsTickets.clear();

    // add types and tickets if applicable
    double tickets;

    tickets = Settings::get<double>("GENOTYPE_INIT_TICKETS_RANDOM");
    if (tickets > 0.0) {
        genotypeInits.push_back(GenotypeInitType::RANDOM);
        genotypeInitsTickets.push_back(tickets);
    }

    tickets = Settings::get<double>("GENOTYPE_INIT_TICKETS_EVEN");
    if (tickets > 0.0) {
        genotypeInits.push_back(GenotypeInitType::EVEN);
        genotypeInitsTickets.push_back(tickets);
    }

    // check if valid
    if (genotypeInits.empty()) {
        throwError("No applicable genotype inits.");
    }
}

void PopulationGA::getPossibleMutations() {
    // clear lists
    mutations.clear();
    mutationsTickets.clear();

    // add types and tickets if applicable
    double tickets;

    tickets = Settings::get<double>("MUTATION_TICKETS_REDISTRIBUTE");
    if (tickets > 0.0) {
        mutations.push_back(MutationType::REDISTRIBUTE);
        mutationsTickets.push_back(tickets);
    }

    // check if valid
    if (mutations.empty()) {
        throwError("No applicable mutations.");
    }
}

void PopulationGA::getPossibleCrossovers() {
    // clear lists
    crossovers.clear();
    crossoversTickets.clear();

    // add types and tickets if applicable
    double tickets;

    tickets = Settings::get<double>("CROSSOVER_TICKETS_SINGLE_POINT");
    if (tickets > 0.0) {
        crossovers.push_back(CrossoverType::SINGLE_POINT);
        crossoversTickets.push_back(tickets);
    }

    // check if valid
    if (crossovers.empty()) {
        throwError("No applicable crossovers.");
    }
}

void PopulationGA::getPossibleParentSelections() {
    // clear lists
    parentSelections.clear();
    parentSelectionsTickets.clear();

    // add types and tickets if applicable
    double tickets;

    tickets = Settings::get<double>("PARENT_SELECTION_TICKETS_TOURNAMENT");
    if (tickets > 0.0) {
        parentSelections.push_back(SelectionType::TOURNAMENT);
        parentSelectionsTickets.push_back(tickets);
    }

    tickets = Settings::get<double>("PARENT_SELECTION_TICKETS_ROULETTE_WHEEL");
    if (tickets > 0.0) {
        parentSelections.push_back(SelectionType::ROULETTE_WHEEL);
        parentSelectionsTickets.push_back(tickets);
    }

    // check if valid
    if (parentSelections.empty()) {
        throwError("No applicable parent selections.");
    }
}

std::vector<IndividualGA> PopulationGA::parentSelection() {
    // generate population pair holding index and fitness for each individual
    // fitness is inversed so selection methods can maximize fitness
    const std::vector<std::pair<int, double>> populationIndices = generateIndexFitnessPair();

    // perform selection
    const int individualsToSelect = 2;
    std::vector<int> selectedIndices;

    switch(parentSelections[weightedLottery(rnd, parentSelectionsTickets, {})]) {
        case SelectionType::TOURNAMENT:
            selectedIndices = tournamentSelection(
                populationIndices,
                individualsToSelect,
                3
            );
            break;
        case SelectionType::ROULETTE_WHEEL:
            selectedIndices = rouletteWheelSelection(
                populationIndices,
                individualsToSelect
            );
            break;
    }

    // return selected individuals
    std::vector<IndividualGA> selectedParents;
    for (int i = 0; i < selectedIndices.size(); i++) {
        selectedParents.push_back(individuals[selectedIndices[i]]);
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

std::vector<std::pair<int, double>> PopulationGA::generateIndexFitnessPair() {
    std::vector<std::pair<int, double>> populationIndices;

    for (int individualIndex = 0; individualIndex < individuals.size(); individualIndex++) {
        // we inverse the fitness, this allows the selection methods to maximize the fitness
        double inverseFitness = 1.0 / (individuals[individualIndex].fitness + std::numeric_limits<double>::epsilon());

        populationIndices.push_back({individualIndex, inverseFitness});
    }

    return populationIndices;
}

std::vector<int> PopulationGA::tournamentSelection(
    const std::vector<std::pair<int, double>>& population,
    const int k,
    const int tournamentSize
) {
    std::vector<int> selected;

    while (selected.size() < k) {
        double bestFitness = -1.0;
        int bestIndex = -1;
        for (int i = 0; i < tournamentSize; ++i) {
            int idx = getRandomInt(rnd, 0, population.size() - 1);

            if (population[idx].second > bestFitness) {
                bestFitness = population[idx].second;
                bestIndex = population[idx].first;
            }
        }

        selected.push_back(bestIndex);
    }

    return selected;
}

std::vector<int> PopulationGA::rouletteWheelSelection(
    const std::vector<std::pair<int, double>>& population,
    const int k
) {
    std::vector<int> selected;

    double totalFitness = 0.0;
    for (const std::pair<int, double>& individual : population) {
        totalFitness += individual.second;
    }

    while (selected.size() < k) {
        double slice = getRandomDouble(rnd, 0.0, totalFitness);
        double current = 0.0;

        for (const auto& individual : population) {
            current += individual.second;

            if (current >= slice) {
                selected.push_back(individual.first);
                break;
            }
        }
    }

    return selected;
}

std::vector<IndividualGA> PopulationGA::crossover(const IndividualGA& parent1, const IndividualGA& parent2) {
    std::vector<std::vector<std::vector<int>>> offspringGenotypes;

    switch(crossovers[weightedLottery(rnd, crossoversTickets, {})]) {
        case CrossoverType::SINGLE_POINT:
            offspringGenotypes = singlePointCrossover(parent1.genotype, parent2.genotype);
            break;
    }

    const bool isChild = true;
    std::vector<IndividualGA> offspring;
    for (int i = 0; i < offspringGenotypes.size(); i++) {
        IndividualGA child = createIndividual(isChild);
        child.genotype = offspringGenotypes[i];

        child.repair();
        child.mutate(mutationProbability, mutations, mutationsTickets);

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
        numAmbulances,
        numTimeSegments,
        numDepots,
        child,
        genotypeInits,
        genotypeInitsTickets
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

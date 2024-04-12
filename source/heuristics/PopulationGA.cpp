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
#include <set>
#include <future>
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
    getPossibleSurvivorSelections();

    // init population
    generatePopulation();
    evaluateFitness();
    sortIndividuals();
    storeGenerationMetrics();
}

void PopulationGA::generatePopulation() {
    individuals.clear();

    const bool isChild = false;
    for (int i = 0; i < populationSize; i++) {
        individuals.push_back(createIndividual(isChild));
    }
}

void PopulationGA::evolve(int generations) {
    // init progress bar
    ProgressBar progressBar(generations, progressBarPrefix, getProgressBarPostfix());

    for (int generationIndex = 0; generationIndex < generations; generationIndex++) {
        // create offspring
        std::vector<IndividualGA> offspring;
        while (offspring.size() < populationSize) {
            std::vector<IndividualGA> parents = parentSelection();

            if (getRandomDouble(rnd) < crossoverProbability) {
                std::vector<IndividualGA> children = crossover(parents[0], parents[1]);

                // calculate how many children can be added without exceeding populationSize
                const size_t spaceLeft = populationSize - offspring.size();
                const size_t childrenToAdd = std::min(children.size(), spaceLeft);

                // add children directly to offspring, ensuring not to exceed populationSize
                offspring.insert(offspring.end(), children.begin(), children.begin() + childrenToAdd);
            } else {
                // clone one of the parents
                const bool isChild = true;
                IndividualGA clonedOffspring = createIndividual(isChild);

                clonedOffspring.genotype = getRandomBool(rnd) ? parents[0].genotype : parents[1].genotype;

                // apply mutation to the cloned offspring
                clonedOffspring.mutate(mutationProbability, mutations, mutationsTickets);

                offspring.push_back(clonedOffspring);
            }
        }

        // combining existing population with children
        for (int i = 0; i < offspring.size(); i++) {
            individuals.push_back(offspring[i]);
        }

        // get fitness of new individuals
        evaluateFitness();
        sortIndividuals();

        // survivor selection
        individuals = survivorSelection(populationSize);
        sortIndividuals();

        // update progress bar
        progressBar.update(generationIndex + 1, getProgressBarPostfix());

        storeGenerationMetrics();
    }

    // get best individual
    IndividualGA finalIndividual = getFittest();

    // write metrics to file
    saveDataToJson(
        Settings::get<std::string>("UNIQUE_RUN_ID"),
        heuristicName,
        metrics
    );
    writeMetrics(Settings::get<std::string>("UNIQUE_RUN_ID"), finalIndividual.simulatedEvents);

    printTimeSegmentedAllocationTable(
        dayShift,
        numTimeSegments,
        finalIndividual.genotype,
        finalIndividual.simulatedEvents,
        finalIndividual.allocationsFitness
    );

    printAmbulanceWorkload(finalIndividual.simulatedAmbulances);

    std::cout
        << "Goal:" << std::endl
        << "\t A, urban: <12 min" << std::endl
        << "\t A, rural: <25 min" << std::endl
        << "\t H, urban: <30 min" << std::endl
        << "\t H, rural: <40 min" << std::endl
        << std::endl
        << "Avg. response time (A, urban): \t\t" << finalIndividual.objectiveAvgResponseTimeUrbanA << "s"
        << " (" << (finalIndividual.objectiveAvgResponseTimeUrbanA  / 60.0) << "m)" << std::endl
        << "Avg. response time (A, rural): \t\t" << finalIndividual.objectiveAvgResponseTimeRuralA << "s"
        << " (" << (finalIndividual.objectiveAvgResponseTimeRuralA / 60.0) << "m)" << std::endl
        << "Avg. response time (H, urban): \t\t" << finalIndividual.objectiveAvgResponseTimeUrbanH << "s"
        << " (" << (finalIndividual.objectiveAvgResponseTimeUrbanH / 60.0) << "m)" << std::endl
        << "Avg. response time (H, rural): \t\t" << finalIndividual.objectiveAvgResponseTimeRuralH << "s"
        << " (" << (finalIndividual.objectiveAvgResponseTimeRuralH / 60.0) << "m)" << std::endl
        << "Avg. response time (V1, urban): \t" << finalIndividual.objectiveAvgResponseTimeUrbanV1 << "s"
        << " (" << (finalIndividual.objectiveAvgResponseTimeUrbanV1 / 60.0) << "m)" << std::endl
        << "Avg. response time (V1, rural): \t" << finalIndividual.objectiveAvgResponseTimeRuralV1 << "s"
        << " (" << (finalIndividual.objectiveAvgResponseTimeRuralV1 / 60.0) << "m)" << std::endl
        << "Percentage violations: \t\t\t" << (finalIndividual.objectivePercentageViolations * 100.0) << "%" << std::endl;

    // printAmbulanceWorkload(finalIndividual.simulatedAmbulances);
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

    tickets = Settings::get<double>("MUTATION_TICKETS_SCRAMBLE");
    if (tickets > 0.0) {
        mutations.push_back(MutationType::SCRAMBLE);
        mutationsTickets.push_back(tickets);
    }

    tickets = Settings::get<double>("MUTATION_TICKETS_NEIGHBOR_DUPLICATION");
    if (tickets > 0.0 && numTimeSegments > 1) {
        mutations.push_back(MutationType::NEIGHBOR_DUPLICATION);
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

    tickets = Settings::get<double>("CROSSOVER_TICKETS_SEGMENT_SWAP");
    if (tickets > 0.0 && numTimeSegments > 1) {
        crossovers.push_back(CrossoverType::SEGMENT_SWAP);
        crossoversTickets.push_back(tickets);
    }

    tickets = Settings::get<double>("CROSSOVER_TICKETS_BEST_ALLOCATION");
    if (tickets > 0.0 && numTimeSegments > 1) {
        crossovers.push_back(CrossoverType::BEST_ALLOCATION);
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

    tickets = Settings::get<double>("PARENT_SELECTION_TICKETS_ELITISM");
    if (tickets > 0.0) {
        parentSelections.push_back(SelectionType::ELITISM);
        parentSelectionsTickets.push_back(tickets);
    }

    tickets = Settings::get<double>("PARENT_SELECTION_TICKETS_RANK");
    if (tickets > 0.0) {
        parentSelections.push_back(SelectionType::RANK);
        parentSelectionsTickets.push_back(tickets);
    }

    // check if valid
    if (parentSelections.empty()) {
        throwError("No applicable parent selections.");
    }
}

void PopulationGA::getPossibleSurvivorSelections() {
    // clear lists
    survivorSelections.clear();
    survivorSelectionsTickets.clear();

    // add types and tickets if applicable
    double tickets;

    tickets = Settings::get<double>("SURVIVOR_SELECTION_TICKETS_TOURNAMENT");
    if (tickets > 0.0) {
        survivorSelections.push_back(SelectionType::TOURNAMENT);
        survivorSelectionsTickets.push_back(tickets);
    }

    tickets = Settings::get<double>("SURVIVOR_SELECTION_TICKETS_ROULETTE_WHEEL");
    if (tickets > 0.0) {
        survivorSelections.push_back(SelectionType::ROULETTE_WHEEL);
        survivorSelectionsTickets.push_back(tickets);
    }

    tickets = Settings::get<double>("SURVIVOR_SELECTION_TICKETS_ELITISM");
    if (tickets > 0.0) {
        survivorSelections.push_back(SelectionType::ELITISM);
        survivorSelectionsTickets.push_back(tickets);
    }

    tickets = Settings::get<double>("SURVIVOR_SELECTION_TICKETS_RANK");
    if (tickets > 0.0) {
        survivorSelections.push_back(SelectionType::RANK);
        survivorSelectionsTickets.push_back(tickets);
    }

    // check if valid
    if (survivorSelections.empty()) {
        throwError("No applicable survivor selections.");
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
                Settings::get<int>("PARENT_SELECTION_TOURNAMENT_SIZE")
            );
            break;
        case SelectionType::ROULETTE_WHEEL:
            selectedIndices = rouletteWheelSelection(
                populationIndices,
                individualsToSelect
            );
            break;
        case SelectionType::ELITISM:
            selectedIndices = elitismSelection(
                populationIndices,
                individualsToSelect
            );
            break;
        case SelectionType::RANK:
            selectedIndices = rankSelection(
                populationIndices,
                individualsToSelect,
                Settings::get<double>("PARENT_SELECTION_RANK_SELECTION_PRESSURE")
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
    // generate population pair holding index and fitness for each individual
    // fitness is inversed so selection methods can maximize fitness
    const int startIndex = Settings::get<int>("SURVIVOR_SELECTION_KEEP_N_BEST");
    const std::vector<std::pair<int, double>> populationIndices = generateIndexFitnessPair(startIndex);

    // perform selection
    const int individualsToSelect = std::min(numSurvivors, static_cast<int>(individuals.size()));
    std::vector<int> selectedIndices;

    switch(survivorSelections[weightedLottery(rnd, survivorSelectionsTickets, {})]) {
        case SelectionType::TOURNAMENT:
            selectedIndices = tournamentSelection(
                populationIndices,
                individualsToSelect,
                Settings::get<int>("SURVIVOR_SELECTION_TOURNAMENT_SIZE")
            );
            break;
        case SelectionType::ROULETTE_WHEEL:
            selectedIndices = rouletteWheelSelection(
                populationIndices,
                individualsToSelect
            );
            break;
        case SelectionType::ELITISM:
            selectedIndices = elitismSelection(
                populationIndices,
                individualsToSelect
            );
            break;
        case SelectionType::RANK:
            selectedIndices = rankSelection(
                populationIndices,
                individualsToSelect,
                Settings::get<double>("SURVIVOR_SELECTION_RANK_SELECTION_PRESSURE")
            );
            break;
    }

    // return selected individuals
    std::vector<IndividualGA> selectedSurvivors;

    // add the n best individuals
    for (int i = 0; i < Settings::get<int>("SURVIVOR_SELECTION_KEEP_N_BEST"); i++) {
        selectedSurvivors.push_back(individuals[i]);
    }

    for (int i = 0; i < selectedIndices.size(); i++) {
        selectedSurvivors.push_back(individuals[selectedIndices[i]]);
    }

    return selectedSurvivors;
}

std::vector<std::pair<int, double>> PopulationGA::generateIndexFitnessPair(const int startIndex) {
    std::vector<std::pair<int, double>> populationIndices;

    for (int individualIndex = startIndex; individualIndex < individuals.size(); individualIndex++) {
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

        for (const std::pair<int, double>& individual : population) {
            current += individual.second;

            if (current >= slice) {
                selected.push_back(individual.first);
                break;
            }
        }
    }

    return selected;
}

std::vector<int> PopulationGA::elitismSelection(
    const std::vector<std::pair<int, double>>& population,
    const int k
) {
    std::vector<int> selected;

    // assumes population is already sorted by fitness (best to worst)
    for (int individualIndex = 0; individualIndex < k && individualIndex < population.size(); individualIndex++) {
        selected.push_back(population[individualIndex].first);
    }

    return selected;
}

std::vector<int> PopulationGA::rankSelection(
    const std::vector<std::pair<int, double>>& population,
    const int k,
    const double selectionPressure
) {
    std::vector<int> selected;

    const int N = population.size();
    std::vector<double> probabilities(N);
    std::vector<double> cumulativeProbabilities(N);

    // calculate selection probabilities for each rank
    // assumes population is sorted
    double totalProbability = 0.0;
    for (int i = 0; i < N; i++) {
        probabilities[i] = (2 - selectionPressure) / N + 2 * (i + 1) * (selectionPressure - 1) / (N * (N - 1));
        totalProbability += probabilities[i];
    }

    // calculate cumulative probabilities
    cumulativeProbabilities[0] = probabilities[0];
    for (int i = 1; i < N; i++) {
        cumulativeProbabilities[i] = cumulativeProbabilities[i - 1] + probabilities[i];
    }

    // select k individuals based on cumulative probabilities
    for (int n = 0; n < k; n++) {
        const double r = getRandomDouble(rnd, 0.0, totalProbability);

        for (int i = 0; i < N; i++) {
            if (r <= cumulativeProbabilities[i]) {
                selected.push_back(population[i].first);
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
        case CrossoverType::SEGMENT_SWAP:
            offspringGenotypes = segmentSwapCrossover(parent1.genotype, parent2.genotype);
            break;
        case CrossoverType::BEST_ALLOCATION:
            offspringGenotypes = bestAllocationCrossover(
                parent1.genotype,
                parent2.genotype,
                parent1.allocationsFitness,
                parent2.allocationsFitness
            );
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

std::vector<std::vector<std::vector<int>>> PopulationGA::segmentSwapCrossover(
    const std::vector<std::vector<int>>& parent1Genotype,
    const std::vector<std::vector<int>>& parent2Genotype
) {
    // initialize offspring genotypes to respective parents
    std::vector<std::vector<int>> offspring1Genotype = parent1Genotype;
    std::vector<std::vector<int>> offspring2Genotype = parent2Genotype;

    // iterate over each time segment
    for (size_t t = 0; t < numTimeSegments; t++) {
         // swap entire time segments
        if (getRandomBool(rnd)) {
            std::swap(offspring1Genotype[t], offspring2Genotype[t]);
        }
    }

    std::vector<std::vector<std::vector<int>>> offspring = {offspring1Genotype, offspring2Genotype};

    return offspring;
}

std::vector<std::vector<std::vector<int>>> PopulationGA::bestAllocationCrossover(
    const std::vector<std::vector<int>>& parent1Genotype,
    const std::vector<std::vector<int>>& parent2Genotype,
    const std::vector<double>& parent1AllocationsFitness,
    const std::vector<double>& parent2AllocationsFitness
) {
    // initialize offspring genotypes to respective parents
    std::vector<std::vector<int>> offspring1Genotype = parent1Genotype;
    std::vector<std::vector<int>> offspring2Genotype = parent2Genotype;

    // iterate over each time segment
    for (int allocationIndex = 0; allocationIndex < numTimeSegments; allocationIndex++) {
        // swap to best parent allocation
        if (getRandomBool(rnd)) {
            if (parent1AllocationsFitness[allocationIndex] < parent2AllocationsFitness[allocationIndex]) {
                offspring2Genotype[allocationIndex] = parent1Genotype[allocationIndex];
            } else {
                offspring1Genotype[allocationIndex] = parent2Genotype[allocationIndex];
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
    for (int individualIndex = 0; individualIndex < individuals.size(); individualIndex++) {
        individuals[individualIndex].evaluate(events, dayShift, dispatchStrategy);
    }
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
    std::set<std::vector<std::vector<int>>> uniqueGenotypes;

    for (int individualIndex = 0; individualIndex < individuals.size(); individualIndex++) {
        uniqueGenotypes.insert(individuals[individualIndex].genotype);
    }

    return uniqueGenotypes.size();
}

void PopulationGA::storeGenerationMetrics() {
    std::vector<double> generationFitness;
    std::vector<double> generationDiversity;
    std::vector<double> generationAvgResponseTimeUrbanA;
    std::vector<double> generationAvgResponseTimeUrbanH;
    std::vector<double> generationAvgResponseTimeUrbanV1;
    std::vector<double> generationAvgResponseTimeRuralA;
    std::vector<double> generationAvgResponseTimeRuralH;
    std::vector<double> generationAvgResponseTimeRuralV1;
    std::vector<double> generationPercentageViolations;

    // add individual data
    for (int individualIndex = 0; individualIndex < individuals.size(); individualIndex++) {
        generationFitness.push_back(individuals[individualIndex].fitness);
        generationAvgResponseTimeUrbanA.push_back(individuals[individualIndex].objectiveAvgResponseTimeUrbanA);
        generationAvgResponseTimeUrbanH.push_back(individuals[individualIndex].objectiveAvgResponseTimeUrbanH);
        generationAvgResponseTimeUrbanV1.push_back(individuals[individualIndex].objectiveAvgResponseTimeUrbanV1);
        generationAvgResponseTimeRuralA.push_back(individuals[individualIndex].objectiveAvgResponseTimeRuralA);
        generationAvgResponseTimeRuralH.push_back(individuals[individualIndex].objectiveAvgResponseTimeRuralH);
        generationAvgResponseTimeRuralV1.push_back(individuals[individualIndex].objectiveAvgResponseTimeRuralV1);
        generationPercentageViolations.push_back(individuals[individualIndex].objectivePercentageViolations);
    }

    // add global generation data
    generationDiversity.push_back(static_cast<double>(countUnique()) / static_cast<double>(individuals.size()));

    // add to metrics
    metrics["fitness"].push_back(generationFitness);
    metrics["diversity"].push_back(generationDiversity);
    metrics["avg_response_time_urban_a"].push_back(generationAvgResponseTimeUrbanA);
    metrics["avg_response_time_urban_h"].push_back(generationAvgResponseTimeUrbanH);
    metrics["avg_response_time_urban_v1"].push_back(generationAvgResponseTimeUrbanV1);
    metrics["avg_response_time_rural_a"].push_back(generationAvgResponseTimeRuralA);
    metrics["avg_response_time_rural_h"].push_back(generationAvgResponseTimeRuralH);
    metrics["avg_response_time_rural_v1"].push_back(generationAvgResponseTimeRuralV1);
    metrics["percentage_violations"].push_back(generationPercentageViolations);
}

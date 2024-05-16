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
/* internal libraries */
#include "ProgressBar.hpp"
#include "heuristics/PopulationGA.hpp"
#include "Utils.hpp"
#include "simulator/MonteCarloSimulator.hpp"

PopulationGA::PopulationGA(const std::vector<Event>& events) : events(events) {
    // generate list of possible genotype inits, mutations, crossovers
    getPossibleGenotypeInits();
    getPossibleMutations();
    getPossibleCrossovers();
    getPossibleParentSelections();
    getPossibleSurvivorSelections();

    // init population
    generatePopulation();
}

void PopulationGA::generatePopulation() {
    individuals.clear();

    const bool isChild = false;
    for (int i = 0; i < populationSize; i++) {
        Individual newIndividual = createIndividual(isChild);
        newIndividual.evaluate(events, dayShift, dispatchStrategy);

        individuals.push_back(newIndividual);
    }
}

void PopulationGA::evolve(const bool verbose, std::string extraFileName) {
    // sort and store metrics for initial population
    sortIndividuals();
    storeGenerationMetrics();

    // init progress bar
    ProgressBar progressBar(maxRunTimeSeconds, "Running " + getHeuristicName(), getProgressBarPostfix());
    startRunTimeClock = std::chrono::high_resolution_clock::now();

    bool keepRunning = true;
    bool autoStopProgressBar = false;
    bool lastPrintProgressBar = false;

    while (keepRunning) {
        generation++;

        // create offspring
        std::vector<Individual> offspring = createOffspring();

        // combining existing population with children
        for (int i = 0; i < offspring.size(); i++) {
            individuals.push_back(offspring[i]);
        }

        // survivor selection
        individuals = survivorSelection();
        sortIndividuals();

        // update progress bar
        storeGenerationMetrics();

        keepRunning = !shouldStop();
        progressBar.update(runTimeDuration, getProgressBarPostfix(), autoStopProgressBar, lastPrintProgressBar);
    }

    // get best individual and print last progress bar
    Individual finalIndividual = getFittest();

    lastPrintProgressBar = true;
    progressBar.update(runTimeDuration, getProgressBarPostfix(), autoStopProgressBar, lastPrintProgressBar);

    // write to file
    const std::string dirName = Settings::get<std::string>("UNIQUE_RUN_ID") + "_" + getHeuristicName();

    saveDataToJson(dirName, "heuristic" + extraFileName, metrics);
    writeEvents(dirName, finalIndividual.simulatedEvents, "events" + extraFileName);
    writeGenotype(dirName, finalIndividual.genotype, "genotype" + extraFileName);
    writeAmbulances(dirName, finalIndividual.simulatedAmbulances, "ambulances" + extraFileName);

    if (verbose) {
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
    }
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

    tickets = Settings::get<double>("GENOTYPE_INIT_TICKETS_UNIFORM");
    if (tickets > 0.0) {
        genotypeInits.push_back(GenotypeInitType::UNIFORM);
        genotypeInitsTickets.push_back(tickets);
    }

    tickets = Settings::get<double>("GENOTYPE_INIT_TICKETS_POPULATION_PROPORTIONATE_2KM");
    if (tickets > 0.0) {
        genotypeInits.push_back(GenotypeInitType::POPULATION_PROPORTIONATE_2KM);
        genotypeInitsTickets.push_back(tickets);
    }

    tickets = Settings::get<double>("GENOTYPE_INIT_TICKETS_POPULATION_PROPORTIONATE_5KM");
    if (tickets > 0.0) {
        genotypeInits.push_back(GenotypeInitType::POPULATION_PROPORTIONATE_5KM);
        genotypeInitsTickets.push_back(tickets);
    }

    tickets = Settings::get<double>("GENOTYPE_INIT_TICKETS_INCIDENT_PROPORTIONATE_2KM");
    if (tickets > 0.0) {
        genotypeInits.push_back(GenotypeInitType::INCIDENT_PROPORTIONATE_2KM);
        genotypeInitsTickets.push_back(tickets);
    }

    tickets = Settings::get<double>("GENOTYPE_INIT_TICKETS_INCIDENT_PROPORTIONATE_5KM");
    if (tickets > 0.0) {
        genotypeInits.push_back(GenotypeInitType::INCIDENT_PROPORTIONATE_5KM);
        genotypeInitsTickets.push_back(tickets);
    }

    tickets = Settings::get<double>("GENOTYPE_INIT_TICKETS_POPULATION_PROPORTIONATE_CLUSTER");
    if (tickets > 0.0) {
        genotypeInits.push_back(GenotypeInitType::POPULATION_PROPORTIONATE_CLUSTER);
        genotypeInitsTickets.push_back(tickets);
    }

    tickets = Settings::get<double>("GENOTYPE_INIT_TICKETS_INCIDENT_PROPORTIONATE_CLUSTER");
    if (tickets > 0.0) {
        genotypeInits.push_back(GenotypeInitType::INCIDENT_PROPORTIONATE_CLUSTER);
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

    tickets = Settings::get<double>("MUTATION_TICKETS_SWAP");
    if (tickets > 0.0) {
        mutations.push_back(MutationType::SWAP);
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

    tickets = Settings::get<double>("CROSSOVER_TICKETS_SEGMENT_SINGLE_POINT");
    if (tickets > 0.0 && numTimeSegments > 1) {
        crossovers.push_back(CrossoverType::SEGMENT_SINGLE_POINT);
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

std::vector<Individual> PopulationGA::createOffspring() {
    std::vector<Individual> offspring;

    while (offspring.size() < populationSize) {
        std::vector<Individual> parents = parentSelection();

        if (getRandomDouble(rnd) < crossoverProbability) {
            std::vector<Individual> children = crossover(parents[0], parents[1]);

            for (auto& child : children) {
                child.evaluate(events, dayShift, dispatchStrategy);
                offspring.push_back(child);
            }
        } else {
            // clone one of the parents
            const bool isChild = true;
            Individual clonedOffspring = createIndividual(isChild);

            clonedOffspring.genotype = getRandomBool(rnd) ? parents[0].genotype : parents[1].genotype;

            // apply mutation to the cloned offspring
            clonedOffspring.mutate(mutationProbability, mutations, mutationsTickets);
            clonedOffspring.evaluate(events, dayShift, dispatchStrategy);

            offspring.push_back(clonedOffspring);
        }
    }

    return offspring;
}

std::vector<Individual> PopulationGA::parentSelection() {
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
    std::vector<Individual> selectedParents;
    for (int i = 0; i < selectedIndices.size(); i++) {
        selectedParents.push_back(individuals[selectedIndices[i]]);
    }

    return selectedParents;
}

std::vector<Individual> PopulationGA::survivorSelection() {
    sortIndividuals();
    // generate population pair holding index and fitness for each individual
    // fitness is inversed so selection methods can maximize fitness
    const int startIndex = Settings::get<int>("SURVIVOR_SELECTION_KEEP_N_BEST");
    const std::vector<std::pair<int, double>> populationIndices = generateIndexFitnessPair(startIndex);

    // perform selection
    const int individualsToSelect = std::min(populationSize - startIndex, static_cast<int>(individuals.size()));
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
    std::vector<Individual> selectedSurvivors;

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
        populationIndices.push_back({individualIndex, inverseFitness(individuals[individualIndex].fitness)});
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
            int idx = getRandomInt(rnd, 0, static_cast<int>(population.size()) - 1);

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

    const int N = static_cast<int>(population.size());
    std::vector<double> probabilities(N);
    std::vector<double> cumulativeProbabilities(N);

    // calculate selection probabilities for each rank
    // assumes population is sorted in descending order (best to worst)
    double totalProbability = 0.0;
    for (int i = 0; i < N; i++) {
        probabilities[i] = (2 - selectionPressure) / N + 2 * (N - i) * (selectionPressure - 1) / (N * (N - 1));
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

std::vector<Individual> PopulationGA::crossover(const Individual& parent1, const Individual& parent2) {
    std::vector<std::vector<std::vector<int>>> offspringGenotypes;

    switch(crossovers[weightedLottery(rnd, crossoversTickets, {})]) {
        case CrossoverType::SINGLE_POINT:
            offspringGenotypes = singlePointCrossover(parent1.genotype, parent2.genotype);
            break;
        case CrossoverType::SEGMENT_SWAP:
            offspringGenotypes = segmentSwapCrossover(parent1.genotype, parent2.genotype);
            break;
        case CrossoverType::SEGMENT_SINGLE_POINT:
            offspringGenotypes = segmentSinglePointCrossover(parent1.genotype, parent2.genotype);
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
    std::vector<Individual> offspring;
    for (int i = 0; i < offspringGenotypes.size(); i++) {
        Individual child = createIndividual(isChild);
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
        size_t midpoint = getRandomInt(rnd, 1, static_cast<int>(offspring1Genotype[t].size()) - 2);

        // perform crossover around this randomly chosen midpoint for the current time segment
        for (size_t i = 0; i < numDepots; i++) {
            if (t <= midpoint) {
                offspring1Genotype[t][i] = parent1Genotype[t][i];
                offspring2Genotype[t][i] = parent2Genotype[t][i];
            } else {
                offspring1Genotype[t][i] = parent2Genotype[t][i];
                offspring2Genotype[t][i] = parent1Genotype[t][i];
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

std::vector<std::vector<std::vector<int>>> PopulationGA::segmentSinglePointCrossover(
    const std::vector<std::vector<int>>& parent1Genotype,
    const std::vector<std::vector<int>>& parent2Genotype
) {
    // initialize offspring genotypes
    std::vector<std::vector<int>> offspring1Genotype = parent1Genotype;
    std::vector<std::vector<int>> offspring2Genotype = parent2Genotype;

    // generate random midpoint
    size_t midpoint = getRandomInt(rnd, 1, numTimeSegments - 2);

    // perform crossover around this randomly chosen midpoint
    for (size_t t = 0; t < numTimeSegments; t++) {
        if (t <= midpoint) {
            offspring1Genotype[t] = parent1Genotype[t];
            offspring2Genotype[t] = parent2Genotype[t];
        } else {
            offspring1Genotype[t] = parent2Genotype[t];
            offspring2Genotype[t] = parent1Genotype[t];
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

Individual PopulationGA::createIndividual(const bool child) {
    Individual individual = Individual(
        rnd,
        numAmbulances,
        numTimeSegments,
        numDepots,
        child,
        dayShift,
        genotypeInits,
        genotypeInitsTickets
    );

    return individual;
}

void PopulationGA::sortIndividuals() {
    std::sort(
        individuals.begin(),
        individuals.end(),
        [](const Individual &a, const Individual &b) { return a.fitness < b.fitness; }
    );
}

const std::string PopulationGA::getProgressBarPostfix() const {
    const Individual fittest = getFittest();

    const double fitness = fittest.fitness;
    const double violationsUrban = fittest.objectivePercentageViolationsUrban;
    const double violationsRural = fittest.objectivePercentageViolationsRural;
    const double diversity = static_cast<double>(countUnique()) / static_cast<double>(individuals.size());

    std::ostringstream postfix;
    postfix
        << "Gen: " << std::setw(4) << generation
        << ", Div: " << std::fixed << std::setprecision(2) << std::setw(4) << diversity
        << ", Vio: ("
        << "U: " << std::fixed << std::setprecision(2) << std::setw(4) << violationsUrban
        << ", R: " << std::fixed << std::setprecision(2) << std::setw(4) << violationsRural
        << ")"
        << ", Fit: " << std::fixed << std::setprecision(2) << std::setw(7) << fitness;

    return postfix.str();
}

const std::string PopulationGA::getHeuristicName() const {
    return heuristicName;
}

const Individual PopulationGA::getFittest() const {
    return individuals[0];
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
    std::vector<double> generationPercentageViolationsUrban;
    std::vector<double> generationPercentageViolationsRural;

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
        generationPercentageViolationsUrban.push_back(individuals[individualIndex].objectivePercentageViolationsUrban);
        generationPercentageViolationsRural.push_back(individuals[individualIndex].objectivePercentageViolationsRural);
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
    metrics["percentage_violations_urban"].push_back(generationPercentageViolationsUrban);
    metrics["percentage_violations_rural"].push_back(generationPercentageViolationsRural);
}

int PopulationGA::countUnique() const {
    std::set<std::vector<std::vector<int>>> uniqueGenotypes;

    for (int individualIndex = 0; individualIndex < individuals.size(); individualIndex++) {
        uniqueGenotypes.insert(individuals[individualIndex].genotype);
    }

    return static_cast<int>(uniqueGenotypes.size());
}

bool PopulationGA::shouldStop() {
    bool stoppingCriteria = false;

    // check run time criteria
    std::chrono::steady_clock::time_point endRunTimeClock = std::chrono::high_resolution_clock::now();
    runTimeDuration = static_cast<int>(
        std::chrono::duration_cast<std::chrono::seconds>(endRunTimeClock - startRunTimeClock).count()
    );

    stoppingCriteria |= runTimeDuration > maxRunTimeSeconds;

    // check max generations criteria
    stoppingCriteria |= maxGenerations != -1 && generation >= maxGenerations;

    // check min diversity criteria
    stoppingCriteria |= minDiversity != -1 && countUnique() < minDiversity;

    // check min imrpovement
    if (Settings::get<int>("STOPPING_CRITERIA_MIN_GEN_IMPROVEMENT") != -1) {
        double currentBestViolationsUrban = individuals[0].objectivePercentageViolationsUrban;
        double currentBestViolationsRural = individuals[0].objectivePercentageViolationsRural;

        if (currentBestViolationsUrban < bestVioUrban || currentBestViolationsRural < bestVioRural) {
            bestVioUrban = currentBestViolationsUrban;
            bestVioRural = currentBestViolationsRural;

            generationsSinceImprovment = Settings::get<int>("STOPPING_CRITERIA_MIN_GEN_IMPROVEMENT");
        } else {
            generationsSinceImprovment--;
        }

        stoppingCriteria |= generationsSinceImprovment <= 0;
    }

    return stoppingCriteria;
}

/**
 * @file PopulationMA.cpp
 *
 * @copyright Copyright (c) 2024 Emergency-Optimizers
 */

/* external libraries */
#include <sstream>
#include <iomanip>
#include <string>
#include <iostream>
#include <set>
#include <algorithm>
/* internal libraries */
#include "ProgressBar.hpp"
#include "heuristics/PopulationMA.hpp"
#include "file-reader/Settings.hpp"
#include "Utils.hpp"

PopulationMA::PopulationMA(
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
) : PopulationGA(
    rnd,
    events,
    dayShift,
    dispatchStrategy,
    numAmbulancesDuringDay,
    numAmbulancesDuringNight,
    populationSize,
    mutationProbability,
    crossoverProbability,
    numTimeSegments
) { }

void PopulationMA::evolve(int generations) {
    // sort and store metrics for initial population
    sortIndividuals();
    storeGenerationMetrics();

    // init progress bar
    ProgressBar progressBar(generations, progressBarPrefix, getProgressBarPostfix());

    for (int generationIndex = 0; generationIndex < generations; generationIndex++) {
        // create offspring
        std::vector<Individual> offspring;
        while (offspring.size() < populationSize) {
            std::vector<Individual> parents = parentSelection();

            if (getRandomDouble(rnd) < crossoverProbability) {
                std::vector<Individual> children = crossover(parents[0], parents[1]);

                // calculate how many children can be added without exceeding populationSize
                const size_t spaceLeft = static_cast<size_t>(populationSize) - offspring.size();
                const size_t childrenToAdd = std::min(children.size(), spaceLeft);

                // add children directly to offspring, ensuring not to exceed populationSize
                offspring.insert(offspring.end(), children.begin(), children.begin() + childrenToAdd);
            } else {
                // clone one of the parents
                const bool isChild = true;
                Individual clonedOffspring = createIndividual(isChild);

                clonedOffspring.genotype = getRandomBool(rnd) ? parents[0].genotype : parents[1].genotype;

                // apply mutation to the cloned offspring
                clonedOffspring.mutate(mutationProbability, mutations, mutationsTickets);

                offspring.push_back(clonedOffspring);
            }
        }

        // perform local search on offspring
        for (int i = 0; i < offspring.size(); i++) {
            localSearch(offspring[i]);
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
    Individual finalIndividual = getFittest();

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
}

void PopulationMA::localSearch(Individual& individual) {
    bool improved = true;

    individual.evaluate(events, dayShift, dispatchStrategy);

    for (int allocationIndex = 0; allocationIndex < numTimeSegments; allocationIndex++) {
        improved = true;
        // std::cout << std::endl;

        while (improved) {
            improved = false;
            int bestPerformingDepotIndex = -1;
            double bestPerformance = std::numeric_limits<double>::max();
            int worstPerformingDepotIndex = -1;
            double worstPerformance = std::numeric_limits<double>::min();

            for (int depotIndex = 0; depotIndex < numDepots; depotIndex++) {
                if (individual.genotype[allocationIndex][depotIndex] < 1) {
                    continue;
                }

                double depotPerformance = responseTimeViolations(
                    individual.simulatedEvents,
                    allocationIndex,
                    depotIndex
                );

                if (depotPerformance < bestPerformance) {
                    bestPerformingDepotIndex = depotIndex;
                    bestPerformance = depotPerformance;
                } else if (depotPerformance > worstPerformance) {
                    worstPerformingDepotIndex = depotIndex;
                    worstPerformance = depotPerformance;
                }
            }

            const bool depotIndexNotFound = bestPerformingDepotIndex == -1 || worstPerformingDepotIndex == -1;
            const bool noImprovementToBeMade = bestPerformance == 0.0 && worstPerformance == 0.0;
            if (depotIndexNotFound || noImprovementToBeMade) {
                break;
            }

            Individual newIndividual = individual;
            newIndividual.genotype[allocationIndex][bestPerformingDepotIndex]--;
            newIndividual.genotype[allocationIndex][worstPerformingDepotIndex]++;

            newIndividual.metricsChecked = false;
            newIndividual.evaluate(events, dayShift, dispatchStrategy);

            /*std::cout
                << individual.fitness << " (best: " << bestPerformance << ", worst: " << worstPerformance << ") -> " << newIndividual.fitness
                << " (best: " << responseTimeViolations(newIndividual.simulatedEvents, allocationIndex, bestPerformingDepotIndex)
                << ", worst: " << responseTimeViolations(newIndividual.simulatedEvents, allocationIndex, worstPerformingDepotIndex) << ")\n";*/

            if (newIndividual.fitness < individual.fitness) {
                individual = newIndividual;
                improved = true;
            }
        }
    }
    // std::cout << "DONE: " << individual.fitness << "\n\n";
}

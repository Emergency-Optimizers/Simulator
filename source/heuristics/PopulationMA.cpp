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

PopulationMA::PopulationMA(const std::vector<Event>& events) : PopulationGA(events) { }

std::vector<Individual> PopulationMA::createOffspring() {
    std::vector<Individual> offspring = PopulationGA::createOffspring();

    for (Individual& child : offspring) {
        if (getRandomDouble(rnd) < localSearchProbability) {
            localSearch(child);
        }
    }

    return offspring;
}

void PopulationMA::localSearch(Individual& individual) {
    const int allocationIndex = getRandomInt(rnd, 0, numTimeSegments - 1);

    // double oldFitness = individual.fitness;

    int worstPerformingDepotIndex = -1;
    double worstPerformance = std::numeric_limits<double>::min();

    for (int depotIndex = 0; depotIndex < numDepots; depotIndex++) {
        double depotPerformance = responseTimeViolations(
            individual.simulatedEvents,
            allocationIndex,
            depotIndex
        );

        if (worstPerformingDepotIndex == -1 || depotPerformance > worstPerformance) {
            worstPerformingDepotIndex = depotIndex;
            worstPerformance = depotPerformance;
        }
    }

    for (int depotIndex = 0; depotIndex < numDepots; depotIndex++) {
        if (individual.genotype[allocationIndex][depotIndex] < 1) {
            continue;
        }

        if (depotIndex == worstPerformingDepotIndex) {
            continue;
        }

        Individual newIndividual = individual;
        newIndividual.genotype[allocationIndex][depotIndex]--;
        newIndividual.genotype[allocationIndex][worstPerformingDepotIndex]++;

        newIndividual.evaluate(events, dayShift, dispatchStrategy);

        /*std::cout
            << individual.fitness << " (current: " << responseTimeViolations(individual.simulatedEvents, allocationIndex, depotIndex)
            << ", worst: " << worstPerformance << ") -> " << newIndividual.fitness
            << " (current: " << responseTimeViolations(newIndividual.simulatedEvents, allocationIndex, depotIndex)
            << ", worst: " << responseTimeViolations(newIndividual.simulatedEvents, allocationIndex, worstPerformingDepotIndex) << ")\n";*/

        if (newIndividual.fitness < individual.fitness) {
            individual = newIndividual;

            break;
        }
    }

    // std::cout << "DONE: "<< oldFitness << " -> " << individual.fitness << "\n\n";
}

const std::string PopulationMA::getHeuristicName() const {
    return heuristicName;
}

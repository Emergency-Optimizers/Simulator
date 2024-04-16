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

std::vector<Individual> PopulationMA::createOffspring() {
    std::vector<Individual> offspring = PopulationGA::createOffspring();

    for (Individual& child : offspring) {
        localSearch(child);
    }

    return offspring;
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

const std::string PopulationMA::getHeuristicName() const {
    return heuristicName;
}

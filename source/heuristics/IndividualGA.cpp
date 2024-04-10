/**
 * @file IndividualGA.cpp
 *
 * @copyright Copyright (c) 2024 Emergency-Optimizers
 */

/* external libraries */
#include <iomanip>
#include <numeric>
#include <iostream>
/* internal libraries */
#include "heuristics/IndividualGA.hpp"
#include "Utils.hpp"
#include "simulator/AmbulanceAllocator.hpp"
#include "simulator/Simulator.hpp"
#include "file-reader/Settings.hpp"

IndividualGA::IndividualGA(
    std::mt19937& rnd,
    const int numAmbulances,
    const int numAllocations,
    const int numDepots,
    const bool isChild,
    const std::vector<GenotypeInitType>& genotypeInits,
    const std::vector<double>& genotypeInitsTickets
) : rnd(rnd),
    numAmbulances(numAmbulances),
    numAllocations(numAllocations),
    numDepots(numDepots),
    allocationsObjectiveAvgResponseTimeUrbanA(numAllocations, 0.0),
    allocationsObjectiveAvgResponseTimeUrbanH(numAllocations, 0.0),
    allocationsObjectiveAvgResponseTimeUrbanV1(numAllocations, 0.0),
    allocationsObjectiveAvgResponseTimeRuralA(numAllocations, 0.0),
    allocationsObjectiveAvgResponseTimeRuralH(numAllocations, 0.0),
    allocationsObjectiveAvgResponseTimeRuralV1(numAllocations, 0.0),
    allocationsObjectiveNumViolations(numAllocations, 0.0),
    allocationsFitness(numAllocations, 0.0) {
    generateGenotype(isChild, genotypeInits, genotypeInitsTickets);
}

void IndividualGA::generateGenotype(
    const bool isChild,
    const std::vector<GenotypeInitType>& inits,
    const std::vector<double>& tickets
) {
    // reset genotype
    emptyGenotype();

    // branch if individual is child and don't init genotype (will be done in crossover)
    if (isChild) {
        return;
    }

    // get random init from tickets
    switch (inits[weightedLottery(rnd, tickets, {})]) {
        case GenotypeInitType::RANDOM:
            randomGenotype();
            break;
        case GenotypeInitType::EVEN:
            evenGenotype();
            break;
    }
}

void IndividualGA::emptyGenotype() {
    genotype = std::vector<std::vector<int>>(numAllocations, std::vector<int>(numDepots, 0));
}

void IndividualGA::randomGenotype() {
    for (int allocationIndex = 0; allocationIndex < numAllocations; allocationIndex++) {
        for (int ambulanceIndex = 0; ambulanceIndex < numAmbulances; ambulanceIndex++) {
            int depotIndex = getRandomInt(rnd, 0, numDepots - 1);

            genotype[allocationIndex][depotIndex]++;
        }
    }
}

void IndividualGA::evenGenotype() {
    // calculate the base number of ambulances per depot and the remainder
    int baseAmbulancesPerDepot = numAmbulances / numDepots;
    int remainder = numAmbulances % numDepots;

    // create a vector of depot indices
    std::vector<int> depotIndices(numDepots);
    for (int depotIndex = 0; depotIndex < numDepots; depotIndex++) {
        depotIndices[depotIndex] = depotIndex;
    }

    for (int allocationIndex = 0; allocationIndex < numAllocations; allocationIndex++) {
        // distribute the base number of ambulances to each depot
        for (int depotIndex = 0; depotIndex < numDepots; depotIndex++) {
            genotype[allocationIndex][depotIndex] = baseAmbulancesPerDepot;
        }

        // shuffle the depot indices to randomize which depots get the remainder ambulances
        std::shuffle(depotIndices.begin(), depotIndices.end(), rnd);

        // evenly and randomly distribute the remainder ambulances to the depots
        for (int remainderIndex = 0; remainderIndex < remainder; remainderIndex++) {
            int depotIndex = depotIndices[remainderIndex];
            genotype[allocationIndex][depotIndex]++;
        }
    }
}

void IndividualGA::evaluate(std::vector<Event> events, const bool dayShift, const DispatchEngineStrategyType dispatchStrategy) {
    // branch if metrics is already checked
    if (metricsChecked) {
        return;
    }

    // allocate ambulances based on genotype
    AmbulanceAllocator ambulanceAllocator;
    ambulanceAllocator.allocate(events, genotype, dayShift);

    // run simulator and store results
    Simulator simulator(
        rnd,
        ambulanceAllocator,
        dispatchStrategy,
        events
    );

    simulatedEvents = simulator.run();
    simulatedAmbulances = ambulanceAllocator.ambulances;

    // sort simulated events
    std::sort(simulatedEvents.begin(), simulatedEvents.end(), [](const Event& a, const Event& b) {
        std::tm aTimeStruct = a.callReceived;
        std::tm bTimeStruct = b.callReceived;

        time_t aTime = std::mktime(&aTimeStruct);
        time_t bTime = std::mktime(&bTimeStruct);

        return aTime < bTime;
    });

    // update objectives
    objectiveAvgResponseTimeUrbanA = averageResponseTime(simulatedEvents, "A", true);
    objectiveAvgResponseTimeUrbanH = averageResponseTime(simulatedEvents, "H", true);
    objectiveAvgResponseTimeUrbanV1 = averageResponseTime(simulatedEvents, "V1", true);
    objectiveAvgResponseTimeRuralA = averageResponseTime(simulatedEvents, "A", false);
    objectiveAvgResponseTimeRuralH = averageResponseTime(simulatedEvents, "H", false);
    objectiveAvgResponseTimeRuralV1 = averageResponseTime(simulatedEvents, "V1", false);
    objectiveNumViolations = responseTimeViolations(simulatedEvents);

    // update objectives per allocation
    for (int allocationIndex = 0; allocationIndex < numAllocations; allocationIndex++) {
        allocationsObjectiveAvgResponseTimeUrbanA[allocationIndex] = averageResponseTime(simulatedEvents, "A", true, allocationIndex);
        allocationsObjectiveAvgResponseTimeUrbanH[allocationIndex] = averageResponseTime(simulatedEvents, "H", true, allocationIndex);
        allocationsObjectiveAvgResponseTimeUrbanV1[allocationIndex] = averageResponseTime(simulatedEvents, "V1", true, allocationIndex);
        allocationsObjectiveAvgResponseTimeRuralA[allocationIndex] = averageResponseTime(simulatedEvents, "A", false, allocationIndex);
        allocationsObjectiveAvgResponseTimeRuralH[allocationIndex] = averageResponseTime(simulatedEvents, "H", false, allocationIndex);
        allocationsObjectiveAvgResponseTimeRuralV1[allocationIndex] = averageResponseTime(simulatedEvents, "V1", false, allocationIndex);
        allocationsObjectiveNumViolations[allocationIndex] = responseTimeViolations(simulatedEvents, allocationIndex);
    }

    // update metrics (fitness, rank, etc.)
    updateMetrics();

    // mark as checked to not run simulator again
    metricsChecked = true;
}

void IndividualGA::updateMetrics() {
    const double weightAvgResponseTimeUrbanA = Settings::get<double>("OBJECTIVE_WEIGHT_AVG_RESPONSE_TIME_URBAN_A");
    const double weightAvgResponseTimeUrbanH = Settings::get<double>("OBJECTIVE_WEIGHT_AVG_RESPONSE_TIME_URBAN_H");
    const double weightAvgResponseTimeUrbanV1 = Settings::get<double>("OBJECTIVE_WEIGHT_AVG_RESPONSE_TIME_URBAN_V1");
    const double weightAvgResponseTimeRuralA = Settings::get<double>("OBJECTIVE_WEIGHT_AVG_RESPONSE_TIME_RURAL_A");
    const double weightAvgResponseTimeRuralH = Settings::get<double>("OBJECTIVE_WEIGHT_AVG_RESPONSE_TIME_RURAL_H");
    const double weightAvgResponseTimeRuralV1 = Settings::get<double>("OBJECTIVE_WEIGHT_AVG_RESPONSE_TIME_RURAL_V1");
    const double weightNumViolations = Settings::get<double>("OBJECTIVE_WEIGHT_NUM_VIOLATIONS");

    fitness = 0.0;
    fitness += objectiveAvgResponseTimeUrbanA * weightAvgResponseTimeUrbanA;
    fitness += objectiveAvgResponseTimeUrbanH * weightAvgResponseTimeUrbanH;
    fitness += objectiveAvgResponseTimeUrbanV1 * weightAvgResponseTimeUrbanV1;
    fitness += objectiveAvgResponseTimeRuralA * weightAvgResponseTimeRuralA;
    fitness += objectiveAvgResponseTimeRuralH * weightAvgResponseTimeRuralH;
    fitness += objectiveAvgResponseTimeRuralV1 * weightAvgResponseTimeRuralV1;
    fitness += objectiveNumViolations * weightNumViolations;

    for (int allocationIndex = 0; allocationIndex < numAllocations; allocationIndex++) {
        allocationsFitness[allocationIndex] = 0.0;
        allocationsFitness[allocationIndex] += allocationsObjectiveAvgResponseTimeUrbanA[allocationIndex] * weightAvgResponseTimeUrbanA;
        allocationsFitness[allocationIndex] += allocationsObjectiveAvgResponseTimeUrbanH[allocationIndex] * weightAvgResponseTimeUrbanH;
        allocationsFitness[allocationIndex] += allocationsObjectiveAvgResponseTimeUrbanV1[allocationIndex] * weightAvgResponseTimeUrbanV1;
        allocationsFitness[allocationIndex] += allocationsObjectiveAvgResponseTimeRuralA[allocationIndex] * weightAvgResponseTimeRuralA;
        allocationsFitness[allocationIndex] += allocationsObjectiveAvgResponseTimeRuralH[allocationIndex] * weightAvgResponseTimeRuralH;
        allocationsFitness[allocationIndex] += allocationsObjectiveAvgResponseTimeRuralV1[allocationIndex] * weightAvgResponseTimeRuralV1;
        allocationsFitness[allocationIndex] += allocationsObjectiveNumViolations[allocationIndex] * weightNumViolations;
    }
}

void IndividualGA::mutate(
    const double mutationProbability,
    const std::vector<MutationType>& mutations,
    const std::vector<double>& tickets
) {
    // get random mutation from tickets
    switch (mutations[weightedLottery(rnd, tickets, {})]) {
        case MutationType::REDISTRIBUTE:
            redistributeMutation(mutationProbability);
            break;
        case MutationType::SCRAMBLE:
            scrambleMutation(mutationProbability);
            break;
    }
}

void IndividualGA::redistributeMutation(const double mutationProbability) {
    // fill a vector of possible depot indices
    std::vector<int> depotIndices(numDepots);
    std::iota(depotIndices.begin(), depotIndices.end(), 0);

    for (int allocationIndex = 0; allocationIndex < numAllocations; allocationIndex++) {
        for (int depotIndex = 0; depotIndex < numDepots; depotIndex++) {
            // check if depot should be mutated
            if (getRandomDouble(rnd) > mutationProbability) {
                continue;
            }

            // check if depot contains any resources
            if (genotype[allocationIndex][depotIndex] <= 0) {
                continue;
            }

            // remove the selected depot index from pool of available target depot indices
            std::vector<int> potentialTargetDepotIndices = depotIndices;

            potentialTargetDepotIndices.erase(potentialTargetDepotIndices.begin() + depotIndex);

            // randomly select a target depot index
            int targetDepotIndex = getRandomElement<int>(rnd, potentialTargetDepotIndices);

            // perform the redistribution
            genotype[allocationIndex][depotIndex]--;
            genotype[allocationIndex][targetDepotIndex]++;
        }
    }
}

void IndividualGA::scrambleMutation(const double mutationProbability) {
    for (int allocationIndex = 0; allocationIndex < numAllocations; allocationIndex++) {
        // check if allocation should be mutated
        if (getRandomDouble(rnd) > mutationProbability) {
            continue;
        }

        // get a random range within the allocation
        int start = getRandomInt(rnd, 0, numDepots - 2);
        int end = getRandomInt(rnd, start + 1, numDepots - 1);

        // create a temporary vector to hold the subset to be scrambled
        std::vector<int> subsetToScramble(genotype[allocationIndex].begin() + start, genotype[allocationIndex].begin() + end + 1);

        // shuffle the subset and place it back into the genotype
        std::shuffle(subsetToScramble.begin(), subsetToScramble.end(), rnd);

        std::copy(subsetToScramble.begin(), subsetToScramble.end(), genotype[allocationIndex].begin() + start);
    }
}

void IndividualGA::repair() {
    for (int allocationIndex = 0; allocationIndex < numAllocations; allocationIndex++) {
        int totalAmbulancesInSegment = std::accumulate(genotype[allocationIndex].begin(), genotype[allocationIndex].end(), 0);

        while (totalAmbulancesInSegment != numAmbulances) {
            int depotIndex = getRandomInt(rnd, 0, numDepots - 1);

            if (totalAmbulancesInSegment < numAmbulances) {
                genotype[allocationIndex][depotIndex]++;
                totalAmbulancesInSegment++;
            } else if (totalAmbulancesInSegment > numAmbulances && genotype[allocationIndex][depotIndex] > 0) {
                genotype[allocationIndex][depotIndex]--;
                totalAmbulancesInSegment--;
            }
        }
    }

    if (!isValid()) {
        throw std::runtime_error("Repair operation failed to produce a valid solution.");
    }
}

bool IndividualGA::isValid() const {
    for (int allocationIndex = 0; allocationIndex < numAllocations; allocationIndex++) {
        // sum all ambulances allocated for segment and verify it
        int totalAmbulances = std::accumulate(genotype[allocationIndex].begin(), genotype[allocationIndex].end(), 0);

        if (totalAmbulances != numAmbulances) return false;
    }

    return true;
}

void IndividualGA::printGenotype() const {
    std::cout << "Genotype: " << std::endl;

    for (int allocationIndex = 0; allocationIndex < numAllocations; allocationIndex++) {
        std::cout << "    TS " << allocationIndex + 1 << ": ";

        for (int depotIndex = 0; depotIndex < numDepots; depotIndex++) {
            std::cout << genotype[allocationIndex][depotIndex] << " ";
        }

        std::cout << std::endl;
    }
}

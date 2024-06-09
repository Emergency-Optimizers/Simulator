/**
 * @file Individual.cpp
 *
 * @copyright Copyright (c) 2024 Emergency-Optimizers
 */

/* external libraries */
#include <iomanip>
#include <numeric>
#include <iostream>
/* internal libraries */
#include "heuristics/Individual.hpp"
#include "Utils.hpp"
#include "simulator/AmbulanceAllocator.hpp"
#include "simulator/Simulator.hpp"
#include "file-reader/Stations.hpp"

Individual::Individual(
    std::mt19937& rnd,
    const int numAmbulances,
    const int numAllocations,
    const int numDepots,
    const bool isChild,
    const bool dayShift,
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
    allocationsObjectivePercentageViolations(numAllocations, 0.0),
    allocationsObjectivePercentageViolationsUrban(numAllocations, 0.0),
    allocationsObjectivePercentageViolationsRural(numAllocations, 0.0),
    allocationsFitness(numAllocations, 0.0) {
    // init genotype
    generateGenotype(isChild, dayShift, genotypeInits, genotypeInitsTickets);
}

void Individual::generateGenotype(
    const bool isChild,
    const bool dayShift,
    const std::vector<GenotypeInitType>& inits,
    const std::vector<double>& tickets
) {
    // reset genotype
    emptyGenotype();

    // branch if individual is child and don't init genotype (will be done in crossover)
    if (isChild) {
        return;
    }

    // get random init from tickets (defined in settings.txt)
    switch (inits[weightedLottery(rnd, tickets, {})]) {
        case GenotypeInitType::RANDOM:
            randomGenotype();
            break;
        case GenotypeInitType::UNIFORM:
            uniformGenotype();
            break;
        case GenotypeInitType::POPULATION_PROPORTIONATE_2KM:
            proportionateGenotype("total_population_radius_2km", dayShift);
            break;
        case GenotypeInitType::POPULATION_PROPORTIONATE_5KM:
            proportionateGenotype("total_population_radius_5km", dayShift);
            break;
        case GenotypeInitType::INCIDENT_PROPORTIONATE_2KM:
            proportionateGenotype("total_incidents_radius_2km", dayShift);
            break;
        case GenotypeInitType::INCIDENT_PROPORTIONATE_5KM:
            proportionateGenotype("total_incidents_radius_5km", dayShift);
            break;
        case GenotypeInitType::POPULATION_PROPORTIONATE_CLUSTER:
            proportionateGenotype("total_population_cluster", dayShift);
            break;
        case GenotypeInitType::INCIDENT_PROPORTIONATE_CLUSTER:
            proportionateGenotype("total_incidents_cluster", dayShift);
            break;
    }
}

void Individual::emptyGenotype() {
    genotype = std::vector<std::vector<int>>(numAllocations, std::vector<int>(numDepots, 0));
}

void Individual::randomGenotype() {
    for (int allocationIndex = 0; allocationIndex < numAllocations; allocationIndex++) {
        for (int ambulanceIndex = 0; ambulanceIndex < numAmbulances; ambulanceIndex++) {
            int depotIndex = getRandomInt(rnd, 0, numDepots - 1);

            genotype[allocationIndex][depotIndex]++;
        }
    }
}

void Individual::uniformGenotype() {
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

        // randomly distribute the remainder ambulances to the depots
        for (int remainderIndex = 0; remainderIndex < remainder; remainderIndex++) {
            int depotIndex = depotIndices[remainderIndex];
            genotype[allocationIndex][depotIndex]++;
        }
    }
}

void Individual::proportionateGenotype(const std::string& column, const bool dayShift) {
    std::vector<unsigned int> depotIndices = Stations::getInstance().getDepotIndices(dayShift);

    // define weights based on column (total population or incidents in radius or cluster defined in data analysis)
    std::vector<double> weights(numDepots, 0.0);
    for (int depotIndex = 0; depotIndex < numDepots; depotIndex++) {
        weights[depotIndex] = static_cast<double>(Stations::getInstance().get<int>(column, depotIndices[depotIndex]));
    }

    for (int allocationIndex = 0; allocationIndex < numAllocations; allocationIndex++) {
        int numAmbulancesToAdd = numAmbulances;

        // add at least 1 ambulance to each depot
        for (int depotIndex = 0; depotIndex < numDepots; depotIndex++) {
            genotype[allocationIndex][depotIndex]++;

            numAmbulancesToAdd--;
        }

        // randomly distribute the rest of the ambulances based on weights
        for (int ambulanceIndex = 0; ambulanceIndex < numAmbulancesToAdd; ambulanceIndex++) {
            int depotIndex = weightedLottery(rnd, weights, {});

            genotype[allocationIndex][depotIndex]++;
        }
    }
}

void Individual::evaluate(std::vector<Event> events, const bool dayShift, const DispatchEngineStrategyType dispatchStrategy) {
    // allocate ambulances based on genotype
    AmbulanceAllocator ambulanceAllocator;
    ambulanceAllocator.allocate(events, genotype, dayShift);

    // run simulator with allocation and unprocessed events
    Simulator simulator(
        ambulanceAllocator,
        dispatchStrategy,
        events
    );

    simulatedEvents = simulator.run();
    simulatedAmbulances = ambulanceAllocator.ambulances;

    // sort simulated/processed events
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
    objectivePercentageViolations = responseTimeViolations(simulatedEvents);
    objectivePercentageViolationsUrban = responseTimeViolationsUrban(simulatedEvents, true);
    objectivePercentageViolationsRural = responseTimeViolationsUrban(simulatedEvents, false);

    // update objectives per allocation
    for (int allocationIndex = 0; allocationIndex < numAllocations; allocationIndex++) {
        allocationsObjectiveAvgResponseTimeUrbanA[allocationIndex] = averageResponseTime(simulatedEvents, "A", true, allocationIndex);
        allocationsObjectiveAvgResponseTimeUrbanH[allocationIndex] = averageResponseTime(simulatedEvents, "H", true, allocationIndex);
        allocationsObjectiveAvgResponseTimeUrbanV1[allocationIndex] = averageResponseTime(simulatedEvents, "V1", true, allocationIndex);
        allocationsObjectiveAvgResponseTimeRuralA[allocationIndex] = averageResponseTime(simulatedEvents, "A", false, allocationIndex);
        allocationsObjectiveAvgResponseTimeRuralH[allocationIndex] = averageResponseTime(simulatedEvents, "H", false, allocationIndex);
        allocationsObjectiveAvgResponseTimeRuralV1[allocationIndex] = averageResponseTime(simulatedEvents, "V1", false, allocationIndex);
        allocationsObjectivePercentageViolations[allocationIndex] = responseTimeViolations(simulatedEvents, allocationIndex);
        allocationsObjectivePercentageViolationsUrban[allocationIndex] = responseTimeViolationsUrban(simulatedEvents, true, allocationIndex);
        allocationsObjectivePercentageViolationsRural[allocationIndex] = responseTimeViolationsUrban(simulatedEvents, false, allocationIndex);
    }

    // update metrics (fitness, rank, etc.)
    updateMetrics();
}

void Individual::updateMetrics() {
    // update fitness, apply objective weights defined in settings.txt
    fitness = 0.0;
    fitness += objectiveAvgResponseTimeUrbanA * weightAvgResponseTimeUrbanA;
    fitness += objectiveAvgResponseTimeUrbanH * weightAvgResponseTimeUrbanH;
    fitness += objectiveAvgResponseTimeUrbanV1 * weightAvgResponseTimeUrbanV1;
    fitness += objectiveAvgResponseTimeRuralA * weightAvgResponseTimeRuralA;
    fitness += objectiveAvgResponseTimeRuralH * weightAvgResponseTimeRuralH;
    fitness += objectiveAvgResponseTimeRuralV1 * weightAvgResponseTimeRuralV1;
    fitness += objectivePercentageViolations * weightPercentageViolations;
    fitness += objectivePercentageViolationsUrban * weightPercentageViolationsUrban;
    fitness += objectivePercentageViolationsRural * weightPercentageViolationsRural;

    // update fitness per allocation
    for (int allocationIndex = 0; allocationIndex < numAllocations; allocationIndex++) {
        allocationsFitness[allocationIndex] = 0.0;
        allocationsFitness[allocationIndex] += allocationsObjectiveAvgResponseTimeUrbanA[allocationIndex] * weightAvgResponseTimeUrbanA;
        allocationsFitness[allocationIndex] += allocationsObjectiveAvgResponseTimeUrbanH[allocationIndex] * weightAvgResponseTimeUrbanH;
        allocationsFitness[allocationIndex] += allocationsObjectiveAvgResponseTimeUrbanV1[allocationIndex] * weightAvgResponseTimeUrbanV1;
        allocationsFitness[allocationIndex] += allocationsObjectiveAvgResponseTimeRuralA[allocationIndex] * weightAvgResponseTimeRuralA;
        allocationsFitness[allocationIndex] += allocationsObjectiveAvgResponseTimeRuralH[allocationIndex] * weightAvgResponseTimeRuralH;
        allocationsFitness[allocationIndex] += allocationsObjectiveAvgResponseTimeRuralV1[allocationIndex] * weightAvgResponseTimeRuralV1;
        allocationsFitness[allocationIndex] += allocationsObjectivePercentageViolations[allocationIndex] * weightPercentageViolations;
        allocationsFitness[allocationIndex] += allocationsObjectivePercentageViolationsUrban[allocationIndex] * weightPercentageViolationsUrban;
        allocationsFitness[allocationIndex] += allocationsObjectivePercentageViolationsRural[allocationIndex] * weightPercentageViolationsRural;
    }

    // get the inverse fitness for selection methods
    for (int i = 0; i < objectives.size(); i++) {
        switch (objectiveTypes[i]) {
            case ObjectiveTypes::AVG_RESPONSE_TIME_URBAN_A:
                objectives[i] = inverseFitness(objectiveAvgResponseTimeUrbanA);
                break;
            case ObjectiveTypes::AVG_RESPONSE_TIME_URBAN_H:
                objectives[i] = inverseFitness(objectiveAvgResponseTimeUrbanH);
                break;
            case ObjectiveTypes::AVG_RESPONSE_TIME_URBAN_V1:
                objectives[i] = inverseFitness(objectiveAvgResponseTimeUrbanV1);
                break;
            case ObjectiveTypes::AVG_RESPONSE_TIME_RURAL_A:
                objectives[i] = inverseFitness(objectiveAvgResponseTimeRuralA);
                break;
            case ObjectiveTypes::AVG_RESPONSE_TIME_RURAL_H:
                objectives[i] = inverseFitness(objectiveAvgResponseTimeRuralH);
                break;
            case ObjectiveTypes::AVG_RESPONSE_TIME_RURAL_V1:
                objectives[i] = inverseFitness(objectiveAvgResponseTimeRuralV1);
                break;
            case ObjectiveTypes::PERCENTAGE_VIOLATIONS:
                objectives[i] = inverseFitness(objectivePercentageViolations);
                break;
            case ObjectiveTypes::PERCENTAGE_VIOLATIONS_URBAN:
                objectives[i] = inverseFitness(objectivePercentageViolationsUrban);
                break;
            case ObjectiveTypes::PERCENTAGE_VIOLATIONS_RURAL:
                objectives[i] = inverseFitness(objectivePercentageViolationsRural);
                break;
            default:
                objectives[i] = 0.0;
        }
    }
}

void Individual::mutate(
    const double mutationProbability,
    const std::vector<MutationType>& mutations,
    const std::vector<double>& tickets
) {
    // do random mutation from tickets (defined in settings.txt)
    switch (mutations[weightedLottery(rnd, tickets, {})]) {
        case MutationType::REDISTRIBUTE:
            redistributeMutation(mutationProbability);
            break;
        case MutationType::SWAP:
            swapMutation(mutationProbability);
            break;
        case MutationType::SCRAMBLE:
            scrambleMutation(mutationProbability);
            break;
        case MutationType::NEIGHBOR_DUPLICATION:
            neighborDuplicationMutation(mutationProbability);
            break;
    }
}

void Individual::redistributeMutation(const double mutationProbability) {
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

void Individual::swapMutation(const double mutationProbability) {
    for (int allocationIndex = 0; allocationIndex < numAllocations; allocationIndex++) {
        for (int depotIndex = 0; depotIndex < numDepots; depotIndex++) {
            // check if depot should be mutated
            if (getRandomDouble(rnd) > mutationProbability) {
                continue;
            }

            // randomly select a target depot index
            int targetDepotIndex = getRandomInt(rnd, 0, numDepots - 1);

            if (depotIndex == targetDepotIndex) {
                continue;
            }

            // swap number of ambulances
            std::swap(genotype[allocationIndex][depotIndex], genotype[allocationIndex][targetDepotIndex]);
        }
    }
}

void Individual::scrambleMutation(const double mutationProbability) {
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

void Individual::neighborDuplicationMutation(const double mutationProbability) {
    for (int allocationIndex = 0; allocationIndex < numAllocations; allocationIndex++) {
        // check if allocation should be mutated
        if (getRandomDouble(rnd) > mutationProbability) {
            continue;
        }

        // check if allocation index before and after is within bounds and replace allocation
        if (allocationIndex - 1 >= 0) {
            genotype[allocationIndex - 1] = genotype[allocationIndex];
        }

        if (allocationIndex + 1 < numAllocations) {
            genotype[allocationIndex + 1] = genotype[allocationIndex];
        }

        // skip the next allocation as to not spread the same allocation too much
        allocationIndex++;
    }
}

void Individual::repair() {
    // repair genotype if needed, can happen after crossovers
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

bool Individual::isValid() const {
    // check if genotype is valid, can be invalid after crossovers
    for (int allocationIndex = 0; allocationIndex < numAllocations; allocationIndex++) {
        // sum all ambulances allocated for segment and verify it
        int totalAmbulances = std::accumulate(genotype[allocationIndex].begin(), genotype[allocationIndex].end(), 0);

        if (totalAmbulances != numAmbulances) return false;
    }

    return true;
}

void Individual::printGenotype() const {
    std::cout << "Genotype: " << std::endl;

    for (int allocationIndex = 0; allocationIndex < numAllocations; allocationIndex++) {
        std::cout << "    TS " << allocationIndex + 1 << ": ";

        for (int depotIndex = 0; depotIndex < numDepots; depotIndex++) {
            std::cout << genotype[allocationIndex][depotIndex] << " ";
        }

        std::cout << std::endl;
    }
}

bool Individual::dominates(const Individual& other) const {
    // for NSGA-II, checks domination based on objectives used (defined in settings.txt)
    bool anyBetter = false;
    for (size_t i = 0; i < objectives.size(); ++i) {
        if (objectives[i] < other.objectives[i]) {
            return false;
        }
        if (objectives[i] > other.objectives[i]) {
            anyBetter = true;
        }
    }
    return anyBetter;
}

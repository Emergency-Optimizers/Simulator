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
    const double mutationProbability,
    const int numAmbulances,
    const int numAllocations,
    const int numDepots,
    const bool isChild,
    const std::vector<GenotypeInitType>& genotypeInits,
    const std::vector<double>& genotypeInitsTickets
) : rnd(rnd),
    mutationProbability(mutationProbability),
    numAmbulances(numAmbulances),
    numAllocations(numAllocations),
    numDepots(numDepots) {
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

    // update the metrics (fitness, rank, etc.)
    updateMetrics();
}

void IndividualGA::updateMetrics() {
    fitness = averageResponseTime(simulatedEvents, "A", true);

    metricsChecked = true;
}

void IndividualGA::mutate(
    const std::vector<MutationType>& mutations,
    const std::vector<double>& tickets
) {
    // get random mutation from tickets
    switch (mutations[weightedLottery(rnd, tickets, {})]) {
        case MutationType::REDISTRIBUTE:
            redistributeMutation();
            break;
    }
}

void IndividualGA::redistributeMutation() {
    // TODO(sindre0830): this mutation used to check the mutationProbability against each depot in the segment
    // until it could mutate, then it would go to next segment, this is an alternative way. Check if we should revert.
    double cumulativeMutationProbability = mutationProbability * (static_cast<double>(genotype[0].size()) / 2.0);

    // fill a vector of possible depot indices
    std::vector<int> depotIndices(numDepots);
    std::iota(depotIndices.begin(), depotIndices.end(), 0);

    for (int allocationIndex = 0; allocationIndex < numAllocations; allocationIndex++) {
        // check if segment should be mutated
        if (getRandomProbability(rnd) > cumulativeMutationProbability) {
            continue;
        }

        // randomly select a depot index for potential mutation and check if it contains any resources
        int depotIndex = getRandomInt(rnd, 0, numDepots  - 1);

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

    // check if genotype is valid after mutation
    if (!isValid()) {
        throw std::runtime_error("Total number of ambulances changed during redistribute mutation.");
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

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
#include "heuristics/MutationType.hpp"

IndividualGA::IndividualGA(
    std::mt19937& rnd,
    int numDepots,
    int numAmbulances,
    int numTimeSegments,
    double mutationProbability,
    bool child,
    const std::vector<GenotypeInitType>& genotypeInitTypes,
    const std::vector<double>& genotypeInitTypeWeights
) : rnd(rnd),
    numDepots(numDepots),
    numAmbulances(numAmbulances),
    numTimeSegments(numTimeSegments),
    mutationProbability(mutationProbability),
    child(child) {
    // generate genotype
    if (!child) {
        generateGenotype(genotypeInitTypes, genotypeInitTypeWeights);
    } else {
        emptyGenotype();
    }
}

void IndividualGA::generateGenotype(
    const std::vector<GenotypeInitType>& initTypes,
    const std::vector<double>& initTypeWeights
) {
    // reset genotype
    emptyGenotype();

    // get random init from weights
    int initTypeIndex = weightedLottery(rnd, initTypeWeights, {});

    switch (initTypes[initTypeIndex]) {
        case GenotypeInitType::RANDOM:
            randomGenotype();
            break;
        case GenotypeInitType::EVEN:
            evenGenotype();
            break;
    }
}

void IndividualGA::emptyGenotype() {
    genotype = std::vector<std::vector<int>>(numTimeSegments, std::vector<int>(numDepots, 0));
}

void IndividualGA::randomGenotype() {
    for (int allocationIndex = 0; allocationIndex < numTimeSegments; allocationIndex++) {
        for (int i = 0; i < numAmbulances; i++) {
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
    for (int i = 0; i < numDepots; i++) {
        depotIndices[i] = i;
    }

    for (int allocationIndex = 0; allocationIndex < numTimeSegments; allocationIndex++) {
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
}

void IndividualGA::mutate() {
    // contains the init types and their weights, add new ones here
    std::vector<MutationType> types = {
        MutationType::REDISTRIBUTE,
    };

    std::vector<double> weights = {
        Settings::get<double>("MUTATION_WEIGHT_REDISTRIBUTE"),
    };

    // get random init from weights
    int typeIndex = weightedLottery(rnd, weights, {});

    switch (types[typeIndex]) {
        case MutationType::REDISTRIBUTE:
            redistributeMutation();
            break;
    }
}

void IndividualGA::redistributeMutation() {
    // TODO(sindre0830): this mutation used to check the mutationProbability against each depot in the segment
    // until it could mutate, then it would go to next segment, this is an alternative way. Check if we should revert.
    double cumulativeMutationProbability = mutationProbability * (static_cast<double>(numDepots) / 2.0);

    // fill a vector of possible depot indices
    std::vector<int> depotIndices(numDepots);
    std::iota(depotIndices.begin(), depotIndices.end(), 0);

    for (auto& segment : genotype) {
        // check if segment should be mutated
        if (getRandomProbability(rnd) > cumulativeMutationProbability) {
            continue;
        }

        // randomly select a depot index for potential mutation and check if it contains any resources
        int depotIndex = getRandomInt(rnd, 0, numDepots  - 1);

        if (segment[depotIndex] <= 0) {
            continue;
        }

        // remove the selected depot index from pool of available target depot indices
        std::vector<int> potentialTargetDepotIndices = depotIndices;

        potentialTargetDepotIndices.erase(potentialTargetDepotIndices.begin() + depotIndex);

        // randomly select a target depot index
        int targetDepotIndex = getRandomElement<int>(rnd, potentialTargetDepotIndices);

        // perform the redistribution
        segment[depotIndex]--;
        segment[targetDepotIndex]++;
    }

    // check if genotype is valid after mutation
    if (!isValid()) {
        throw std::runtime_error("Total number of ambulances changed during redistribute mutation.");
    }
}

void IndividualGA::repair() {
    for (auto& segment : genotype) {
        int totalAmbulancesInSegment = std::accumulate(segment.begin(), segment.end(), 0);

        while (totalAmbulancesInSegment != numAmbulances) {
            int depotIndex = getRandomInt(rnd, 0, numDepots - 1);

            if (totalAmbulancesInSegment < numAmbulances) {
                segment[depotIndex]++;
                totalAmbulancesInSegment++;
            } else if (totalAmbulancesInSegment > numAmbulances && segment[depotIndex] > 0) {
                segment[depotIndex]--;
                totalAmbulancesInSegment--;
            }
        }
    }

    if (!isValid()) {
        throw std::runtime_error("Repair operation failed to produce a valid solution.");
    }
}

bool IndividualGA::isValid() const {
    for (const auto& segment : genotype) {
        // sum all ambulances allocated for segment and verify it
        int totalAmbulances = std::accumulate(segment.begin(), segment.end(), 0);

        if (totalAmbulances != numAmbulances) return false;
    }

    return true;
}

void IndividualGA::printGenotype() const {
    std::cout << "Genotype: " << std::endl;

    for (int timeSegment = 0; timeSegment < numTimeSegments; timeSegment++) {
        std::cout << "    TS " << timeSegment + 1 << ": ";

        for (int depot = 0; depot < numDepots; depot++) {
            std::cout << genotype[timeSegment][depot] << " ";
        }

        std::cout << std::endl;
    }
}

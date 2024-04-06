/**
 * @file IndividualGA.cpp
 *
 * @copyright Copyright (c) 2024 Emergency-Optimizers
 */

/* external libraries */
#include <iomanip>
/* internal libraries */
#include "heuristics/IndividualGA.hpp"
#include "Utils.hpp"
#include "simulator/AmbulanceAllocator.hpp"
#include "simulator/Simulator.hpp"
#include "file-reader/Stations.hpp"
#include "file-reader/Settings.hpp"
#include "heuristics/GenotypeInitType.hpp"

IndividualGA::IndividualGA(
    std::mt19937& rnd,
    int numDepots,
    int numAmbulances,
    int numTimeSegments,
    double mutationProbability,
    const bool dayShift,
    bool child
) : rnd(rnd),
    numDepots(numDepots),
    numAmbulances(numAmbulances),
    numTimeSegments(numTimeSegments),
    mutationProbability(mutationProbability),
    dayShift(dayShift),
    child(child) {
    // generate genotype
    if (!child) {
        generateGenotype();
    } else {
        emptyGenotype();
    }
}

void IndividualGA::generateGenotype() {
    // reset genotype
    emptyGenotype();

    // contains the init types and their weights, add new ones here
    std::vector<GenotypeInitType> initTypes = {
        GenotypeInitType::RANDOM,
        GenotypeInitType::EVEN,
    };

    std::vector<double> initTypeWeights = {
        Settings::get<double>("GENOTYPE_INIT_WEIGHT_RANDOM"),
        Settings::get<double>("GENOTYPE_INIT_WEIGHT_EVEN"),
    };

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

bool IndividualGA::isValid() const {
    for (const auto& segment : genotype) {
        int totalAmbulances = std::accumulate(segment.begin(), segment.end(), 0);
        if (totalAmbulances != numAmbulances) return false;
    }
    return true;
}

void IndividualGA::evaluateFitness(std::vector<Event> events, bool saveMetricsToFile) {
    fitness = 0.0;

    AmbulanceAllocator ambulanceAllocator;
    ambulanceAllocator.allocate(events, genotype, dayShift);

    Simulator simulator(
        rnd,
        ambulanceAllocator,
        Settings::get<DispatchEngineStrategyType>("DISPATCH_STRATEGY"),
        events
    );

    simulatedEvents = simulator.run(saveMetricsToFile);

    fitness = averageResponseTime(simulatedEvents, "A", true);

    if (saveMetricsToFile) {
        int totalHours = 0;
        std::vector<int> times;
        std::cout << std::endl;
        for (int i = 0; i < ambulanceAllocator.ambulances.size(); i++) {
            totalHours += (ambulanceAllocator.ambulances[i].timeUnavailable / 60) / 60;
            times.push_back(ambulanceAllocator.ambulances[i].timeUnavailable);
            std::cout
                << "Ambulance " << i << ": "
                << "Working: " << (ambulanceAllocator.ambulances[i].timeUnavailable / 60) / 60 << " hours"
                << ", Break: " << (ambulanceAllocator.ambulances[i].timeNotWorking / 60) / 60 << " hours"
                << std::endl;
        }
        std::cout
            << "Total: " << totalHours << " hours, "
            << "Standard deviation: " << calculateStandardDeviation(times)
            << std::endl;
    }
}

void IndividualGA::mutate() {
    std::uniform_real_distribution<> probDist(0.0, 1.0);
    std::uniform_int_distribution<> depotDist(0, numDepots - 1);

    for (auto& segment : genotype) {
        for (int depot = 0; depot < segment.size(); ++depot) {
            if (probDist(rnd) < mutationProbability && segment[depot] > 0) {
                int otherDepot = depotDist(rnd);
                while (otherDepot == depot) {
                    otherDepot = depotDist(rnd);
                }

                segment[depot]--;
                segment[otherDepot]++;
            }
        }
    }

    if (!isValid()) {
        throw std::runtime_error("Total number of ambulances changed during mutation.");
    }
}

void IndividualGA::repair() {
    for (auto& segment : genotype) {
        int totalAmbulancesInSegment = std::accumulate(segment.begin(), segment.end(), 0);

        while (totalAmbulancesInSegment != numAmbulances) {
            int depotIndex = std::uniform_int_distribution<>(0, numDepots - 1)(rnd);

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

const std::vector<std::vector<int>>& IndividualGA::getGenotype() const {
    return genotype;
}

void IndividualGA::setGenotype(const std::vector<std::vector<int>> newGenotype) {
    genotype = newGenotype;
}

double IndividualGA::getFitness() const {
    return fitness;
}

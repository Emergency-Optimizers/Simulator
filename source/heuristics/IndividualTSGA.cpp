/**
 * @file IndividualTSGA.cpp
 *
 * @copyright Copyright (c) 2024 Emergency-Optimizers
 */

/* internal libraries */
#include "heuristics/IndividualTSGA.hpp"
#include "Utils.hpp"
#include "simulator/AmbulanceAllocator.hpp"
#include "simulator/Simulator.hpp"
#include "file-reader/Stations.hpp"
#include "file-reader/Settings.hpp"

IndividualTSGA::IndividualTSGA(
    std::mt19937& rnd,
    int numDepots,
    int numAmbulances,
    int numTimeSegments,
    double mutationProbability,
    const bool dayShift,
    bool child
) : rnd(rnd), genotype(numDepots, 0), numDepots(numDepots), numAmbulances(numAmbulances), numTimeSegments(numTimeSegments), fitness(0.0), mutationProbability(mutationProbability), dayShift(dayShift), child(child) {
    if (!child) {
        randomizeAmbulances();
    }
}

void IndividualTSGA::randomizeAmbulances() {
    // reset all depots to 0 ambulances
    std::fill(genotype.begin(), genotype.end(), 0);

    for (int i = 0; i < numAmbulances; i++) {
        int depotIndex = getRandomInt(rnd, 0, genotype.size() - 1);
        genotype[depotIndex]++;
    }
}

bool IndividualTSGA::isValid() const {
    int totalAmbulances = std::accumulate(genotype.begin(), genotype.end(), 0);

    return totalAmbulances == numAmbulances;
}

void IndividualTSGA::evaluateFitness(std::vector<Event> events, bool saveMetricsToFile) const {
    fitness = 0.0;

    std::vector<std::vector<int>> allocations;
    allocations.push_back(genotype);
    allocations.push_back(genotype);

    AmbulanceAllocator ambulanceAllocator;
    ambulanceAllocator.allocate(events, allocations, dayShift);

    Simulator simulator(
        rnd,
        ambulanceAllocator,
        Settings::get<DispatchEngineStrategyType>("DISPATCH_STRATEGY"),
        events
    );
    simulator.run(saveMetricsToFile);

    fitness = simulator.responseTimeViolations();

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

void IndividualTSGA::mutate() {
    std::uniform_real_distribution<> probDist(0.0, 1.0);
    std::uniform_int_distribution<> depotDist(0, genotype.size() - 1);

    for (int depot = 0; depot < genotype.size(); ++depot) {
        // only mutate if depot has at least one ambulance
        if (probDist(rnd) < mutationProbability && genotype[depot] > 0) {
            int otherDepot = depotDist(rnd);
            while (otherDepot == depot) {
                otherDepot = depotDist(rnd);
            }

            // transfer an ambulance from current depot to another depot
            genotype[depot]--;
            genotype[otherDepot]++;
        }
    }

    // ensure total number of ambulances remains constant
    if (!isValid()) {
        throw std::runtime_error("Total number of ambulances changed during mutation.");
    }
}

void IndividualTSGA::repair() {
    int totalAmbulances = std::accumulate(genotype.begin(), genotype.end(), 0);

    while (totalAmbulances != numAmbulances) {
        if (totalAmbulances < numAmbulances) {
            addAmbulances(numAmbulances - totalAmbulances);
        } else {
            removeAmbulances(totalAmbulances - numAmbulances);
        }

        totalAmbulances = std::accumulate(genotype.begin(), genotype.end(), 0);
    }

    // ensure total number of ambulances remains constant
    if (!isValid()) {
        throw std::runtime_error("Total number of ambulances changed during mutation.");
    }
}

void IndividualTSGA::addAmbulances(int ambulancesToAdd) {
    std::uniform_int_distribution<> dist(0, genotype.size() - 1);

    for (int i = 0; i < ambulancesToAdd; i++) {
        int depotIndex = dist(rnd);

        genotype[depotIndex]++;
    }
}

void IndividualTSGA::removeAmbulances(int ambulancesToRemove) {
    std::uniform_int_distribution<> dist(0, genotype.size() - 1);

    for (int i = 0; i < ambulancesToRemove; i++) {
        bool ambulanceRemoved = false;

        while (!ambulanceRemoved) {
            int depotIndex = dist(rnd);

            if (genotype[depotIndex] > 0) {
                genotype[depotIndex]--;
                ambulanceRemoved = true;
            }
        }
    }
}

void IndividualTSGA::printChromosome() const {
    std::vector<unsigned int> depotIndicies = Stations::getInstance().getDepotIndices(true);
    for (int i = 0; i < genotype.size(); i++) {
        std::cout << "Depot " << Stations::getInstance().get<std::string>("name", depotIndicies[i]) << ": " << genotype[i] << " ambulances" << std::endl;
    }
}

const std::vector<int>& IndividualTSGA::getGenotype() const {
    return genotype;
}

void IndividualTSGA::setGenotype(const std::vector<int>& newGenotype) {
    genotype = newGenotype;
}

double IndividualTSGA::getFitness() const {
    return fitness;
}

void IndividualTSGA::setFitness(double newFitness) {
    fitness = newFitness;
}

void IndividualTSGA::setAmbulancesAtDepot(int depotIndex, int count) {
    if (depotIndex >= 0 && depotIndex < genotype.size()) {
        genotype[depotIndex] = count;
    }
}

int IndividualTSGA::getAmbulancesAtDepot(int depotIndex) const {
    return (depotIndex >= 0 && depotIndex < genotype.size()) ? genotype[depotIndex] : -1;
}

int IndividualTSGA::getNumAmbulances() const {
    return numAmbulances;
}

void IndividualTSGA::setNumAmbulances(int newNumAmbulances) {
    numAmbulances = newNumAmbulances;
}

int IndividualTSGA::getNumDepots() const {
    return numDepots;
}

void IndividualTSGA::setNumDepots(int newNumDepots) {
    numDepots = newNumDepots;
}

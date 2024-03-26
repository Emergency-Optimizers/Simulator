/**
 * @file IndividualNSGA.cpp
 *
 * @copyright Copyright (c) 2024 Emergency-Optimizers
 */

/* internal libraries */
#include "heuristics/IndividualNSGA.hpp"
#include "Utils.hpp"
#include "simulator/AmbulanceAllocator.hpp"
#include "simulator/Simulator.hpp"
#include "file-reader/Stations.hpp"

IndividualNSGA::IndividualNSGA(
    std::mt19937& rnd,
    int numObjectives,
    int numDepots,
    int numAmbulances,
    double mutationProbability,
    const bool dayShift,
    bool child
) : rnd(rnd),
    genotype(numDepots, 0),
    numObjectives(numObjectives),
    numDepots(numDepots),
    numAmbulances(numAmbulances),
    objectives(numObjectives, 0),
    crowdingDistance(0),
    dominationCount(0),
    rank(-1),
    mutationProbability(mutationProbability),
    dayShift(dayShift),
    child(child) {
        if (!child) {
            randomizeAmbulances();
        }
    }

void IndividualNSGA::randomizeAmbulances() {
    // reset all depots to 0 ambulances
    std::fill(genotype.begin(), genotype.end(), 0);

    for (int i = 0; i < numAmbulances; i++) {
        int depotIndex = getRandomInt(rnd, 0, genotype.size() - 1);
        genotype[depotIndex]++;
    }
}

bool IndividualNSGA::isValid() const {
    return numAmbulances == std::accumulate(genotype.begin(), genotype.end(), 0);
}

void IndividualNSGA::evaluateObjectives(const std::vector<Event>& events, bool saveMetricsToFile) {
    AmbulanceAllocator ambulanceAllocator;
    ambulanceAllocator.allocate(events, genotype, dayShift);

    Simulator simulator(
        rnd,
        ambulanceAllocator,
        DispatchEngineStrategyType::CLOSEST,
        events
    );

    simulator.run(saveMetricsToFile);

    objectives[0] = simulator.averageResponseTime("A", true);
    objectives[1] = simulator.averageResponseTime("A", false);
    objectives[2] = simulator.averageResponseTime("H", true);
    objectives[3] = simulator.averageResponseTime("H", false);
    objectives[4] = simulator.averageResponseTime("V1", true);
    objectives[5] = simulator.averageResponseTime("V1", false);
    objectives[6] = simulator.responseTimeViolations();
}

double IndividualNSGA::calculateUniformityObjective() {
    // mock objective, can be removed
    double mean = std::accumulate(genotype.begin(), genotype.end(), 0.0) / genotype.size();
    double variance = std::accumulate(genotype.begin(), genotype.end(), 0.0, [mean](double acc, int val) {
        return acc + (val - mean) * (val - mean);
    }) / genotype.size();

    return variance;
}

double IndividualNSGA::calculateMinimizeMaxDepotObjective() {
    // mock objective, can be removed
    return *std::max_element(genotype.begin(), genotype.end());
}

bool IndividualNSGA::dominates(const IndividualNSGA& other) const {
    // an individual dominates another if it is better in at least one objective, and no worse in all other objectives
    bool better = false;
    for (int i = 0; i < objectives.size(); ++i) {
        if (objectives[i] < other.objectives[i]) {
            better = true;
        } else if (objectives[i] > other.objectives[i]) {
            return false;
        }
    }
    return better;
}

void IndividualNSGA::mutate() {
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

void IndividualNSGA::repair() {
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

void IndividualNSGA::addAmbulances(int ambulancesToAdd) {
    std::uniform_int_distribution<> dist(0, genotype.size() - 1);


    for (int i = 0; i < ambulancesToAdd; i++) {
        int depotIndex = dist(rnd);

        genotype[depotIndex]++;
    }
}

void IndividualNSGA::removeAmbulances(int ambulancesToRemove) {
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

void IndividualNSGA::printChromosome() const {
    for (int i = 0; i < genotype.size(); i++) {
        std::cout << "Depot " << i << ": " << genotype[i] << " ambulances" << std::endl;
    }
}

const std::vector<int>& IndividualNSGA::getGenotype() const {
    return genotype;
}

void IndividualNSGA::setGenotype(const std::vector<int>& newGenotype) {
    genotype = newGenotype;
}

void IndividualNSGA::setAmbulancesAtDepot(int depotIndex, int count) {
    if (depotIndex >= 0 && depotIndex < genotype.size()) {
        genotype[depotIndex] = count;
    }
}

int IndividualNSGA::getAmbulancesAtDepot(int depotIndex) const {
    return (depotIndex >= 0 && depotIndex < genotype.size()) ? genotype[depotIndex] : -1;
}

int IndividualNSGA::getNumAmbulances() const {
    return numAmbulances;
}

void IndividualNSGA::setNumAmbulances(int newNumAmbulances) {
    numAmbulances = newNumAmbulances;
}

int IndividualNSGA::getNumDepots() const {
    return numDepots;
}

void IndividualNSGA::setNumDepots(int newNumDepots) {
    numDepots = newNumDepots;
}

double IndividualNSGA::getCrowdingDistance() const {
    return crowdingDistance;
}

void IndividualNSGA::setCrowdingDistance(double newCrowdingDistance) {
    crowdingDistance = newCrowdingDistance;
}

int IndividualNSGA::getRank() const {
    return rank;
}

void IndividualNSGA::setRank(int newRank) {
    rank = newRank;
}

const std::vector<double>& IndividualNSGA::getObjectives() const {
    return objectives;
}

void IndividualNSGA::setObjectives(const std::vector<double>& newObjectives) {
    objectives = newObjectives;
}

int IndividualNSGA::getDominationCount() {
    return dominationCount;
}

void IndividualNSGA::incrementDominationCount() {
    dominationCount++;
}

void IndividualNSGA::decrementDominationCount() {
    dominationCount--;
}

void IndividualNSGA::clearDominationCount() {
    dominationCount = 0;
}

const std::vector<IndividualNSGA*>& IndividualNSGA::getDominatedIndividuals() const {
    return dominatedIndividuals;
}

void IndividualNSGA::nowDominates(IndividualNSGA* dominatedIndividual) {
    dominatedIndividuals.push_back(dominatedIndividual);
}

void IndividualNSGA::clearDominatedIndividuals() {
    dominatedIndividuals.clear();
}

/**
 * @file Individual.cpp
 *
 * @copyright Copyright (c) 2024 Emergency-Optimizers
 */

/* internal libraries */
#include "metaheuristics/nsga2/Individual.hpp"
#include "Utils.hpp"
#include "simulator/AmbulanceAllocator.hpp"
#include "simulator/Simulator.hpp"

Individual::Individual(
    std::mt19937& rnd,
    Incidents& incidents,
    Stations& stations,
    ODMatrix& odMatrix,
    std::vector<Event> events,
    int numObjectives,
    int numDepots,
    int numAmbulances,
    double mutationProbability,
    bool child
) : rnd(rnd),
    incidents(incidents),  
    stations(stations),
    odMatrix(odMatrix),
    genotype(numDepots, 0),
    numObjectives(numObjectives),
    numDepots(numDepots),
    numAmbulances(numAmbulances),
    objectives(numObjectives, 0),
    crowdingDistance(0),
    dominationCount(0),
    rank(-1),
    mutationProbability(mutationProbability),
    child(child) {
        if (!child) {
            randomizeAmbulances();
        }
    }

void Individual::randomizeAmbulances() {
    // reset all depots to 0 ambulances
    std::fill(genotype.begin(), genotype.end(), 0);

    for (int i = 0; i < numAmbulances; i++) {
        int depotIndex = Utils::getRandomInt(rnd, 0, genotype.size() - 1);
        genotype[depotIndex]++;
    }
}

bool Individual::isValid() const {
    return numAmbulances == std::accumulate(genotype.begin(), genotype.end(), 0);
}

void Individual::evaluateObjectives(const std::vector<Event>& events, bool saveMetricsToFile) {
    std::cout << "Here" << std::endl;
    AmbulanceAllocator ambulanceAllocator(stations);
    ambulanceAllocator.allocate(genotype);

    Simulator simulator(
        rnd,
        incidents,
        stations,
        odMatrix,
        ambulanceAllocator,
        DispatchEngineStrategyType::CLOSEST,
        events
    );
    simulator.run(saveMetricsToFile);

    // TODO:
    // objectives[0] = simulator.getAverageResponseTime("A", "Urban");
    // objectives[1] = simulator.getAverageResponseTime("H", "Urban");
    // objectives[2] = simulator.getAverageResponseTime("A", "NonUrban");
    // objectives[3] = simulator.getAverageResponseTime("H", "NonUrban");
    // objectives[4] = simulator.getCountOverThreshold("Urban", 12)
    // objectives[5] =  simulator.getCountOverThreshold("NonUrban", 25);
    // objectives[6] = simulator.calculateUHU();

    // mock objectives for purpose of testing:
    objectives[0] = simulator.averageResponseTime("A", true);
    objectives[1] = calculateUniformityObjective();
    objectives[2] = calculateMinimizeMaxDepotObjective();
}

double Individual::calculateUniformityObjective() {
    // mock objective, can be removed
    double mean = std::accumulate(genotype.begin(), genotype.end(), 0.0) / genotype.size();
    double variance = std::accumulate(genotype.begin(), genotype.end(), 0.0, [mean](double acc, int val) {
        return acc + (val - mean) * (val - mean);
    }) / genotype.size();

    return variance;
}

double Individual::calculateMinimizeMaxDepotObjective() {
    // mock objective, can be removed
    return *std::max_element(genotype.begin(), genotype.end());
}

bool Individual::dominates(const Individual& other) const {
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

void Individual::mutate() {
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

void Individual::repair() {
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

void Individual::addAmbulances(int ambulancesToAdd) {
    std::uniform_int_distribution<> dist(0, genotype.size() - 1);


    for (int i = 0; i < ambulancesToAdd; i++) {
        int depotIndex = dist(rnd);

        genotype[depotIndex]++;
    }
}

void Individual::removeAmbulances(int ambulancesToRemove) {
    std::uniform_int_distribution<> dist(0, genotype.size() - 1);
    std::cout << "In removeAmulances() - Min: " << 0 << ", Max: " << genotype.size() - 1 << std::endl;

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

void Individual::printChromosome() const {
    std::cout << "Okay gonna try: " << genotype.size() << std::endl;
    for (int i = 0; i < genotype.size(); i++) {
        std::cout << "Depot " << i << ": " << genotype[i] << " ambulances" << std::endl;
    }        
    std::cout << "Heelep" << std::endl;

}

const std::vector<int>& Individual::getGenotype() const {
    return genotype;
}

void Individual::setGenotype(const std::vector<int>& newGenotype) {
    genotype = newGenotype;
}

void Individual::setAmbulancesAtDepot(int depotIndex, int count) {
    if (depotIndex >= 0 && depotIndex < genotype.size()) {
        genotype[depotIndex] = count;
    }
}

int Individual::getAmbulancesAtDepot(int depotIndex) const {
    return (depotIndex >= 0 && depotIndex < genotype.size()) ? genotype[depotIndex] : -1;
}

int Individual::getNumAmbulances() const {
    return numAmbulances;
}

void Individual::setNumAmbulances(int newNumAmbulances) {
    numAmbulances = newNumAmbulances;
}

int Individual::getNumDepots() const {
    return numDepots;
}

void Individual::setNumDepots(int newNumDepots) {
    numDepots = newNumDepots;
}

double Individual::getCrowdingDistance() const {
    return crowdingDistance;
}

void Individual::setCrowdingDistance(double newCrowdingDistance) {
    crowdingDistance = newCrowdingDistance;
}

int Individual::getRank() const {
    return rank;
}

void Individual::setRank(int newRank) {
    rank = newRank;
}

const std::vector<double>& Individual::getObjectives() const {
    return objectives;
}

void Individual::setObjectives(const std::vector<double>& newObjectives) {
    objectives = newObjectives;
}

int Individual::getDominationCount() {
    return dominationCount;
}

void Individual::incrementDominationCount() {
    dominationCount++;
}

void Individual::decrementDominationCount() {
    dominationCount--;
}

void Individual::clearDominationCount() {
    dominationCount = 0;
}

const std::vector<Individual*>& Individual::getDominatedIndividuals() const {
    return dominatedIndividuals;
}

void Individual::nowDominates(Individual* dominatedIndividual) {
    dominatedIndividuals.push_back(dominatedIndividual);
} 

void Individual::clearDominatedIndividuals() {
    dominatedIndividuals.clear();
}
/**
 * @file GAIndividual.cpp
 *
 * @copyright Copyright (c) 2024 Emergency-Optimizers
 */

/* internal libraries */
#include "metaheuristics/genetic-algorithm/GAIndividual.hpp"
#include "Utils.hpp"
#include "simulator/AmbulanceAllocator.hpp"
#include "simulator/Simulator.hpp"

GAIndividual::GAIndividual(
    std::mt19937& rnd,
    Incidents& incidents,
    Stations& stations,
    ODMatrix& odMatrix,
    std::vector<Event> events,
    int numDepots,
    int numAmbulances,
    double mutationProbability,
    bool child
) : rnd(rnd), incidents(incidents), stations(stations), odMatrix(odMatrix), genotype(numDepots, 0), numDepots(numDepots), numAmbulances(numAmbulances), fitness(0.0), mutationProbability(mutationProbability), child(child) {
    if (!child) {
        randomizeAmbulances();
    }
}

void GAIndividual::randomizeAmbulances() {
    // reset all depots to 0 ambulances
    std::fill(genotype.begin(), genotype.end(), 0);

    for (int i = 0; i < numAmbulances; i++) {
        int depotIndex = Utils::getRandomInt(rnd, 0, genotype.size() - 1);
        genotype[depotIndex]++;
    }
}

bool GAIndividual::isValid() const {
    int totalAmbulances = std::accumulate(genotype.begin(), genotype.end(), 0);

    return totalAmbulances == numAmbulances;
}

void GAIndividual::evaluateFitness(std::vector<Event> events, bool saveMetricsToFile) const {
    fitness = 0.0;

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
                << (ambulanceAllocator.ambulances[i].timeUnavailable / 60) / 60 << " hours"
                << std::endl;
        }
        std::cout
            << "Total: " << totalHours << " hours, "
            << "Standard deviation: " << Utils::calculateStandardDeviation(times)
            << std::endl;
    }
}

void GAIndividual::mutate() {
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

void GAIndividual::repair() {
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

void GAIndividual::addAmbulances(int ambulancesToAdd) {
    std::uniform_int_distribution<> dist(0, genotype.size() - 1);

    for (int i = 0; i < ambulancesToAdd; i++) {
        int depotIndex = dist(rnd);

        genotype[depotIndex]++;
    }
}

void GAIndividual::removeAmbulances(int ambulancesToRemove) {
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

void GAIndividual::printChromosome() const {
    for (int i = 0; i < genotype.size(); i++) {
        std::cout << "Depot " << i << ": " << genotype[i] << " ambulances" << std::endl;
    }
}

const std::vector<int>& GAIndividual::getGenotype() const {
    return genotype;
}

void GAIndividual::setGenotype(const std::vector<int>& newGenotype) {
    genotype = newGenotype;
}

double GAIndividual::getFitness() const {
    return fitness;
}

void GAIndividual::setFitness(double newFitness) {
    fitness = newFitness;
}

void GAIndividual::setAmbulancesAtDepot(int depotIndex, int count) {
    if (depotIndex >= 0 && depotIndex < genotype.size()) {
        genotype[depotIndex] = count;
    }
}

int GAIndividual::getAmbulancesAtDepot(int depotIndex) const {
    return (depotIndex >= 0 && depotIndex < genotype.size()) ? genotype[depotIndex] : -1;
}

int GAIndividual::getNumAmbulances() const {
    return numAmbulances;
}

void GAIndividual::setNumAmbulances(int newNumAmbulances) {
    numAmbulances = newNumAmbulances;
}

int GAIndividual::getNumDepots() const {
    return numDepots;
}

void GAIndividual::setNumDepots(int newNumDepots) {
    numDepots = newNumDepots;
}

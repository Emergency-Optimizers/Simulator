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
#include "file-reader/Settings.hpp"
#include <iomanip>

IndividualNSGA::IndividualNSGA(
    std::mt19937& rnd,
    int numObjectives,
    int numDepots,
    int numAmbulances,
    int numTimeSegments,
    double mutationProbability,
    const bool dayShift,
    bool child
) : rnd(rnd),
    genotype(numTimeSegments, std::vector<int>(numDepots, 0)),
    numObjectives(numObjectives),
    numDepots(numDepots),
    numAmbulances(numAmbulances),
    numTimeSegments(numTimeSegments),
    objectives(numObjectives, 0),
    fitness(0.0),
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
    for (auto& segment : genotype) {
        std::fill(segment.begin(), segment.end(), 0);
    }
    for (int t = 0; t < numTimeSegments; ++t) {
        for (int i = 0; i < numAmbulances; ++i) {
            int depotIndex = getRandomInt(rnd, 0, numDepots - 1);
            genotype[t][depotIndex]++;
        }
    }
}

bool IndividualNSGA::isValid() const {
    for (const auto& segment : genotype) {
        int totalAmbulances = std::accumulate(segment.begin(), segment.end(), 0);
        if (totalAmbulances != numAmbulances) return false;
    }
    return true;
}

void IndividualNSGA::evaluateObjectives(std::vector<Event> events, std::vector<float> objectiveWeights, bool saveMetricsToFile) {

    AmbulanceAllocator ambulanceAllocator;
    ambulanceAllocator.allocate(events, genotype, dayShift);

    Simulator simulator(
        rnd,
        ambulanceAllocator,
        Settings::get<DispatchEngineStrategyType>("DISPATCH_STRATEGY"),
        events
    );

    simulator.run(saveMetricsToFile);

    objectives[0] = objectiveWeights[0] * simulator.averageResponseTime("A", true);
    objectives[1] = objectiveWeights[1] * simulator.averageResponseTime("A", false);
    objectives[2] = objectiveWeights[2] * simulator.averageResponseTime("H", true);
    objectives[3] = objectiveWeights[3] * simulator.averageResponseTime("H", false);
    objectives[4] = objectiveWeights[4] * simulator.averageResponseTime("V1", true);
    objectives[5] = objectiveWeights[5] * simulator.averageResponseTime("V1", false);
    objectives[6] = objectiveWeights[6] * simulator.responseTimeViolations();

    fitness = 0.0f;
    for (size_t i = 0; i < objectives.size(); ++i) {
        fitness += objectives[i];
    }
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

void IndividualNSGA::repair() {
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

void IndividualNSGA::printChromosome() const {
    std::vector<unsigned int> depotIndices = Stations::getInstance().getDepotIndices(dayShift);
    for (int t = 0; t < genotype.size(); ++t) {
        std::cout << "Time Segment " << (t + 1) << ":\n";
        for (int d = 0; d < genotype[t].size(); ++d) {
            std::cout << "  Depot " << Stations::getInstance().get<std::string>("name", depotIndices[d])
                      << ": " << genotype[t][d] << " ambulances\n";
        }
    }
}

void IndividualNSGA::printTimeSegmentedChromosome() const {
    std::cout << "\n";
    std::vector<std::string> depotNames = {"Eidsvoll", "Ullensaker", "Nes", "Aurskog-Holand", "Nittedal", "Lorenskog", "Asker", "Barum", "Smestad", "Ulleval", "Brobekk", "Sentrum", "Prinsdal", "Nordre Follo", "Sondre Follo", "Bekkestua", "Grorud", "Skedsmokorset", "Ryen"};

    // TODO: Placeholder
    auto calculateFitnessForSegment = [&](int segmentIndex) -> double {
        return 33.0;
    };

    // Print the header
    std::cout << std::left << std::setw(20) << "Time Segment" << "|";
    for (int t = 0; t < numTimeSegments; ++t) {
        std::cout << " T" << t + 1;
        if (t + 1 < 10) { // One-digit time segment number
            std::cout << "  "; // Three spaces
        } else { // Two-digit time segment number
            std::cout << " "; // Two spaces
        }
    }
    std::cout << "   Fitness\n";
    std::cout << std::string(100, '-') << "\n";

    // iterate over depots and print each row
    for (size_t d = 0; d < depotNames.size(); ++d) {
        std::cout << std::right << std::setw(19) << depotNames[d] << " |";
        for (size_t t = 0; t < numTimeSegments; ++t) {
            std::cout << std::right << std::setw(2) << genotype[t][d] << "   ";
        }
        std::cout << "|" << std::setw(4) << calculateFitnessForSegment(d) << "\n";
    }
}

const std::vector<std::vector<int>>& IndividualNSGA::getGenotype() const {
    return genotype;
}

void IndividualNSGA::setGenotype(const std::vector<std::vector<int>>& newGenotype) {
    genotype = newGenotype;
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

double IndividualNSGA::getFitness() const {
    return fitness;
}

void IndividualNSGA::setFitness(double newFitness) {
    fitness = newFitness;
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

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

IndividualGA::IndividualGA(
    std::mt19937& rnd,
    int numDepots,
    int numAmbulances,
    int numTimeSegments,
    double mutationProbability,
    const bool dayShift,
    bool child
) : rnd(rnd),
    genotype(numTimeSegments,
    std::vector<int>(numDepots, 0)),
    numDepots(numDepots),
    numAmbulances(numAmbulances),
    numTimeSegments(numTimeSegments),
    fitness(0.0),
    mutationProbability(mutationProbability),
    dayShift(dayShift),
    child(child) {
    if (!child) {
        randomizeAmbulances();
    }
}

void IndividualGA::randomizeAmbulances() {
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

void IndividualGA::printChromosome() const {
    std::vector<unsigned int> depotIndices = Stations::getInstance().getDepotIndices(dayShift);
    for (int t = 0; t < genotype.size(); ++t) {
        std::cout << "Time Segment " << (t + 1) << ":\n";
        for (int d = 0; d < genotype[t].size(); ++d) {
            std::cout << "  Depot " << Stations::getInstance().get<std::string>("name", depotIndices[d])
                      << ": " << genotype[t][d] << " ambulances\n";
        }
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

void IndividualGA::setFitness(double newFitness) {
    fitness = newFitness;
}

int IndividualGA::getNumAmbulances() const {
    return numAmbulances;
}

void IndividualGA::setNumAmbulances(int newNumAmbulances) {
    numAmbulances = newNumAmbulances;
}

int IndividualGA::getNumDepots() const {
    return numDepots;
}

void IndividualGA::setNumDepots(int newNumDepots) {
    numDepots = newNumDepots;
}

void IndividualGA::printTimeSegmentedChromosome() const {
    std::vector<std::string> depotNames = {"Eidsvoll", "Ullensaker", "Nes", "Aurskog-Holand", "Nittedal", "Lorenskog", "Asker", "Barum", "Smestad", "Ulleval", "Brobekk", "Sentrum", "Prinsdal", "Nordre Follo", "Sondre Follo", "Bekkestua", "Grorud", "Skedsmokorset", "Ryen"};

    // TODO: Placeholder
    auto calculateFitnessForSegment = [&](int segmentIndex) -> double {
        return 33.0;
    };

    // Print the header
    std::cout << std::left << std::setw(20) << "Time Segment" << "|";
    for (int t = 0; t < numTimeSegments; ++t) {
        std::cout << " T" << t + 1;
        if (t + 1 < 10) {  // One-digit time segment number
            std::cout << " ";  // Three spaces
        }
    }

    int headerStart = 24;
    int timeSegmentWidth = 4;
    int headerEnd = 9;
    int totalWidth = headerStart + (numTimeSegments * timeSegmentWidth) - 1 + headerEnd;

    std::cout << " | Fitness\n";
    std::cout << std::string(totalWidth, '-') << "\n";

    // iterate over depots and print each row
    for (size_t d = 0; d < depotNames.size(); ++d) {
        std::cout << std::right << std::setw(19) << depotNames[d] << " | ";
        for (size_t t = 0; t < numTimeSegments; ++t) {
            std::cout << std::right << std::setw(1) << genotype[t][d] << "   ";
        }
        std::cout << "|" << std::setw(5) << calculateFitnessForSegment(d) << "\n";
    }
}

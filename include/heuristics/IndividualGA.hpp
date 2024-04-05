/**
 * @file IndividualGA.hpp
 *
 * @copyright Copyright (c) 2024 Emergency-Optimizers
 */

#pragma once

/* external libraries */
#include <vector>
#include <iostream>
#include <numeric>
#include <random>
/* internal libraries */
#include "file-reader/Incidents.hpp"
#include "simulator/Event.hpp"

class IndividualGA {
 private:
    std::mt19937& rnd;
    std::vector<std::vector<int>> genotype;
    std::vector<Event> simulatedEvents;
    int numDepots;
    int numAmbulances;
    int numTimeSegments;
    mutable double fitness = 0;
    double mutationProbability;
    const bool dayShift;
    bool child;

 public:
    IndividualGA(
        std::mt19937& rnd,
        int numDepots,
        int numAmbulances,
        int numTimeSegments,
        double mutationProbability,
        const bool dayShift,
        bool child = true
    );
    void randomizeAmbulances();
    bool isValid() const;
    void evaluateFitness(std::vector<Event> events, bool saveMetricsToFile = false);
    void mutate();
    void repair();
    void printChromosome() const;
    const std::vector<std::vector<int>>& getGenotype() const;
    void setGenotype(const std::vector<std::vector<int>> newGenotype);
    double getFitness() const;
    void setFitness(double fitness);
    void setAmbulancesAtDepot(int depotIndex, int count);
    int getAmbulancesAtDepot(int depotIndex) const;
    int getNumAmbulances() const;
    void setNumAmbulances(int newNumAmbulances);
    int getNumDepots() const;
    void setNumDepots(int newNumDepots);
    void printTimeSegmentedChromosome() const;
    IndividualGA& IndividualGA::operator=(const IndividualGA& other) {
        if (this != &other) {
            genotype = other.genotype;
            numDepots = other.numDepots;
            numAmbulances = other.numAmbulances;
            fitness = other.fitness;
            mutationProbability = other.mutationProbability;
            child = other.child;
        }
        return *this;
    }
};

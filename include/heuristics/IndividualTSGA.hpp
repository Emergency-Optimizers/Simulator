/**
 * @file IndividualTSGA.hpp
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

class IndividualTSGA {
 private:
    std::mt19937& rnd;
    std::vector<int> genotype;
    int numDepots;
    int numAmbulances;
    int numTimeSegments;
    mutable double fitness = 0;
    double mutationProbability;
    const bool dayShift;
    bool child;

 public:
    IndividualTSGA(
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
    void evaluateFitness(std::vector<Event> events, bool saveMetricsToFile = false) const;
    void mutate();
    void repair();
    void addAmbulances(int ambulancesToAdd = 1);
    void removeAmbulances(int ambulancesToRemove = 1);
    void printChromosome() const;
    const std::vector<int>& getGenotype() const;
    void setGenotype(const std::vector<int>& newGenotype);
    double getFitness() const;
    void setFitness(double fitness);
    void setAmbulancesAtDepot(int depotIndex, int count);
    int getAmbulancesAtDepot(int depotIndex) const;
    int getNumAmbulances() const;
    void setNumAmbulances(int newNumAmbulances);
    int getNumDepots() const;
    void setNumDepots(int newNumDepots);
    IndividualTSGA& IndividualTSGA::operator=(const IndividualTSGA& other) {
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

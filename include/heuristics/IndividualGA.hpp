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
    mutable std::vector<std::vector<int>> genotype;
    mutable std::vector<Event> simulatedEvents;
    mutable double fitness = 0.0;
    int numDepots;
    int numAmbulances;
    int numTimeSegments;
    double mutationProbability;
    bool dayShift;
    bool child;

    void generateGenotype();
    void emptyGenotype();
    void randomGenotype();
    void evenGenotype();

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
    bool isValid() const;
    void evaluateFitness(std::vector<Event> events, bool saveMetricsToFile = false);
    void mutate();
    void repair();
    void printGenotype() const;
    const std::vector<std::vector<int>>& getGenotype() const;
    void setGenotype(const std::vector<std::vector<int>> newGenotype);
    double getFitness() const;
    IndividualGA& IndividualGA::operator=(const IndividualGA& other) {
        if (this != &other) {
            rnd = other.rnd;
            genotype = other.genotype;
            simulatedEvents = other.simulatedEvents;
            fitness = other.fitness;
            numDepots = other.numDepots;
            numAmbulances = other.numAmbulances;
            numTimeSegments = other.numTimeSegments;
            mutationProbability = other.mutationProbability;
            dayShift = other.dayShift;
            child = other.child;
        }
        return *this;
    }
};

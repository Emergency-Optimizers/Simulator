/**
 * @file IndividualGA.hpp
 *
 * @copyright Copyright (c) 2024 Emergency-Optimizers
 */

#pragma once

/* external libraries */
#include <vector>
#include <numeric>
#include <random>
/* internal libraries */
#include "simulator/Event.hpp"
#include "simulator/Ambulance.hpp"
#include "simulator/strategies/DispatchEngineStrategyType.hpp"
#include "heuristics/GenotypeInitType.hpp"

class IndividualGA {
 private:
    std::mt19937& rnd;
    int numDepots;
    int numAmbulances;
    int numTimeSegments;
    double mutationProbability;
    bool child;

    void generateGenotype(
        const std::vector<GenotypeInitType>& initTypes,
        const std::vector<double>& initTypeWeights
    );
    void emptyGenotype();
    void randomGenotype();
    void evenGenotype();
    void redistributeMutation();

 protected:
    virtual void updateMetrics();

 public:
    double fitness = 0.0;
    std::vector<std::vector<int>> genotype;
    std::vector<Event> simulatedEvents;
    std::vector<Ambulance> simulatedAmbulances;

    IndividualGA(
        std::mt19937& rnd,
        int numDepots,
        int numAmbulances,
        int numTimeSegments,
        double mutationProbability,
        bool child,
        const std::vector<GenotypeInitType>& genotypeInitTypes,
        const std::vector<double>& genotypeInitTypeWeights
    );
    void evaluate(std::vector<Event> events, const bool dayShift, const DispatchEngineStrategyType dispatchStrategy);
    void mutate();
    void repair();
    bool isValid() const;
    void printGenotype() const;

    IndividualGA& IndividualGA::operator=(const IndividualGA& other) {
        if (this != &other) {
            rnd = other.rnd;
            numDepots = other.numDepots;
            numAmbulances = other.numAmbulances;
            numTimeSegments = other.numTimeSegments;
            mutationProbability = other.mutationProbability;
            child = other.child;
            fitness = other.fitness;
            genotype = other.genotype;
            simulatedEvents = other.simulatedEvents;
            simulatedAmbulances = other.simulatedAmbulances;
        }

        return *this;
    }
};

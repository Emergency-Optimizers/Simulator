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
#include "heuristics/MutationType.hpp"

class IndividualGA {
 private:
    std::mt19937& rnd;
    double mutationProbability;
    int numAmbulances;
    int numAllocations;
    int numDepots;
    bool metricsChecked = false;

    void generateGenotype(
        const bool isChild,
        const std::vector<GenotypeInitType>& inits,
        const std::vector<double>& tickets
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
        const double mutationProbability,
        const int numAmbulances,
        const int numAllocations,
        const int numDepots,
        const bool isChild,
        const std::vector<GenotypeInitType>& genotypeInits,
        const std::vector<double>& genotypeInitsTickets
    );
    void evaluate(std::vector<Event> events, const bool dayShift, const DispatchEngineStrategyType dispatchStrategy);
    void mutate(
        const std::vector<MutationType>& mutations,
        const std::vector<double>& tickets
    );
    void repair();
    bool isValid() const;
    void printGenotype() const;

    IndividualGA& IndividualGA::operator=(const IndividualGA& other) {
        if (this != &other) {
            rnd = other.rnd;
            mutationProbability = other.mutationProbability;
            numAmbulances = other.numAmbulances;
            numAllocations = other.numAllocations;
            numDepots = other.numDepots;
            fitness = other.fitness;
            genotype = other.genotype;
            simulatedEvents = other.simulatedEvents;
            simulatedAmbulances = other.simulatedAmbulances;
        }

        return *this;
    }
};

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
    int numAmbulances;
    int numAllocations;
    int numDepots;
    bool metricsChecked = false;
    double objectiveAvgResponseTimeUrbanA = 0.0;
    double objectiveAvgResponseTimeUrbanH = 0.0;
    double objectiveAvgResponseTimeUrbanV1 = 0.0;
    double objectiveAvgResponseTimeRuralA = 0.0;
    double objectiveAvgResponseTimeRuralH = 0.0;
    double objectiveAvgResponseTimeRuralV1 = 0.0;
    double objectiveNumViolations = 0.0;

    void generateGenotype(
        const bool isChild,
        const std::vector<GenotypeInitType>& inits,
        const std::vector<double>& tickets
    );
    void emptyGenotype();
    void randomGenotype();
    void evenGenotype();
    void redistributeMutation(const double mutationProbability);
    void scrambleMutation(const double mutationProbability);

 protected:
    virtual void updateMetrics();

 public:
    double fitness = 0.0;
    std::vector<std::vector<int>> genotype;
    std::vector<Event> simulatedEvents;
    std::vector<Ambulance> simulatedAmbulances;

    IndividualGA(
        std::mt19937& rnd,
        const int numAmbulances,
        const int numAllocations,
        const int numDepots,
        const bool isChild,
        const std::vector<GenotypeInitType>& genotypeInits,
        const std::vector<double>& genotypeInitsTickets
    );
    void evaluate(std::vector<Event> events, const bool dayShift, const DispatchEngineStrategyType dispatchStrategy);
    void mutate(
        const double mutationProbability,
        const std::vector<MutationType>& mutations,
        const std::vector<double>& tickets
    );
    void repair();
    bool isValid() const;
    void printGenotype() const;

    IndividualGA& IndividualGA::operator=(const IndividualGA& other) {
        if (this != &other) {
            rnd = other.rnd;
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

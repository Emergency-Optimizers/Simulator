/**
 * @file Individual.hpp
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
#include "file-reader/Settings.hpp"
#include "heuristics/ObjectiveTypes.hpp"

class Individual {
 private:
    std::mt19937& rnd;
    int numAmbulances;
    int numAllocations;
    int numDepots;

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
    void Individual::neighborDuplicationMutation(const double mutationProbability);
    void updateMetrics();

 public:
    std::vector<std::vector<int>> genotype;
    std::vector<Event> simulatedEvents;
    std::vector<Ambulance> simulatedAmbulances;
    bool metricsChecked = false;

    double weightAvgResponseTimeUrbanA = Settings::get<double>("OBJECTIVE_WEIGHT_AVG_RESPONSE_TIME_URBAN_A");
    double weightAvgResponseTimeUrbanH = Settings::get<double>("OBJECTIVE_WEIGHT_AVG_RESPONSE_TIME_URBAN_H");
    double weightAvgResponseTimeUrbanV1 = Settings::get<double>("OBJECTIVE_WEIGHT_AVG_RESPONSE_TIME_URBAN_V1");
    double weightAvgResponseTimeRuralA = Settings::get<double>("OBJECTIVE_WEIGHT_AVG_RESPONSE_TIME_RURAL_A");
    double weightAvgResponseTimeRuralH = Settings::get<double>("OBJECTIVE_WEIGHT_AVG_RESPONSE_TIME_RURAL_H");
    double weightAvgResponseTimeRuralV1 = Settings::get<double>("OBJECTIVE_WEIGHT_AVG_RESPONSE_TIME_RURAL_V1");
    double weightPercentageViolations = Settings::get<double>("OBJECTIVE_WEIGHT_PERCENTAGE_VIOLATIONS");

    double fitness = 0.0;
    double objectiveAvgResponseTimeUrbanA = 0.0;
    double objectiveAvgResponseTimeUrbanH = 0.0;
    double objectiveAvgResponseTimeUrbanV1 = 0.0;
    double objectiveAvgResponseTimeRuralA = 0.0;
    double objectiveAvgResponseTimeRuralH = 0.0;
    double objectiveAvgResponseTimeRuralV1 = 0.0;
    double objectivePercentageViolations = 0.0;

    std::vector<double> allocationsFitness;
    std::vector<double> allocationsObjectiveAvgResponseTimeUrbanA;
    std::vector<double> allocationsObjectiveAvgResponseTimeUrbanH;
    std::vector<double> allocationsObjectiveAvgResponseTimeUrbanV1;
    std::vector<double> allocationsObjectiveAvgResponseTimeRuralA;
    std::vector<double> allocationsObjectiveAvgResponseTimeRuralH;
    std::vector<double> allocationsObjectiveAvgResponseTimeRuralV1;
    std::vector<double> allocationsObjectivePercentageViolations;

    std::vector<ObjectiveTypes> objectiveTypes = Settings::get<std::vector<ObjectiveTypes>>("OBJECTIVES");
    std::vector<double> objectives = std::vector<double>(objectiveTypes.size(), 0.0);
    std::vector<Individual*> dominatedIndividuals;
    int frontNumber = 0;
    double crowdingDistance = 0.0;

    Individual(
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
    bool dominates(const Individual& other) const;

    Individual& Individual::operator=(const Individual& other) {
        if (this != &other) {
            rnd = other.rnd;
            numAmbulances = other.numAmbulances;
            numAllocations = other.numAllocations;
            numDepots = other.numDepots;
            metricsChecked = other.metricsChecked;
            genotype = other.genotype;
            simulatedEvents = other.simulatedEvents;
            simulatedAmbulances = other.simulatedAmbulances;
            weightAvgResponseTimeUrbanA = other.weightAvgResponseTimeUrbanA;
            weightAvgResponseTimeUrbanH = other.weightAvgResponseTimeUrbanH;
            weightAvgResponseTimeUrbanV1 = other.weightAvgResponseTimeUrbanV1;
            weightAvgResponseTimeRuralA = other.weightAvgResponseTimeRuralA;
            weightAvgResponseTimeRuralH = other.weightAvgResponseTimeRuralH;
            weightAvgResponseTimeRuralV1 = other.weightAvgResponseTimeRuralV1;
            weightPercentageViolations = other.weightPercentageViolations;
            fitness = other.fitness;
            objectiveAvgResponseTimeUrbanA = other.objectiveAvgResponseTimeUrbanA;
            objectiveAvgResponseTimeUrbanH = other.objectiveAvgResponseTimeUrbanH;
            objectiveAvgResponseTimeUrbanV1 = other.objectiveAvgResponseTimeUrbanV1;
            objectiveAvgResponseTimeRuralA = other.objectiveAvgResponseTimeRuralA;
            objectiveAvgResponseTimeRuralH = other.objectiveAvgResponseTimeRuralH;
            objectiveAvgResponseTimeRuralV1 = other.objectiveAvgResponseTimeRuralV1;
            objectivePercentageViolations = other.objectivePercentageViolations;
            allocationsFitness = other.allocationsFitness;
            allocationsObjectiveAvgResponseTimeUrbanA = other.allocationsObjectiveAvgResponseTimeUrbanA;
            allocationsObjectiveAvgResponseTimeUrbanH = other.allocationsObjectiveAvgResponseTimeUrbanH;
            allocationsObjectiveAvgResponseTimeUrbanV1 = other.allocationsObjectiveAvgResponseTimeUrbanV1;
            allocationsObjectiveAvgResponseTimeRuralA = other.allocationsObjectiveAvgResponseTimeRuralA;
            allocationsObjectiveAvgResponseTimeRuralH = other.allocationsObjectiveAvgResponseTimeRuralH;
            allocationsObjectiveAvgResponseTimeRuralV1 = other.allocationsObjectiveAvgResponseTimeRuralV1;
            allocationsObjectivePercentageViolations = other.allocationsObjectivePercentageViolations;
            objectiveTypes = other.objectiveTypes;
            objectives = other.objectives;
            dominatedIndividuals = other.dominatedIndividuals;
            frontNumber = other.frontNumber;
            crowdingDistance = other.crowdingDistance;
        }

        return *this;
    }
};
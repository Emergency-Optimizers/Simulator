/**
 * @file IndividualNSGA.hpp
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
#include "simulator/Event.hpp"

class IndividualNSGA {
 private:
    std::mt19937& rnd;
    std::vector<std::vector<int>> genotype;
    int numObjectives;
    int numDepots;
    int numAmbulances;
    int numTimeSegments;
    const bool dayShift;
    std::vector<double> objectives;
    float fitness;
    double crowdingDistance;
    int dominationCount;  // how many individuals dominate this one
    std::vector<IndividualNSGA*> dominatedIndividuals;
    int rank;
    double mutationProbability;
    bool child;

 public:
    IndividualNSGA(
        std::mt19937& rnd,
        int numObjectives,
        int numDepots,
        int numAmbulances,
        int numTimeSegments,
        double mutationProbability,
        const bool dayShift,
        bool child = true
    );
    void randomizeAmbulances();
    bool isValid() const;
    void evaluateObjectives(std::vector<Event> events, const std::vector<float> objectiveWeights, bool saveMetricsToFile = false);
    void evaluateFitness();
    double calculateMinimizeMaxDepotObjective();
    double calculateUniformityObjective();
    bool dominates(const IndividualNSGA& other) const;
    void mutate();
    void repair();
    void addAmbulances(int ambulancesToAdd = 1);
    void removeAmbulances(int ambulancesToRemove = 1);
    void printChromosome() const;
    void printTimeSegmentedChromosome() const;
    const std::vector<std::vector<int>>& getGenotype() const;
    void setGenotype(const std::vector<std::vector<int>>& newGenotype);
    void setAmbulancesAtDepot(int depotIndex, int count);
    int getAmbulancesAtDepot(int depotIndex) const;
    int getNumAmbulances() const;
    void setNumAmbulances(int newNumAmbulances);
    int getNumDepots() const;
    void setNumDepots(int newNumDepots);
    double getCrowdingDistance() const;
    void setCrowdingDistance(double newCrowdingDistance);
    int getRank() const;
    void setRank(int newRank);
    const std::vector<double>& getObjectives() const;
    void setObjectives(const std::vector<double>& newObjectives);
    double getFitness() const;
    void setFitness(double newFitness);
    int getDominationCount();  // how many individuals
    void incrementDominationCount();
    void decrementDominationCount();
    void clearDominationCount();
    const std::vector<IndividualNSGA*>& getDominatedIndividuals() const;
    void nowDominates(IndividualNSGA* dominatedIndividual);
    void clearDominatedIndividuals();
    IndividualNSGA& IndividualNSGA::operator=(const IndividualNSGA& other) {
        if (this != &other) {
            genotype = other.genotype;
            numDepots = other.numDepots;
            numAmbulances = other.numAmbulances;
            objectives = other.objectives;
            crowdingDistance = other.crowdingDistance;
            mutationProbability = other.mutationProbability;
            child = other.child;
        }
        return *this;
    }
};

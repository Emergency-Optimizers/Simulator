/**
 * @file Individual.hpp
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
#include "simulator/Incidents.hpp"
#include "simulator/Stations.hpp"
#include "simulator/ODMatrix.hpp"
#include "simulator/Event.hpp"

class Individual {
 private:
    std::mt19937& rnd;
    Incidents& incidents;
    Stations& stations;
    ODMatrix& odMatrix;
    std::vector<int> genotype;
    int numDepots;
    int numAmbulances;
    std::vector<double> objectives;
    double crowdingDistance;
    double mutationProbability;
    bool child;

 public:
    Individual(
        std::mt19937& rnd,
        Incidents& incidents,
        Stations& stations,
        ODMatrix& odMatrix,
        std::vector<Event> events,
        int numDepots,
        int numAmbulances,
        double mutationProbability,
        bool child = true
    );
    void randomizeAmbulances();
    bool isValid() const;
    void evaluateObjectives(const std::vector<Event>& events, bool saveMetricsToFile = false) const;
    bool dominates(const Individual& other) const;
    void calculateCrowdingDistance(const std::vector<Individual>& population);
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
    double getCrowdingDistance() const;
    const std::vector<double>& getObjectives() const;
    void setObjectives(const std::vector<double>& newObjectives);
    Individual& Individual::operator=(const Individual& other) {
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

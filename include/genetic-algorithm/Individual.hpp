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

class Individual {
 private:
    std::mt19937 rnd;
    std::vector<int> genotype;
    int numDepots;
    int numAmbulances;
    mutable double fitness;
    double mutationProbability;
    bool child;

 public:
    Individual() = default;
    Individual(std::mt19937 rnd, int numDepots, int numAmbulances, double mutationProbability, bool child = true);

    void randomizeAmbulances();
    bool isValid() const;
    void evaluateFitness() const;
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
};

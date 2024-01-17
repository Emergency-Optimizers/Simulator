// Individual.hpp

#ifndef INDIVIDUAL_HPP
#define INDIVIDUAL_HPP

#include <vector>
#include <iostream>
#include <numeric>
#include <random>

class Individual {
private:
    std::vector<int> genotype;
    int numDepots;
    int numAmbulances;
    mutable double fitness;
    double mutationProbability;

public:
    Individual();
    Individual(int numDepots, int numAmbulances, double mutationProbability);

    void randomizeAmbulances();
    bool isValid() const;
    void evaluateFitness() const;
    void mutate();
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

#endif // INDIVIDUAL_HPP

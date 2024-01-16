// Individual.hpp

#ifndef INDIVIDUAL_HPP
#define INDIVIDUAL_HPP

#include <vector>
#include <iostream>
#include <numeric>

class Individual {
private:
    std::vector<int> genotype;
    int numDepots;
    int numAmbulances;
    mutable double fitness;

public:
    Individual(int numDepots, int numAmbulances);

    void randomizeAmbulances();

    bool isValid() const;

    void evaluateFitness() const;

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

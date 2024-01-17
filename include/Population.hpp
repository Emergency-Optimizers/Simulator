// Population.hpp

#ifndef POPULATION_HPP
#define POPULATION_HPP

#include <vector>
#include <iostream>
#include <numeric>
#include <random>

#include "Individual.hpp"

class Population {
private:
    std::vector<Individual> individuals;
    int populationSize;
    int numDepots;
    int numAmbulances;
public:
    Population::Population(int populationSize, int numDepots, int numAmbulances);
    void evaluateFitness();
    std::vector<Individual> parentSelection(int numParents, int tournamentSize);
    std::vector<Individual> survivorSelection(int numSurvivors);
    void addChildren(const std::vector<Individual>& children);
    std::vector<Individual> getIndividuals() const;

};

#endif // POPULATION_HPP

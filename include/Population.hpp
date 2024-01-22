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
    double mutationProbability;

public:
    Population::Population(int populationSize, int numDepots, int numAmbulances, double mutationProbability);
    void evaluateFitness();
    std::vector<Individual> parentSelection(int numParents, int tournamentSize);
    std::vector<Individual> survivorSelection(int numSurvivors);
    void addChildren(const std::vector<Individual>& children);
    Individual crossover(const Individual& parent1, const Individual& parent2);
    void evolve(int generations);
    Individual findFittest() const;
    Individual findLeastFit() const;
    double averageFitness() const;
    std::vector<Individual> getIndividuals() const;

};

#endif // POPULATION_HPP

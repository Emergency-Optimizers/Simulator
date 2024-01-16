// Population.hpp

#ifndef POPULATION_HPP
#define POPULATION_HPP

#include <vector>
#include <iostream>
#include <numeric>

#include "Individual.hpp"

class Population {
private:
    std::vector<Individual> individuals;
    int populationSize;
    int numDepots;
    int numAmbulances;
public:
    Population::Population(int populationSize, int numDepots, int numAmbulances);
};

#endif // POPULATION_HPP

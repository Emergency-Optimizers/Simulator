/**
 * @file Population.cpp
 *
 * @copyright Copyright (c) 2024 Emergency-Optimizers
 */

/* internal libraries */
#include "metaheuristics/nsga2/Population.hpp"
#include "Utils.hpp"
#include "simulator/MonteCarloSimulator.hpp"

Population::Population(
    std::mt19937& rnd,
    Incidents& incidents,
    Stations& stations,
    ODMatrix& odMatrix,
    int populationSize,
    int numDepots,
    int numAmbulances,
    int numObjectives,
    double mutationProbability,
    bool saveEventsToCSV
) : rnd(rnd),
    incidents(incidents),
    stations(stations),
    odMatrix(odMatrix),
    populationSize(populationSize),
    numDepots(numDepots),
    numAmbulances(numAmbulances),
    numObjectives(numObjectives),
    mutationProbability(mutationProbability) {

    MonteCarloSimulator monteCarloSim(rnd, incidents, 2019, 2, 7, true, 4);
    events = monteCarloSim.generateEvents(saveEventsToCSV);

    for (int i = 0; i < populationSize; i++) {
        Individual individual = Individual(rnd, incidents, stations, odMatrix, events, numObjectives, numDepots, numAmbulances, 0, mutationProbability, false);
        individuals.push_back(individual);
    }
    evaluateFitness();
}

void Population::evaluateFitness() {
    for (Individual& individual : individuals) {
        individual.evaluateObjectives(events);
    }
}

std::vector<Individual> Population::parentSelection(int numParents, int tournamentSize) {
    std::vector<Individual> selectedParents;

    for (int i = 0; i < numParents; i++) {
        std::vector<Individual> tournament;
        for (int j = 0; j < tournamentSize; j++) {
            tournament.push_back(Utils::getRandomElement(rnd, individuals));
        }

        // select the individual with the highest fitness in the tournament
        auto best = std::max_element(
            tournament.begin(),
            tournament.end(),
            [](const Individual &a, const Individual &b) {
                return a.getFitness() > b.getFitness();
            }
        );

        selectedParents.push_back(*best);
    }

    return selectedParents;
}

std::vector<Individual> Population::survivorSelection(int numSurvivors) {
    // sort the current population based on fitness in descending order
    std::sort(
        individuals.begin(),
        individuals.end(),
        [](const Individual &a, const Individual &b) { return a.getFitness() < b.getFitness(); }
    );

    std::vector<Individual> survivors;

    // calculate the actual number of survivors to keep, which is the minimum
    // of numSurvivors and the current population size to avoid out-of-bounds access
    int actualNumSurvivors = std::min(numSurvivors, static_cast<int>(individuals.size()));

    // copy the top 'actualNumSurvivors' individuals to the survivors vector
    for (int i = 0; i < actualNumSurvivors; i++) {
        survivors.push_back(individuals[i]);
    }

    return survivors;
}

void Population::addChildren(const std::vector<Individual>& children) {
    for (int i = 0; i < children.size(); i++) individuals.push_back(children[i]);
}

Individual Population::crossover(const Individual& parent1, const Individual& parent2) {
    std::vector<int> offspringGenotype;
    offspringGenotype.reserve(parent1.getGenotype().size());

    std::uniform_real_distribution<> dist(0, 1);

    for (size_t i = 0; i < parent1.getGenotype().size(); i++) {
        double alpha = dist(rnd);
        int gene = static_cast<int>(alpha * parent1.getGenotype()[i] + (1 - alpha) * parent2.getGenotype()[i]);
        offspringGenotype.push_back(gene);
    }

    Individual offspring = Individual(rnd, incidents, stations, odMatrix, events, numObjectives, numDepots, numAmbulances, crowdingDistance, mutationProbability);
    offspring.setGenotype(offspringGenotype);
    offspring.repair();
    offspring.evaluateObjectives(events);

    return offspring;
}

void Population::calculateCrowdingDistance(std::vector<Individual>& front) {
    size_t numObjectives = front[0].getObjectives().size();
    // Initialize crowding distance for all individuals in the front
    for (Individual& individual : front) {
        individual.setCrowdingDistance(0.0);
    }

    for (size_t i = 0; i < numObjectives; ++i) {
        // Sort individuals in the front based on the objective i
        std::sort(front.begin(), front.end(), [i](const Individual& a, const Individual& b) {
            return a.getObjectives()[i] < b.getObjectives()[i];
        });

        // Assign infinity distance to boundary individuals
        front.front().setCrowdingDistance(std::numeric_limits<double>::infinity());
        front.back().setCrowdingDistance(std::numeric_limits<double>::infinity());

        // Calculate normalized distance for intermediate individuals
        double objectiveRange = front.back().getObjectives()[i] - front.front().getObjectives()[i];
        if (objectiveRange > 0) { // Avoid division by zero
            for (size_t j = 1; j < front.size() - 1; ++j) {
                front[j].setCrowdingDistance(front[j].getCrowdingDistance() + (front[j + 1].getObjectives()[i] - front[j - 1].getObjectives()[i]) / objectiveRange);
            }
        }
    }
}

void Population::evolve(int generations) {
    for (int gen = 0; gen < generations; gen++) {
        // step 1: parent Selection
        int numParents = 2;
        int tournamentSize = 5;
        std::vector<Individual> parents = parentSelection(numParents, tournamentSize);

        // step 2: crossover to create offspring
        std::vector<Individual> children;
        children.reserve(populationSize);

        for (int i = 0; i < populationSize; i += 2) {
            Individual offspring = crossover(parents[i % numParents], parents[(i + 1) % numParents]);
            offspring.mutate();
            offspring.evaluateObjectives(events);
            children.push_back(offspring);
        }

        // step 3: survivor Selection
        // combining existing population with children
        addChildren(children);
        individuals = survivorSelection(populationSize);

        Individual fittest = findFittest();
        std::cout << "Generation " << gen  << ": " << fittest.getFitness() << ", [" << countUnique(individuals) << "] unique individuals" << std::endl;
        
      }
      
    // run one last time to print metrics
    Individual finalIndividual = findFittest();
    bool saveMetricsToFile = true;
    finalIndividual.evaluateObjectives(events, saveMetricsToFile);
}

int Population::countUnique(const std::vector<Individual>& population) {
    std::vector<std::vector<int>> genotypes;
    genotypes.reserve(population.size());

    for (const auto& individual : population) {
        genotypes.push_back(individual.getGenotype());
    }

    // sort genotypes to bring identical ones together
    std::sort(genotypes.begin(), genotypes.end());

    // remove consecutive duplicates
    auto lastUnique = std::unique(genotypes.begin(), genotypes.end());

    // calculate the distance between the beginning and the point of last unique element
    return std::distance(genotypes.begin(), lastUnique);
}

const Individual Population::findFittest() {
    auto fittest = std::max_element(
        individuals.begin(),
        individuals.end(),
        [](const Individual &a, const Individual &b) {
            return a.getFitness() > b.getFitness();
        }
    );

    return *fittest;
}

const Individual Population::findLeastFit() {
    auto leastFit = std::min_element(
        individuals.begin(),
        individuals.end(),
        [](const Individual &a, const Individual &b) {
            return a.getFitness() > b.getFitness();
        }
    );

    return *leastFit;
}

const double Population::averageFitness() {
    if (individuals.empty()) {
        return 0.0;
    }

    double totalFitness = std::accumulate(
        individuals.begin(),
        individuals.end(),
        0.0,
        [](double sum, const Individual& individual) {
            return sum + individual.getFitness();
        }
    );

    return totalFitness / individuals.size();
}

/**
 * @file PopulationTSGA.cpp
 *
 * @copyright Copyright (c) 2024 Emergency-Optimizers
 */

/* external libraries */
#include <sstream>
#include <iomanip>
/* internal libraries */
#include "ProgressBar.hpp"
#include "heuristics/PopulationTSGA.hpp"
#include "file-reader/Settings.hpp"
#include "Utils.hpp"
#include "simulator/MonteCarloSimulator.hpp"

PopulationTSGA::PopulationTSGA(
    std::mt19937& rnd,
    int populationSize,
    double mutationProbability,
    const bool dayShift,
    int numTimeSegments
) : rnd(rnd), populationSize(populationSize), mutationProbability(mutationProbability), dayShift(dayShift), numTimeSegments(numTimeSegments) {
    MonteCarloSimulator monteCarloSim(
        rnd,
        Settings::get<int>("SIMULATE_YEAR"),
        Settings::get<int>("SIMULATE_MONTH"),
        Settings::get<int>("SIMULATE_DAY"),
        dayShift,
        Settings::get<int>("SIMULATION_GENERATION_WINDOW_SIZE")
    );

    numDepots = Stations::getInstance().getDepotIndices(dayShift).size();
    numAmbulances = dayShift ? Settings::get<int>("TOTAL_AMBULANCES_DURING_DAY") : Settings::get<int>("TOTAL_AMBULANCES_DURING_NIGHT");

    events = monteCarloSim.generateEvents();

    for (int i = 0; i < populationSize; i++) {
        IndividualTSGA individual = IndividualTSGA(rnd, numDepots, numAmbulances, numTimeSegments, mutationProbability, dayShift, false);
        individuals.push_back(individual);
    }

    evaluateFitness();
}

void PopulationTSGA::evaluateFitness() {
    for (IndividualTSGA& individual : individuals) {
        individual.evaluateFitness(events);
        }
}

std::vector<IndividualTSGA> PopulationTSGA::parentSelection(int numParents, int tournamentSize) {
    std::vector<IndividualTSGA> selectedParents;

    for (int i = 0; i < numParents; i++) {
        std::vector<IndividualTSGA> tournament;
        for (int j = 0; j < tournamentSize; j++) {
            tournament.push_back(getRandomElement(rnd, individuals));
        }

        // select the individual with the highest fitness in the tournament
        auto best = std::max_element(
            tournament.begin(),
            tournament.end(),
            [](const IndividualTSGA &a, const IndividualTSGA &b) {
                return a.getFitness() > b.getFitness();
            }
        );

        selectedParents.push_back(*best);
    }

    return selectedParents;
}

std::vector<IndividualTSGA> PopulationTSGA::survivorSelection(int numSurvivors) {
    // sort the current population based on fitness in descending order
    std::sort(
        individuals.begin(),
        individuals.end(),
        [](const IndividualTSGA &a, const IndividualTSGA &b) { return a.getFitness() < b.getFitness(); }
    );

    std::vector<IndividualTSGA> survivors;

    // calculate the actual number of survivors to keep, which is the minimum
    // of numSurvivors and the current population size to avoid out-of-bounds access
    int actualNumSurvivors = std::min(numSurvivors, static_cast<int>(individuals.size()));

    // copy the top 'actualNumSurvivors' individuals to the survivors vector
    for (int i = 0; i < actualNumSurvivors; i++) {
        survivors.push_back(individuals[i]);
    }

    return survivors;
}

void PopulationTSGA::addChildren(const std::vector<IndividualTSGA>& children) {
    for (int i = 0; i < children.size(); i++) individuals.push_back(children[i]);
}

IndividualTSGA PopulationTSGA::crossover(const IndividualTSGA& parent1, const IndividualTSGA& parent2) {
    std::vector<std::vector<int>> offspringGenotype = parent2.getGenotype();

    std::uniform_int_distribution<> dist(0, 1);

    for (size_t t = 0; t < numTimeSegments; ++t) {
        if (dist(rnd) == 1) {
            offspringGenotype[t] = parent1.getGenotype()[t];
        }
    }

    IndividualTSGA offspring(rnd, numDepots, numAmbulances, numTimeSegments, mutationProbability, dayShift, true);
    offspring.setGenotype(offspringGenotype);
    // offspring.repair();
    offspring.evaluateFitness(events);

    return offspring;
}

void PopulationTSGA::evolve(int generations) {
    ProgressBar progressBar(generations, "Running TSGA");

    for (int gen = 0; gen < generations; gen++) {
        int numParents = populationSize / 2;

        // don't branch if population size is set to 1
        // this is for debugging when we only want to simulate once
        if (numParents > 1) {
            // step 1: parent selection
            int tournamentSize = 3;
            std::vector<IndividualTSGA> parents = parentSelection(numParents, tournamentSize);

            // step 2: crossover to create offspring
            std::vector<IndividualTSGA> children;
            children.reserve(populationSize);

            for (int i = 0; i < populationSize; i += 2) {
                IndividualTSGA offspring = crossover(parents[i % numParents], parents[(i + 1) % numParents]);
                offspring.mutate();
                offspring.evaluateFitness(events);
                children.push_back(offspring);
            }
            // step 3: survivor selection
            // combining existing population with children
            addChildren(children);
            individuals = survivorSelection(populationSize);
        }

        IndividualTSGA fittest = findFittest();

        std::ostringstream postfix;
        postfix
            << "Best fitness: " << std::fixed << std::setprecision(2) << std::setw(6) << fittest.getFitness()
            << ", Valid: " << (fittest.isValid() ? "true " : "false");

        progressBar.update(gen + 1, postfix.str());
    }

    // run one last time to print metrics
    IndividualTSGA finalIndividual = findFittest();
    bool saveMetricsToFile = true;
    finalIndividual.evaluateFitness(events, saveMetricsToFile);

    std::cout
        << "Fittest IndividualGA: " << finalIndividual.getFitness()
        << (finalIndividual.isValid() ? " [valid]\n" : " [invalid]\n")
        << std::endl;
    finalIndividual.printTimeSegmentedChromosome();
}

int PopulationTSGA::countUnique(const std::vector<IndividualTSGA>& population) {
    std::vector<std::string> genotypeStrings;
    genotypeStrings.reserve(population.size());

    for (const auto& individual : population) {
        std::ostringstream genotypeStream;
        for (const auto& segment : individual.getGenotype()) {
            for (const auto& depotAllocation : segment) {
                genotypeStream << depotAllocation << ",";
            }
            genotypeStream << ";";
        }
        genotypeStrings.push_back(genotypeStream.str());
    }

    std::sort(genotypeStrings.begin(), genotypeStrings.end());

    auto lastUnique = std::unique(genotypeStrings.begin(), genotypeStrings.end());

    return std::distance(genotypeStrings.begin(), lastUnique);
}

const IndividualTSGA PopulationTSGA::findFittest() {
    auto fittest = std::max_element(
        individuals.begin(),
        individuals.end(),
        [](const IndividualTSGA &a, const IndividualTSGA &b) {
            return a.getFitness() > b.getFitness();
        }
    );

    return *fittest;
}

const IndividualTSGA PopulationTSGA::findLeastFit() {
    auto leastFit = std::min_element(
        individuals.begin(),
        individuals.end(),
        [](const IndividualTSGA &a, const IndividualTSGA &b) {
            return a.getFitness() > b.getFitness();
        }
    );

    return *leastFit;
}

const double PopulationTSGA::averageFitness() {
    if (individuals.empty()) {
        return 0.0;
    }

    double totalFitness = std::accumulate(
        individuals.begin(),
        individuals.end(),
        0.0,
        [](double sum, const IndividualTSGA& individual) {
            return sum + individual.getFitness();
        }
    );

    return totalFitness / individuals.size();
}

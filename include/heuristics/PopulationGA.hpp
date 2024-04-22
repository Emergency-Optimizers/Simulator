/**
 * @file PopulationGA.hpp
 *
 * @copyright Copyright (c) 2024 Emergency-Optimizers
 */

#pragma once

/* external libraries */
#include <vector>
#include <numeric>
#include <random>
#include <string>
#include <utility>
#include <map>
#include <thread>
/* internal libraries */
#include "heuristics/Individual.hpp"
#include "file-reader/Settings.hpp"
#include "file-reader/Incidents.hpp"
#include "file-reader/Stations.hpp"
#include "file-reader/ODMatrix.hpp"
#include "simulator/Event.hpp"
#include "heuristics/GenotypeInitType.hpp"
#include "heuristics/MutationType.hpp"
#include "heuristics/CrossoverType.hpp"
#include "heuristics/SelectionType.hpp"

class PopulationGA {
 private:
    std::vector<GenotypeInitType> genotypeInits;
    std::vector<double> genotypeInitsTickets;
    std::vector<CrossoverType> crossovers;
    std::vector<double> crossoversTickets;
    std::vector<SelectionType> parentSelections;
    std::vector<double> parentSelectionsTickets;
    std::vector<SelectionType> survivorSelections;
    std::vector<double> survivorSelectionsTickets;
    int numThreads = std::thread::hardware_concurrency() - 2;
    bool multiThread = Settings::get<bool>("MULTI_THREAD");

    void generatePopulation();
    void getPossibleGenotypeInits();
    void getPossibleMutations();
    void getPossibleCrossovers();
    void getPossibleParentSelections();
    void getPossibleSurvivorSelections();
    std::vector<std::pair<int, double>> generateIndexFitnessPair(const int startIndex = 0);
    std::vector<int> tournamentSelection(
        const std::vector<std::pair<int, double>>& population,
        const int k,
        const int tournamentSize
    );
    std::vector<int> rouletteWheelSelection(
        const std::vector<std::pair<int, double>>& population,
        const int k
    );
    std::vector<int> elitismSelection(
        const std::vector<std::pair<int, double>>& population,
        const int k
    );
    std::vector<int> rankSelection(
        const std::vector<std::pair<int, double>>& population,
        const int k,
        const double selectionPressure
    );
    std::vector<std::vector<std::vector<int>>> singlePointCrossover(
        const std::vector<std::vector<int>>& parent1Genotype,
        const std::vector<std::vector<int>>& parent2Genotype
    );
    std::vector<std::vector<std::vector<int>>> segmentSwapCrossover(
        const std::vector<std::vector<int>>& parent1Genotype,
        const std::vector<std::vector<int>>& parent2Genotype
    );
    std::vector<std::vector<std::vector<int>>> bestAllocationCrossover(
        const std::vector<std::vector<int>>& parent1Genotype,
        const std::vector<std::vector<int>>& parent2Genotype,
        const std::vector<double>& parent1AllocationsFitness,
        const std::vector<double>& parent2AllocationsFitness
    );

 protected:
    std::mt19937 rnd = std::mt19937(Settings::get<int>("SEED"));
    const std::vector<Event>& events;
    std::vector<Individual> individuals;
    int generation = 0;
    const bool dayShift = Settings::get<bool>("SIMULATE_DAY_SHIFT");
    const int populationSize = Settings::get<int>("POPULATION_SIZE");
    const int numDepots = static_cast<int>(Stations::getInstance().getDepotIndices(dayShift).size());
    const int numAmbulances = dayShift ?
        Settings::get<int>("TOTAL_AMBULANCES_DURING_DAY") : Settings::get<int>("TOTAL_AMBULANCES_DURING_NIGHT");
    const int numTimeSegments = Settings::get<int>("NUM_TIME_SEGMENTS");
    const DispatchEngineStrategyType dispatchStrategy = Settings::get<DispatchEngineStrategyType>("DISPATCH_STRATEGY");
    const double crossoverProbability = Settings::get<float>("CROSSOVER_PROBABILITY");
    const double mutationProbability = Settings::get<float>("MUTATION_PROBABILITY");
    std::vector<MutationType> mutations;
    std::vector<double> mutationsTickets;
    const std::string heuristicName = "GA";
    std::map<std::string, std::vector<std::vector<double>>> metrics = {
        {"fitness", {}},
        {"diversity", {}},
        {"avg_response_time_urban_a", {}},
        {"avg_response_time_urban_h", {}},
        {"avg_response_time_urban_v1", {}},
        {"avg_response_time_rural_a", {}},
        {"avg_response_time_rural_h", {}},
        {"avg_response_time_rural_v1", {}},
        {"percentage_violations", {}},
        {"percentage_violations_urban", {}},
        {"percentage_violations_rural", {}},
    };
    const int maxRunTimeSeconds = static_cast<int>(Settings::get<float>("STOPPING_CRITERIA_TIME_MIN") * 60.0f);
    int runTimeDuration = 0;
    std::chrono::steady_clock::time_point startRunTimeClock;
    const int maxGenerations = Settings::get<int>("STOPPING_CRITERIA_MAX_GENERATIONS");
    const int minDiversity = Settings::get<int>("STOPPING_CRITERIA_MIN_DIVERSITY");

    virtual std::vector<Individual> createOffspring();
    virtual std::vector<Individual> parentSelection();
    virtual std::vector<Individual> survivorSelection();
    std::vector<Individual> crossover(const Individual& parent1, const Individual& parent2);
    Individual createIndividual(const bool child);
    virtual void sortIndividuals();
    virtual const std::string getProgressBarPostfix() const;
    virtual const std::string getHeuristicName() const;
    const Individual getFittest() const;
    virtual void storeGenerationMetrics();
    int countUnique() const;
    bool shouldStop();

 public:
    explicit PopulationGA(const std::vector<Event>& events);
    virtual void evolve();
};

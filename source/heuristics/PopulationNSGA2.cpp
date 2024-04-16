/**
 * @file PopulationNSGA2.cpp
 *
 * @copyright Copyright (c) 2024 Emergency-Optimizers
 */

/* external libraries */
#include <sstream>
#include <iomanip>
#include <string>
#include <iostream>
#include <set>
/* internal libraries */
#include "ProgressBar.hpp"
#include "heuristics/PopulationNSGA2.hpp"
#include "file-reader/Settings.hpp"
#include "Utils.hpp"
#include "simulator/MonteCarloSimulator.hpp"

PopulationNSGA2::PopulationNSGA2(
    std::mt19937& rnd,
    const std::vector<Event>& events,
    const bool dayShift,
    const DispatchEngineStrategyType dispatchStrategy,
    const int numAmbulancesDuringDay,
    const int numAmbulancesDuringNight,
    const int populationSize,
    const double mutationProbability,
    const double crossoverProbability,
    const int numTimeSegments
) : PopulationGA(
    rnd,
    events,
    dayShift,
    dispatchStrategy,
    numAmbulancesDuringDay,
    numAmbulancesDuringNight,
    populationSize,
    mutationProbability,
    crossoverProbability,
    numTimeSegments
) { }

void PopulationNSGA2::evolve() {
    // sort and store metrics for initial population
    nonDominatedSort();
    for (auto& front : fronts) {
        calculateCrowdingDistance(front);
    }

    sortIndividuals();
    storeGenerationMetrics();

    // init progress bar
    ProgressBar progressBar(Settings::get<int>("GENERATION_SIZE"), progressBarPrefix, getProgressBarPostfix());

    for (int generationIndex = 0; generationIndex < Settings::get<int>("GENERATION_SIZE"); generationIndex++) {
        // create offspring
        std::vector<Individual> offspring;
        while (offspring.size() < populationSize) {
            const int individualsToSelect = 2;
            std::vector<Individual> parents = tournamentSelection(
                individualsToSelect,
                Settings::get<int>("PARENT_SELECTION_TOURNAMENT_SIZE")
            );

            if (getRandomDouble(rnd) < crossoverProbability) {
                std::vector<Individual> children = crossover(parents[0], parents[1]);

                for (Individual& child : children) {
                    child.evaluate(events, dayShift, dispatchStrategy);
                }

                // calculate how many children can be added without exceeding populationSize
                const size_t spaceLeft = populationSize - offspring.size();
                const size_t childrenToAdd = std::min(children.size(), spaceLeft);

                // add children directly to offspring, ensuring not to exceed populationSize
                offspring.insert(offspring.end(), children.begin(), children.begin() + childrenToAdd);
            } else {
                // clone one of the parents
                const bool isChild = true;
                Individual clonedOffspring = createIndividual(isChild);

                clonedOffspring.genotype = getRandomBool(rnd) ? parents[0].genotype : parents[1].genotype;

                // apply mutation to the cloned offspring
                clonedOffspring.mutate(mutationProbability, mutations, mutationsTickets);

                offspring.push_back(clonedOffspring);
            }
        }

        // combining existing population with children
        for (int i = 0; i < offspring.size(); i++) {
            individuals.push_back(offspring[i]);
        }

        // set rank and crowding distance
        nonDominatedSort();
        for (auto& front : fronts) {
            calculateCrowdingDistance(front);
        }

        // survivor selection
        individuals = selectNextGeneration(populationSize);

        // sort population for metrics
        sortIndividuals();

        // update progress bar
        progressBar.update(generationIndex + 1, getProgressBarPostfix());

        storeGenerationMetrics();
    }

    // get best individual
    Individual finalIndividual = getFittest();

    // write metrics to file
    saveDataToJson(
        Settings::get<std::string>("UNIQUE_RUN_ID"),
        heuristicName,
        metrics
    );
    writeMetrics(Settings::get<std::string>("UNIQUE_RUN_ID"), finalIndividual.simulatedEvents);

    printTimeSegmentedAllocationTable(
        dayShift,
        numTimeSegments,
        finalIndividual.genotype,
        finalIndividual.simulatedEvents,
        finalIndividual.allocationsFitness
    );

    printAmbulanceWorkload(finalIndividual.simulatedAmbulances);

    std::cout
        << "Goal:" << std::endl
        << "\t A, urban: <12 min" << std::endl
        << "\t A, rural: <25 min" << std::endl
        << "\t H, urban: <30 min" << std::endl
        << "\t H, rural: <40 min" << std::endl
        << std::endl
        << "Avg. response time (A, urban): \t\t" << finalIndividual.objectiveAvgResponseTimeUrbanA << "s"
        << " (" << (finalIndividual.objectiveAvgResponseTimeUrbanA  / 60.0) << "m)" << std::endl
        << "Avg. response time (A, rural): \t\t" << finalIndividual.objectiveAvgResponseTimeRuralA << "s"
        << " (" << (finalIndividual.objectiveAvgResponseTimeRuralA / 60.0) << "m)" << std::endl
        << "Avg. response time (H, urban): \t\t" << finalIndividual.objectiveAvgResponseTimeUrbanH << "s"
        << " (" << (finalIndividual.objectiveAvgResponseTimeUrbanH / 60.0) << "m)" << std::endl
        << "Avg. response time (H, rural): \t\t" << finalIndividual.objectiveAvgResponseTimeRuralH << "s"
        << " (" << (finalIndividual.objectiveAvgResponseTimeRuralH / 60.0) << "m)" << std::endl
        << "Avg. response time (V1, urban): \t" << finalIndividual.objectiveAvgResponseTimeUrbanV1 << "s"
        << " (" << (finalIndividual.objectiveAvgResponseTimeUrbanV1 / 60.0) << "m)" << std::endl
        << "Avg. response time (V1, rural): \t" << finalIndividual.objectiveAvgResponseTimeRuralV1 << "s"
        << " (" << (finalIndividual.objectiveAvgResponseTimeRuralV1 / 60.0) << "m)" << std::endl
        << "Percentage violations: \t\t\t" << (finalIndividual.objectivePercentageViolations * 100.0) << "%" << std::endl;
}

void PopulationNSGA2::nonDominatedSort() {
    fronts.clear();
    std::vector<int> dominationCounts(individuals.size(), 0);
    std::vector<std::vector<Individual*>> tempFronts;
    tempFronts.resize(individuals.size());

    for (int i = 0; i < individuals.size(); i++) {
        Individual* current = &individuals[i];
        current->dominatedIndividuals.clear();
        current->frontNumber = 0;
    }

    for (int i = 0; i < individuals.size(); i++) {
        Individual* current = &individuals[i];
        for (int j = i + 1; j < individuals.size(); j++) {
            Individual* other = &individuals[j];
            if (current->dominates(*other)) {
                current->dominatedIndividuals.push_back(other);
                dominationCounts[j]++;
            } else if (other->dominates(*current)) {
                other->dominatedIndividuals.push_back(current);
                dominationCounts[i]++;
            }
        }
        if (dominationCounts[i] == 0) {
            tempFronts[0].push_back(current);
        }
    }

    for (int f = 0; f < tempFronts.size() && !tempFronts[f].empty(); f++) {
        fronts.push_back(std::vector<Individual*>());
        for (Individual* ind : tempFronts[f]) {
            fronts[f].push_back(ind);
            ind->frontNumber = f;
            for (Individual* dominatedInd : ind->dominatedIndividuals) {
                dominationCounts[dominatedInd - &individuals[0]]--;
                if (dominationCounts[dominatedInd - &individuals[0]] == 0) {
                    if (f + 1 >= tempFronts.size()) tempFronts.resize(f + 2);
                    tempFronts[f + 1].push_back(dominatedInd);
                    dominatedInd->frontNumber = f + 1;
                }
            }
        }
    }
}

void PopulationNSGA2::calculateCrowdingDistance(std::vector<Individual*>& front) {
    int size = static_cast<int>(front.size());
    if (size == 0) return;
    int numObjectives = static_cast<int>(front[0]->objectives.size());

    for (Individual* ind : front) {
        ind->crowdingDistance = 0.0;
    }

    for (int m = 0; m < numObjectives; m++) {
        std::sort(front.begin(), front.end(), [m](Individual* a, Individual* b) {
            return a->objectives[m] < b->objectives[m];
        });
        front.front()->crowdingDistance = std::numeric_limits<double>::max();
        front.back()->crowdingDistance = std::numeric_limits<double>::max();

        double minObjective = front.front()->objectives[m];
        double maxObjective = front.back()->objectives[m];
        double range = maxObjective - minObjective;

        if (range == 0) {
            continue;
        }

        for (int i = 1; i < size - 1; i++) {
            front[i]->crowdingDistance += (front[i + 1]->objectives[m] - front[i - 1]->objectives[m]) / range;
        }
    }
}

std::vector<Individual> PopulationNSGA2::selectNextGeneration(const int selectionSize) {
    std::vector<Individual> nextGeneration;
    int count = 0;
    for (auto& front : fronts) {
        if (count + front.size() <= selectionSize) {
            for (auto& individual : front) {
                nextGeneration.push_back(*individual);
            }
            count += static_cast<int>(front.size());
        } else {
            std::sort(front.begin(), front.end(), [](const Individual* a, const Individual* b) {
                return a->crowdingDistance > b->crowdingDistance;
            });
            int toAdd = selectionSize - count;
            for (int i = 0; i < toAdd; ++i) {
                nextGeneration.push_back(*front[i]);
            }
            break;
        }
    }
    return nextGeneration;
}

std::vector<Individual> PopulationNSGA2::tournamentSelection(const int k, const int tournamentSize) {
    std::vector<Individual> selected;
    while (selected.size() < k) {
        Individual winner = individuals[getRandomInt(rnd, 0, static_cast<int>(individuals.size()) - 1)];
        for (int i = 1; i < tournamentSize; i++) {
            Individual contender = individuals[getRandomInt(rnd, 0, static_cast<int>(individuals.size()) - 1)];
            winner = tournamentWinner(winner, contender);
        }
        selected.push_back(winner);
    }
    return selected;
}

Individual PopulationNSGA2::tournamentWinner(Individual& individual1, Individual& individual2) {
    if (individual1.frontNumber < individual2.frontNumber) {
        return individual1;
    } else if (individual1.frontNumber > individual2.frontNumber) {
        return individual2;
    } else {
        if (individual1.crowdingDistance > individual2.crowdingDistance) {
            return individual1;
        } else {
            return individual2;
        }
    }
}

void PopulationNSGA2::sortIndividuals() {
    std::sort(
        individuals.begin(),
        individuals.end(),
        [](const Individual &a, const Individual &b) {
            if (a.frontNumber == b.frontNumber) {
                return a.crowdingDistance > b.crowdingDistance;
            }

            return a.frontNumber < b.frontNumber;
        }
    );
}

const std::string PopulationNSGA2::getProgressBarPostfix() const {
    const Individual fittest = getFittest();

    const double violations = fittest.objectivePercentageViolations;
    const double diversity = static_cast<double>(countUnique()) / static_cast<double>(individuals.size());

    std::ostringstream postfix;
    postfix
        << "Violations: " << std::fixed << std::setprecision(2) << std::setw(6)
        << (violations * 100.0) << "%"
        << ", Diversity: " << std::fixed << std::setprecision(2) << std::setw(6)
        << (diversity * 100.0) << "%";

    return postfix.str();
}

void PopulationNSGA2::storeGenerationMetrics() {
    std::vector<double> generationDiversity;
    std::vector<double> generationAvgResponseTimeUrbanA;
    std::vector<double> generationAvgResponseTimeUrbanH;
    std::vector<double> generationAvgResponseTimeUrbanV1;
    std::vector<double> generationAvgResponseTimeRuralA;
    std::vector<double> generationAvgResponseTimeRuralH;
    std::vector<double> generationAvgResponseTimeRuralV1;
    std::vector<double> generationPercentageViolations;
    std::vector<double> generationFrontNumber;
    std::vector<double> generationCrowdingDistance;

    // add individual data
    for (int individualIndex = 0; individualIndex < individuals.size(); individualIndex++) {
        generationAvgResponseTimeUrbanA.push_back(individuals[individualIndex].objectiveAvgResponseTimeUrbanA);
        generationAvgResponseTimeUrbanH.push_back(individuals[individualIndex].objectiveAvgResponseTimeUrbanH);
        generationAvgResponseTimeUrbanV1.push_back(individuals[individualIndex].objectiveAvgResponseTimeUrbanV1);
        generationAvgResponseTimeRuralA.push_back(individuals[individualIndex].objectiveAvgResponseTimeRuralA);
        generationAvgResponseTimeRuralH.push_back(individuals[individualIndex].objectiveAvgResponseTimeRuralH);
        generationAvgResponseTimeRuralV1.push_back(individuals[individualIndex].objectiveAvgResponseTimeRuralV1);
        generationPercentageViolations.push_back(individuals[individualIndex].objectivePercentageViolations);
        generationFrontNumber.push_back(individuals[individualIndex].frontNumber);
        generationCrowdingDistance.push_back(individuals[individualIndex].crowdingDistance);
    }

    // add global generation data
    generationDiversity.push_back(static_cast<double>(countUnique()) / static_cast<double>(individuals.size()));

    // add to metrics
    metrics["diversity"].push_back(generationDiversity);
    metrics["avg_response_time_urban_a"].push_back(generationAvgResponseTimeUrbanA);
    metrics["avg_response_time_urban_h"].push_back(generationAvgResponseTimeUrbanH);
    metrics["avg_response_time_urban_v1"].push_back(generationAvgResponseTimeUrbanV1);
    metrics["avg_response_time_rural_a"].push_back(generationAvgResponseTimeRuralA);
    metrics["avg_response_time_rural_h"].push_back(generationAvgResponseTimeRuralH);
    metrics["avg_response_time_rural_v1"].push_back(generationAvgResponseTimeRuralV1);
    metrics["percentage_violations"].push_back(generationPercentageViolations);
    metrics["front_number"].push_back(generationFrontNumber);
    metrics["crowding_distance"].push_back(generationCrowdingDistance);
}

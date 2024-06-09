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

PopulationNSGA2::PopulationNSGA2(const std::vector<Event>& events) : PopulationGA(events) { }

void PopulationNSGA2::evolve(const bool verbose, std::string extraFileName) {
    // sort and store metrics for initial population
    nonDominatedSort();
    for (auto& front : fronts) {
        calculateCrowdingDistance(front);
    }

    storeGenerationMetrics();

    // init progress bar
    ProgressBar progressBar(maxRunTimeSeconds, "Running " + getHeuristicName(), getProgressBarPostfix());
    startRunTimeClock = std::chrono::steady_clock::now();

    bool keepRunning = true;
    bool autoStopProgressBar = false;
    bool lastPrintProgressBar = false;

    while (keepRunning) {
        generation++;

        // create offspring
        std::vector<Individual> offspring = createOffspring();

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
        individuals = survivorSelection();

        // store generation metrics for analysis
        storeGenerationMetrics();

        // check stopping criteria
        keepRunning = !shouldStop();

        // update progress bar
        progressBar.update(runTimeDuration, getProgressBarPostfix(), autoStopProgressBar, lastPrintProgressBar);
    }

    // get best individual and print last progress bar
    // there might be better individuals in first front, analyze heursitc.json, see Data-Processing repo for how we did it
    Individual finalIndividual = getFittest();

    lastPrintProgressBar = true;
    progressBar.update(runTimeDuration, getProgressBarPostfix(), autoStopProgressBar, lastPrintProgressBar);

    // write to file
    const std::string dirName = Settings::get<std::string>("UNIQUE_RUN_ID") + "_" + getHeuristicName();
    saveDataToJson(dirName, "heuristic" + extraFileName, metrics);
    for (int i = 0; i < individuals.size(); i++) {
        writeEvents(dirName, finalIndividual.simulatedEvents, "events" + extraFileName + "_ind_" + std::to_string(i));
        writeGenotype(dirName, finalIndividual.genotype, "genotype" + extraFileName + "_ind_" + std::to_string(i));
        writeAmbulances(dirName, finalIndividual.simulatedAmbulances, "ambulances" + extraFileName + "_ind_" + std::to_string(i));
    }

    // print metrics to terminal
    if (verbose) {
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

std::vector<Individual> PopulationNSGA2::parentSelection() {
    const int individualsToSelect = 2;
    std::vector<Individual> selectedParents = tournamentSelection(
        individualsToSelect,
        Settings::get<int>("PARENT_SELECTION_TOURNAMENT_SIZE")
    );

    return selectedParents;
}

std::vector<Individual> PopulationNSGA2::survivorSelection() {
    std::vector<Individual> nextGeneration;
    int count = 0;
    for (auto& front : fronts) {
        if (count + front.size() <= populationSize) {
            // add all individuals from the current front to the next generation
            for (auto& individual : front) {
                nextGeneration.push_back(*individual);
            }

            count += static_cast<int>(front.size());
        } else {
            // sort the remaining individuals in the current front based on crowding distance
            std::sort(front.begin(), front.end(), [](const Individual* a, const Individual* b) {
                return a->crowdingDistance > b->crowdingDistance;
            });

            // add individuals until full
            int toAdd = populationSize - count;
            for (int i = 0; i < toAdd; i++) {
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
    // decide winner based on front, crowding distance if from same front
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
    // hard coded objectives to sort by, was only used for testing
    std::sort(
        individuals.begin(),
        individuals.end(),
        [](const Individual &a, const Individual &b) { return a.objectivePercentageViolations < b.objectivePercentageViolations; }
    );
}

const std::string PopulationNSGA2::getProgressBarPostfix() const {
    const Individual fittest = getFittest();

    const double violationsUrban = fittest.objectivePercentageViolationsUrban;
    const double violationsRural = fittest.objectivePercentageViolationsRural;
    const double diversity = static_cast<double>(countUnique()) / static_cast<double>(individuals.size());

    std::ostringstream postfix;
    postfix
        << "Gen: " << std::setw(4) << generation
        << ", Div: " << std::fixed << std::setprecision(2) << std::setw(4) << diversity
        << ", Vio: ("
        << "U: " << std::fixed << std::setprecision(2) << std::setw(4) << violationsUrban
        << ", R: " << std::fixed << std::setprecision(2) << std::setw(4) << violationsRural
        << ")";

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
    std::vector<double> generationPercentageViolationsUrban;
    std::vector<double> generationPercentageViolationsRural;
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
        generationPercentageViolationsUrban.push_back(individuals[individualIndex].objectivePercentageViolationsUrban);
        generationPercentageViolationsRural.push_back(individuals[individualIndex].objectivePercentageViolationsRural);
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
    metrics["percentage_violations_urban"].push_back(generationPercentageViolationsUrban);
    metrics["percentage_violations_rural"].push_back(generationPercentageViolationsRural);
    metrics["front_number"].push_back(generationFrontNumber);
    metrics["crowding_distance"].push_back(generationCrowdingDistance);
}

const std::string PopulationNSGA2::getHeuristicName() const {
    return heuristicName;
}

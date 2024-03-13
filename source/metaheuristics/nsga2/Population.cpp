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
        Individual individual = Individual(rnd, incidents, stations, odMatrix, events, numObjectives, numDepots, numAmbulances, mutationProbability, false);
        individuals.push_back(individual);
    }
    evaluateObjectives();
}

void Population::evaluateObjectives() {
    for (Individual& individual : individuals) {
        individual.evaluateObjectives(events);
    }
}

std::vector<Individual> Population::parentSelection(int tournamentSize) {
    std::cout << "Inside Parent Selection" << std::endl;

    std::vector<Individual> selectedParents;

    for (int i = 0; i < 2; i++) {
        std::vector<Individual> tournament;
        for (int j = 0; j < tournamentSize; j++) {
            tournament.push_back(Utils::getRandomElement(rnd, individuals));
        }

        // tournament selection based on rank and crowding distance
        auto best = std::min_element(tournament.begin(), tournament.end(),
                                     [](const Individual &a, const Individual &b) {
                                         // first, compare by rank (lower rank is better)
                                         if (a.getRank() != b.getRank()) {
                                             return a.getRank() < b.getRank();
                                         }
                                         // if ranks are equal, compare by crowding distance (higher is better)
                                         return a.getCrowdingDistance() > b.getCrowdingDistance();
                                     });

        selectedParents.push_back(*best);
    }

    return selectedParents;
}


std::vector<Individual> Population::survivorSelection(int numSurvivors) {
    std::vector<Individual> nextGeneration;
    int currentFrontIndex = 0;

    while (nextGeneration.size() < numSurvivors && currentFrontIndex < fronts.size()) {
        auto& currentFront = fronts[currentFrontIndex];

        // Calculate crowding distance for all individuals in the current front
        calculateCrowdingDistance(currentFront);

        // If adding the entire front does not exceed numSurvivors, add all; otherwise, add as many as needed
        if (nextGeneration.size() + currentFront.size() <= numSurvivors) {
            for (auto* indPtr : currentFront) {
                nextGeneration.push_back(*indPtr); // Dereference the pointer to get the Individual
            }
        } else {
            // Need to add partially from the current front based on crowding distance
            std::sort(currentFront.begin(), currentFront.end(), [](const Individual* a, const Individual* b) {
                return a->getCrowdingDistance() > b->getCrowdingDistance(); // Prefer higher crowding distance
            });
            int slotsLeft = numSurvivors - nextGeneration.size();
            int i = 0;
            while (slotsLeft > 0 && i < currentFront.size()) {
                if (!currentFront[i]->getGenotype().empty()) {
                    nextGeneration.push_back(*currentFront[i]); // Add individuals until slots are filled
                    slotsLeft--;
                }
                i++;
            }
        }

        currentFrontIndex++;
    }

    std::cout << "Completed survivor selection. Final generation size: " << nextGeneration.size() << std::endl;
    return nextGeneration;
}

void Population::addChildren(const std::vector<Individual>& children) {
    for (int i = 0; i < children.size(); i++) individuals.push_back(children[i]);
}

Individual Population::crossover(const Individual& parent1, const Individual& parent2) {
    std::vector<int> offspringGenotype;
    offspringGenotype.reserve(parent1.getGenotype().size());

    std::uniform_real_distribution<> dist(0, 1);

    if (parent1.getGenotype().size() == 0 || parent2.getGenotype().size() == 0) {
        std::cerr << "Crossover error: One of the parents has an empty genotype." << std::endl;

        return parent1;
    }

    for (size_t i = 0; i < parent1.getGenotype().size(); i++) {
        double alpha = dist(rnd);
        int gene = static_cast<int>(alpha * parent1.getGenotype()[i] + (1 - alpha) * parent2.getGenotype()[i]);
        offspringGenotype.push_back(gene);
    }

    Individual offspring = Individual(rnd, incidents, stations, odMatrix, events, numObjectives, numDepots, numAmbulances, mutationProbability);
    offspring.setGenotype(offspringGenotype);
    offspring.repair();
    offspring.evaluateObjectives(events);

    return offspring;
}

void Population::calculateCrowdingDistance(std::vector<Individual*>& front) {
    if (front.empty()) return; // Guard against empty fronts

    size_t numObjectives = front[0]->getObjectives().size();
    // Initialize crowding distance for all individuals in the front
    for (Individual* individual : front) {
        individual->setCrowdingDistance(0.0);
    }

    for (size_t i = 0; i < numObjectives; ++i) {
        // Sort individuals in the front based on the i-th objective
        std::sort(front.begin(), front.end(), [i](const Individual* a, const Individual* b) {
            return a->getObjectives()[i] < b->getObjectives()[i];
        });

        // Assign infinity distance to boundary individuals
        front.front()->setCrowdingDistance(std::numeric_limits<double>::infinity());
        front.back()->setCrowdingDistance(std::numeric_limits<double>::infinity());

        // Calculate normalized distance for intermediate individuals
        double objectiveRange = front.back()->getObjectives()[i] - front.front()->getObjectives()[i];
        if (objectiveRange > 0) { // Avoid division by zero
            for (size_t j = 1; j < front.size() - 1; ++j) {
                double distance = (front[j + 1]->getObjectives()[i] - front[j - 1]->getObjectives()[i]) / objectiveRange;
                front[j]->setCrowdingDistance(front[j]->getCrowdingDistance() + distance);
            }
        }
    }
}


void Population::fastNonDominatedSort() {

    std::vector<std::vector<Individual*>> newFronts;
    std::vector<std::vector<Individual*>> dominatedIndividuals(individuals.size());
    std::vector<int> dominationCounter(individuals.size(), 0);

    // Initialize the first front
    newFronts.emplace_back();
    // Determine domination relationships
    for (size_t i = 0; i < individuals.size(); i++) {
        for (size_t j = 0; j < individuals.size(); j++) {
            if (i == j) continue;
            if (individuals[i].dominates(individuals[j])) {
                dominatedIndividuals[i].push_back(&individuals[j]);
            } else if (individuals[j].dominates(individuals[i])) {
                dominationCounter[i]++;
            }
        }
        if (dominationCounter[i] == 0) {
            individuals[i].setRank(0);
            newFronts[0].push_back(&individuals[i]);
        }
    }

    // Construct subsequent fronts
    int currentFront = -1;
    while (currentFront < static_cast<int>(newFronts.size() - 1)) {
        currentFront++;
        std::vector<Individual*> nextFront;
        if (currentFront < static_cast<int>(newFronts.size())) {
            for (auto* p : newFronts[currentFront]) {
                auto pIndex = std::distance(individuals.data(), p); // Find index of p in individuals
                for (auto* q : dominatedIndividuals[pIndex]) {
                    auto qIndex = std::distance(individuals.data(), q); // Find index of q in individuals
                    dominationCounter[qIndex]--;
                    if (dominationCounter[qIndex] == 0) {
                        q->setRank(currentFront + 1);
                        nextFront.push_back(q);
                    }
                }
            }
        }
        if (!nextFront.empty()) {
            newFronts.push_back(nextFront);
        }
    }

    // Update the population's fronts with newFronts
    fronts.insert(fronts.end(), newFronts.begin(), newFronts.end());

    // Update individuals' rank based on front position
    for (size_t frontIndex = 0; frontIndex < fronts.size(); ++frontIndex) {
        for (auto* indPtr : fronts[frontIndex]) {
            indPtr->setRank(frontIndex);
        }
    }

    std::cout << "Fast non-dominated sort completed. Total fronts: " << fronts.size() << std::endl;
}

void Population::evolve(int generations) {
    std::cout << "Starting evolution process...\n";
    for (int gen = 0; gen < generations; ++gen) {
        std::cout << "Generation: " << gen << std::endl;

        // step 1: sort the population into Pareto fronts
        std::cout << "Sorting into Pareto fronts...\n";
        fastNonDominatedSort();

       checkEmptyGenotypes();
       
        // step 2: calculate crowding distance within each front
        std::cout << "Calculating crowding distances...\n";
        for (auto& front : fronts) {
            calculateCrowdingDistance(front);
        }

        checkEmptyGenotypes();

        // step 3: selection, crossover and mutation to create a new offspring pool
        std::cout << "Performing parent selection:\n";
        std::vector<Individual> offspring;
        int tournamentSize = 3;


        while (offspring.size() < populationSize) {
            std::vector<Individual> parents = parentSelection(tournamentSize);
            Individual child = crossover(parents[0], parents[1]);
            child.mutate();
            offspring.push_back(child);
        }

        // step 4: evaluate objectives for offspring
        std::cout << "Evaluating objectives for offspring...\n";
        for (Individual& child : offspring) {
            child.evaluateObjectives(events);
        }

        // step 5: combine, sort, and select the next generation from parents and offspring
        std::cout << "Combining and selecting the next generation...\n";
        individuals.insert(individuals.end(), offspring.begin(), offspring.end());
        fastNonDominatedSort(); // Re-sort combined population
        individuals = survivorSelection(populationSize); // Select the top individuals

        std::cout << "Generation " << gen << " completed.\n\n";
    }

    // Final metrics calculation
    std::cout << "Calculating final metrics...\n";
    Individual finalIndividual = findFittest();
    bool saveMetricsToFile = true;
    finalIndividual.evaluateObjectives(events, saveMetricsToFile);
    std::cout << "Evolution process completed.\n";
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
    // Ensure the population is already sorted into Pareto fronts and crowding distances are calculated
    const auto& firstFront = fronts.front();

    auto fittest = std::max_element(firstFront.begin(), firstFront.end(),
                                    [](const Individual* a, const Individual* b) {
                                        // Compare by crowding distance, assuming you want the individual with the highest crowding distance
                                        return a->getCrowdingDistance() > b->getCrowdingDistance();
                                    });

    // Dereference the iterator to get the pointer, then return the object it points to
    return **fittest;
}


const Individual Population::findLeastFit() {
    const auto& lastFront = fronts.back(); // Assuming the last front contains the least fit individuals

    auto leastFit = std::min_element(lastFront.begin(), lastFront.end(),
                                     [](const Individual* a, const Individual* b) {
                                         // Compare by crowding distance, assuming lower is less fit for the purpose of this method
                                         return a->getCrowdingDistance() < b->getCrowdingDistance();
                                     });

    // Dereference the iterator to get the pointer, then return the object it points to
    return **leastFit;
}

void Population::checkEmptyGenotypes() {
    std::cout << "Checking for empty genotypes..." << std::endl;
    for (const Individual& individual : individuals) {
        if (individual.getGenotype().empty()) {
            std::cout << "Empty genotype found!" << std::endl;
            throw std::runtime_error("Empty genotype encountered. Terminating program.");
        }
    }
}
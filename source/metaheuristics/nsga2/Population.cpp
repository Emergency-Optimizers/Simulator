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

std::vector<Individual> Population::parentSelection(int numParents, int tournamentSize) {
    std::vector<Individual> selectedParents;

    for (int i = 0; i < numParents; i++) {
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
    // assumes that individuals are already sorted into fronts
    std::vector<Individual> nextGeneration;
    int currentFront = 0;

    // continue filling the next generation with individuals from each front
    // until it exceeds the desired number of survivors (population size)
    while (nextGeneration.size() + fronts[currentFront].size() <= numSurvivors) {
        // calculate crowding distance for all individuals in the current front
        calculateCrowdingDistance(fronts[currentFront]);

        // add the entire front to the next generation
        nextGeneration.insert(nextGeneration.end(), fronts[currentFront].begin(), fronts[currentFront].end());
        currentFront++;
    }

    // if last added front exceeds the population limit, sort it by crowding distance
    // and select only as many individuals as needed to fill the next generation
    if (nextGeneration.size() < populationSize) {
        std::sort(fronts[currentFront].begin(), fronts[currentFront].end(),
            [](const Individual& a, const Individual& b) {
                return a.getCrowdingDistance() > b.getCrowdingDistance(); // Higher crowding distance is preferred
            });

        int remainingSlots = populationSize - nextGeneration.size();
        nextGeneration.insert(nextGeneration.end(), fronts[currentFront].begin(), fronts[currentFront].begin() + remainingSlots);
    }

    return nextGeneration;
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

    Individual offspring = Individual(rnd, incidents, stations, odMatrix, events, numObjectives, numDepots, numAmbulances, mutationProbability);
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

void Population::fastNonDominatedSort() {
    std::vector<std::vector<Individual*>> fronts;
    std::vector<int> dominationCounts(individuals.size(), 0);
    std::vector<std::vector<Individual*>> dominatedIndividuals(individuals.size());
    std::vector<int> ranks(individuals.size(), 0);

    // step 1: determine domination relationships and counts
    for (size_t i = 0; i < individuals.size(); ++i) {
        for (size_t j = 0; j < individuals.size(); ++j) {
            if (i != j) {
                if (individuals[i].dominates(individuals[j])) {
                    dominatedIndividuals[i].push_back(&individuals[j]);
                } else if (individuals[j].dominates(individuals[i])) {
                    dominationCounts[i]++;
                }
            }
        }
        if (dominationCounts[i] == 0) {
            ranks[i] = 0;
            if (fronts.empty()) {
                fronts.push_back({});
            }
            fronts[0].push_back(&individuals[i]);
        }
    }

    // step 2: create subsequent fronts
    int currentFront = 0;
    while (currentFront < fronts.size()) {
        std::vector<Individual*> nextFront;
        for (Individual* individual : fronts[currentFront]) {
            for (Individual* dominated : dominatedIndividuals[individual - &individuals[0]]) { // Convert pointer arithmetic to index
                dominationCounts[dominated - &individuals[0]]--; // Decrease domination count
                if (dominationCounts[dominated - &individuals[0]] == 0) {
                    ranks[dominated - &individuals[0]] = currentFront + 1;
                    nextFront.push_back(dominated);
                }
            }
        }
        if (!nextFront.empty()) {
            fronts.push_back(nextFront);
        }
        currentFront++;
    }

    // Optionally update individuals with their ranks for further processing
    for (size_t i = 0; i < individuals.size(); ++i) {
        individuals[i].setRank(ranks[i]);
    }
}

void Population::evolve(int generations) {
    for (int gen = 0; gen < generations; ++gen) {
        // step 1: sort the population into Pareto fronts
        fastNonDominatedSort();

        // step 2: calculate crowding distance within each front
        for (auto& front : fronts) {
            calculateCrowdingDistance(front);
        }

        // step 3: selection, crossover and mutation to create a new offspring pool
        std::vector<Individual> offspring;
        int numParents = populationSize/2;
        int tournamentSize = 3;
        while (offspring.size() < populationSize) {
            std::vector<Individual> parents = parentSelection(numParents, tournamentSize); // Example parameters
            Individual child = crossover(parents[0], parents[1]);
            child.mutate();
            offspring.push_back(child);
        }

        // step 4: evaluate objectives for offspring
        for (Individual& child : offspring) {
            child.evaluateObjectives(events);
        }

        // step 5: combine, sort, and select the next generation from parents and offspring
        individuals.insert(individuals.end(), offspring.begin(), offspring.end());
        fastNonDominatedSort(); // Re-sort combined population
        individuals = survivorSelection(populationSize); // Select the top individuals
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

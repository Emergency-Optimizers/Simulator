/**
 * @file PopulationNSGA.cpp
 *
 * @copyright Copyright (c) 2024 Emergency-Optimizers
 */

/* external libraries */
#include <unordered_set>
/* internal libraries */
#include "ProgressBar.hpp"
#include "heuristics/PopulationNSGA.hpp"
#include "file-reader/Settings.hpp"
#include "file-reader/Stations.hpp"
#include "Utils.hpp"
#include "simulator/MonteCarloSimulator.hpp"

PopulationNSGA::PopulationNSGA(
    std::mt19937& rnd,
    bool useFronts,
    std::vector<float> objectiveWeights,
    int populationSize,
    double mutationProbability,
    double crossoverProbability,
    const bool dayShift,
    int numTimeSegments
) : rnd(rnd),
    useFronts(useFronts),
    objectiveWeights(objectiveWeights),
    populationSize(populationSize),
    dayShift(dayShift),
    mutationProbability(mutationProbability),
    crossoverProbability(crossoverProbability),
    numTimeSegments(numTimeSegments) {
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
    numObjectives = 7;

    events = monteCarloSim.generateEvents();

    for (int i = 0; i < populationSize; i++) {
        IndividualNSGA individual = IndividualNSGA(
            rnd,
            numObjectives,
            numDepots,
            numAmbulances,
            numTimeSegments,
            mutationProbability,
            dayShift,
            false
        );
        individuals.push_back(individual);
    }

    evaluateObjectives();
}

void PopulationNSGA::evaluateObjectives() {
    for (IndividualNSGA& individual : individuals) {
        individual.evaluateObjectives(events, objectiveWeights);
    }
}

std::vector<IndividualNSGA> PopulationNSGA::parentSelection(int tournamentSize) {
    std::vector<IndividualNSGA> selectedParents;

    for (int i = 0; i < 2; i++) {
        std::vector<IndividualNSGA> tournament;
        for (int j = 0; j < tournamentSize; j++) {
            tournament.push_back(getRandomElement(rnd, individuals));
        }

        if (useFronts) {        // tournament selection based on rank and crowding distance
            auto best = std::min_element(tournament.begin(), tournament.end(),
                                        [](const IndividualNSGA &a, const IndividualNSGA &b) {
                                            // first, compare by rank (lower rank is better)
                                            if (a.getRank() != b.getRank()) {
                                                return a.getRank() < b.getRank();
                                            }
                                            // if ranks are equal, compare by crowding distance (higher is better)
                                            return a.getCrowdingDistance() > b.getCrowdingDistance();
                                        });
            selectedParents.push_back(*best);
        } else {
            auto best = std::min_element(tournament.begin(), tournament.end(),
                                        [](const IndividualNSGA &a, const IndividualNSGA &b) {
                                            // first, compare by rank (lower rank is better)
                                            if (a.getFitness() != b.getFitness()) {
                                                return a.getFitness() < b.getFitness();
                                            }
                                            return false;
                                        });
            selectedParents.push_back(*best);
        }
    }

    return selectedParents;
}

std::vector<IndividualNSGA> PopulationNSGA::survivorSelection(int numSurvivors) {
    std::vector<IndividualNSGA> nextGeneration;
    if (useFronts) {
        int currentFrontIndex = 0;

        while (nextGeneration.size() < numSurvivors && currentFrontIndex < fronts.size()) {
            auto& currentFront = fronts[currentFrontIndex];

            // Calculate crowding distance for all individuals in the current front
            calculateCrowdingDistance(currentFront);

            // Remove individuals with empty genotypes from the current front
            currentFront.erase(
                std::remove_if(currentFront.begin(), currentFront.end(),
                            [](const IndividualNSGA* individual) {
                                return individual->getGenotype().empty();
                            }),
                currentFront.end());

            // If adding the entire front does not exceed numSurvivors, add all; otherwise, add as many as needed
            if (nextGeneration.size() + currentFront.size() <= numSurvivors) {
                for (auto* indPtr : currentFront) {
                    nextGeneration.push_back(*indPtr);  // Dereference the pointer to get the IndividualNSGA
                }
            } else {
                // Need to add partially from the current front based on crowding distance
                std::sort(currentFront.begin(), currentFront.end(), [](const IndividualNSGA* a, const IndividualNSGA* b) {
                    return a->getCrowdingDistance() > b->getCrowdingDistance();  // Prefer higher crowding distance
                });
                int slotsLeft = numSurvivors - nextGeneration.size();
                int i = 0;
                while (slotsLeft > 0 && i < currentFront.size()) {
                    nextGeneration.push_back(*currentFront[i]);  // Add individuals until slots are filled
                    slotsLeft--;
                    i++;
                }
            }

            currentFrontIndex++;
        }
    } else {
        // Select based on fitness
        std::sort(individuals.begin(), individuals.end(), [](const IndividualNSGA& a, const IndividualNSGA& b) {
            return a.getFitness() < b.getFitness();  // Assuming minimizing fitness
        });
        nextGeneration.assign(individuals.begin(), individuals.begin() + std::min(numSurvivors, static_cast<int>(individuals.size())));
    }

    return nextGeneration;
}

void PopulationNSGA::addChildren(const std::vector<IndividualNSGA>& children) {
    for (int i = 0; i < children.size(); i++) individuals.push_back(children[i]);
}

std::vector<IndividualNSGA> PopulationNSGA::crossover(const IndividualNSGA& parent1, const IndividualNSGA& parent2) {
    CrossoverType crossoverType = Settings::get<CrossoverType>("CROSSOVER");
    switch(crossoverType) {
        case CrossoverType::SINGLE_POINT:
          return singlePointCrossover(parent1, parent2);
        default:
           return singlePointCrossover(parent1, parent2);
    }
}

std::vector<IndividualNSGA> PopulationNSGA::singlePointCrossover(const IndividualNSGA& parent1, const IndividualNSGA& parent2) {
    // initialize offspring genotypes to respective parents
    std::vector<std::vector<int>> offspring1Genotype = parent1.getGenotype();
    std::vector<std::vector<int>> offspring2Genotype = parent2.getGenotype();

    // iterate over each time segment
    for (size_t t = 0; t < offspring1Genotype.size(); ++t) {
        // generate random midpoint for the current time segment's allocation
        std::uniform_int_distribution<> midpointDist(1, offspring1Genotype[t].size() - 2);
        size_t midpoint = midpointDist(rnd);

        // perform crossover around this randomly chosen midpoint for the current time segment
        for (size_t i = 0; i < offspring1Genotype[t].size(); ++i) {
            if (i <= midpoint) {
                offspring2Genotype[t][i] = parent1.getGenotype()[t][i];
            } else {
                offspring1Genotype[t][i] = parent2.getGenotype()[t][i];
            }
        }
    }

    IndividualNSGA offspring1 = IndividualNSGA(rnd, numObjectives, numDepots, numAmbulances, numTimeSegments, mutationProbability, dayShift, true);
    IndividualNSGA offspring2 = IndividualNSGA(rnd, numObjectives, numDepots, numAmbulances, numTimeSegments, mutationProbability, dayShift, true);

    offspring1.setGenotype(offspring1Genotype);
    offspring2.setGenotype(offspring2Genotype);

    // repair, mutate, and evaluate fitness for each offspring
    std::vector<IndividualNSGA> offspring = {offspring1, offspring2};
    for (auto& child : offspring) {
        child.repair();
        child.mutate();
        child.evaluateObjectives(events, objectiveWeights);
    }

    return offspring;
}

void PopulationNSGA::calculateCrowdingDistance(std::vector<IndividualNSGA*>& front) {
    if (front.empty()) return;  // Guard against empty fronts

    size_t numObjectives = front[0]->getObjectives().size();
    // Initialize crowding distance for all individuals in the front
    for (IndividualNSGA* individual : front) {
        individual->setCrowdingDistance(0.0);
    }

    for (size_t i = 0; i < numObjectives; ++i) {
        // Sort individuals in the front based on the i-th objective
        std::sort(front.begin(), front.end(), [i](const IndividualNSGA* a, const IndividualNSGA* b) {
            return a->getObjectives()[i] < b->getObjectives()[i];
        });

        // Assign infinity distance to boundary individuals
        front.front()->setCrowdingDistance(std::numeric_limits<double>::infinity());
        front.back()->setCrowdingDistance(std::numeric_limits<double>::infinity());

        // Calculate normalized distance for intermediate individuals
        double objectiveRange = front.back()->getObjectives()[i] - front.front()->getObjectives()[i];
        if (objectiveRange > 0) {  // Avoid division by zero
            for (size_t j = 1; j < front.size() - 1; ++j) {
                double distance = (front[j + 1]->getObjectives()[i] - front[j - 1]->getObjectives()[i]) / objectiveRange;
                front[j]->setCrowdingDistance(front[j]->getCrowdingDistance() + distance);
            }
        }
    }
}

void PopulationNSGA::fastNonDominatedSort() {
    // clear domination values
    for (IndividualNSGA& ind : individuals) {
        ind.clearDominatedIndividuals();
        ind.setRank(-1);
        ind.clearDominationCount();
    }

    // clear them fronts and initialize first front
    std::vector<std::vector<IndividualNSGA*>> newFronts;
    std::unordered_set<IndividualNSGA*> addedIndividuals;
    fronts.clear();
    newFronts.emplace_back();

    // find undominated individuals of rank 0
    for (size_t i = 0; i < individuals.size(); i++) {
        for (size_t j = 0; j < individuals.size(); j++) {
            if (i == j) continue;
            if (individuals[i].dominates(individuals[j])) {
                individuals[i].nowDominates(&individuals[j]);
            } else if (individuals[j].dominates(individuals[i])) {
                individuals[i].incrementDominationCount();
            }
        }
        // if not dominated by anyone, set rank 0
        if (individuals[i].getDominationCount() == 0) {
            individuals[i].setRank(0);
            newFronts[0].push_back(&individuals[i]);
            addedIndividuals.insert(&individuals[i]);
        }
    }

    // construct subsequent fronts
    int currentFrontIndex = 0;
    while (currentFrontIndex < newFronts.size() && !newFronts[currentFrontIndex].empty()) {
        std::vector<IndividualNSGA*> nextFrontCandidates;
        // iterate over each individual in the current front
        for (auto* dominatingIndividual : newFronts[currentFrontIndex]) {
            // process each individual that the current dominatingIndividual dominates
            for (auto* dominatedIndividual : dominatingIndividual->getDominatedIndividuals()) {
                // decrement the domination count of the dominated individual
                dominatedIndividual->decrementDominationCount();
                // check if this individual should move to the next front
                if (dominatedIndividual->getDominationCount() == 0 && addedIndividuals.find(dominatedIndividual) == addedIndividuals.end()) {
                    dominatedIndividual->setRank(currentFrontIndex + 1);
                    nextFrontCandidates.push_back(dominatedIndividual);
                    addedIndividuals.insert(dominatedIndividual);
                }
            }
        }
        // move to the next front index for the next iteration
        currentFrontIndex++;
        // if there are candidates for the next front, add them to newFronts
        if (!nextFrontCandidates.empty()) {
            newFronts.push_back(nextFrontCandidates);
        }
    }

    // remove any remaining empty fronts from newFronts
    newFronts.erase(std::remove_if(newFronts.begin(), newFronts.end(),
                                   [](const std::vector<IndividualNSGA*>& front) { return front.empty(); }),
                    newFronts.end());

    // update the population's fronts with newFronts
    fronts = std::move(newFronts);

    // update individuals' rank based on front position
    for (size_t currentFrontIndex = 0; currentFrontIndex < fronts.size(); ++currentFrontIndex) {
        for (auto* individual : fronts[currentFrontIndex]) {
            individual->setRank(currentFrontIndex);
        }
    }
}

void PopulationNSGA::evolve(int generations) {
    ProgressBar progressBar(generations, "Running NSGA");

    if (!useFronts) {
        std::fill(objectiveWeights.begin(), objectiveWeights.end(), 1.0f);
    }

    for (int gen = 0; gen < generations; ++gen) {
        // step 1: sort the population into Pareto fronts
        if (useFronts) {
            fastNonDominatedSort();

        // step 2: calculate crowding distance within each front
            for (auto& front : fronts) {
                calculateCrowdingDistance(front);
            }
        }
        // step 3: selection, crossover and mutation to create a new offspring pool
        std::vector<IndividualNSGA> offspring;
        int tournamentSize = 3;
        std::uniform_real_distribution<> shouldCrossover(0.0, 1.0);

        while (offspring.size() < populationSize) {
            if (shouldCrossover(rnd) < crossoverProbability) {
                std::vector<IndividualNSGA> parents = parentSelection(tournamentSize);
                std::vector<IndividualNSGA> children = crossover(parents[0], parents[1]);

                // calculate how many children can be added without exceeding populationSize
                size_t spaceLeft = populationSize - offspring.size();
                size_t childrenToAdd = std::min(children.size(), spaceLeft);

                // add children directly to offspring, ensuring not to exceed populationSize
                offspring.insert(offspring.end(), children.begin(), children.begin() + childrenToAdd);
            }
        }

        // step 4: combine, sort, and select the next generation from parents and offspring
        addChildren(offspring);
        if (useFronts) {
            fastNonDominatedSort();
        }
        individuals = survivorSelection(populationSize);
        progressBar.update(gen + 1);
    }

    if (useFronts) {
        fastNonDominatedSort();  // Re-sort combined population
    }

    // get best individual
    IndividualNSGA finalIndividual = findFittest();

    // write metrics to file
    writeMetrics(Settings::get<std::string>("UNIQUE_RUN_ID"), finalIndividual.getSimulatedEvents());

    printBestScoresForEachObjective();
}

int PopulationNSGA::countUnique() {
    std::vector<std::string> genotypeStrings;
    genotypeStrings.reserve(individuals.size());

    for (const auto& individual : individuals) {
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

const IndividualNSGA& PopulationNSGA::findFittest() const {
    if (useFronts) {
        if (fronts.empty() || fronts.front().empty()) {
            throw std::runtime_error("No individuals in the population or the first front is empty.");
        }

        const auto& firstFront = fronts.front();

        auto fittest = std::max_element(firstFront.begin(), firstFront.end(),
                                        [](const IndividualNSGA* a, const IndividualNSGA* b) {
                                            return a->getCrowdingDistance() > b->getCrowdingDistance();
                                        });

        return **fittest;
    } else {
        auto fittest = std::max_element(
            individuals.begin(),
            individuals.end(),
            [](const IndividualNSGA &a, const IndividualNSGA &b) {
                return a.getFitness() > b.getFitness();
            }
        );
        return *fittest;
    }
}

const IndividualNSGA PopulationNSGA::findLeastFit() {
    const auto& lastFront = fronts.back();  // Assuming the last front contains the least fit individuals

    auto leastFit = std::min_element(lastFront.begin(), lastFront.end(),
                                     [](const IndividualNSGA* a, const IndividualNSGA* b) {
                                         // Compare by crowding distance, assuming lower is less fit for the purpose of this method
                                         return a->getCrowdingDistance() < b->getCrowdingDistance();
                                     });

    // Dereference the iterator to get the pointer, then return the object it points to
    return **leastFit;
}

void PopulationNSGA::checkEmptyGenotypes() {
    std::cout << "Checking for empty genotypes in the entire population..." << std::endl;
    // Check entire population
    for (const IndividualNSGA& individual : individuals) {
        if (individual.getGenotype().empty()) {
            std::cout << "Empty genotype found in the population!" << std::endl;
            throw std::runtime_error("Empty genotype encountered in the population. Terminating program.");
        }
    }
    std::cout << "No empty genotypes in the population." << std::endl;

    // Now, check each front
    std::cout << "Checking for empty genotypes in each front..." << std::endl;
    for (const auto& front : fronts) {
        for (const IndividualNSGA* individual : front) {
            if (individual->getGenotype().empty()) {
                std::cout << "Empty genotype found in a front!" << std::endl;
                throw std::runtime_error("Empty genotype encountered in a front. Terminating program.");
            }
        }
    }
    std::cout << "No empty genotypes in any front." << std::endl;
}


void PopulationNSGA::printPopulationInfo() {
    std::cout << "PopulationNSGA information:" << std::endl;
    std::cout << "Number of individuals: " << individuals.size() << std::endl;
    std::cout << "Number of fronts: " << fronts.size() << std::endl;

    for (size_t i = 0; i < fronts.size(); ++i) {
        std::cout << "Front " << i << " has " << fronts[i].size() << " members." << std::endl;
    }
}
void PopulationNSGA::printBestScoresForEachObjective() const {
    std::cout << "\n";

    if (useFronts) {
        if (fronts.empty() || fronts.front().empty()) {
            std::cerr << "No non-dominated individuals available in fronts." << std::endl;
            return;
        }

        const auto& firstFront = fronts.front();
        std::vector<double> bestScores(numObjectives, std::numeric_limits<double>::max());
        std::vector<int> bestIndividualIndices(numObjectives, -1);

        for (int objective = 0; objective < numObjectives; ++objective) {
            for (int i = 0; i < firstFront.size(); ++i) {
                const auto& individualObjectives = firstFront[i]->getObjectives();
                if (individualObjectives[objective] < bestScores[objective]) {
                    bestScores[objective] = individualObjectives[objective];
                    bestIndividualIndices[objective] = i;
                }
            }
        }

        for (int objective = 0; objective < numObjectives; ++objective) {
            std::cout << "Front Objective " << objective << ": Best Score = "
                      << bestScores[objective]/objectiveWeights[objective] << ", IndividualNSGA Index = "
                      << bestIndividualIndices[objective] << std::endl;
        }
    } else {
        if (individuals.empty()) {
            std::cerr << "No individuals available." << std::endl;
            return;
        }

        std::vector<double> bestScores(numObjectives, std::numeric_limits<double>::max());
        std::vector<int> bestIndividualIndices(numObjectives, -1);

        for (int objective = 0; objective < numObjectives; ++objective) {
            for (int i = 0; i < individuals.size(); ++i) {
                const auto& individualObjectives = individuals[i].getObjectives();
                if (individualObjectives[objective] < bestScores[objective]) {
                    bestScores[objective] = individualObjectives[objective];
                    bestIndividualIndices[objective] = i;
                }
            }
        }

        // Print the best scores for each objective
        for (int objective = 0; objective < numObjectives; ++objective) {
            std::cout << "Objective " << objective << ": Best Score = "
                      << bestScores[objective]/objectiveWeights[objective] << ", IndividualNSGA Index = "
                      << bestIndividualIndices[objective] << std::endl;
        }

        // Additionally, print total fitness if available
        auto bestIndividual = std::min_element(individuals.begin(), individuals.end(),
                                               [](const IndividualNSGA& a, const IndividualNSGA& b) {
                                                   return a.getFitness() < b.getFitness();
                                               });

        if (bestIndividual != individuals.end()) {
            std::cout << "Total Fitness: Best Score = " << bestIndividual->getFitness() << std::endl;
        }
    }
}


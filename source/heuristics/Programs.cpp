/**
 * @file Programs.cpp
 *
 * @copyright Copyright (c) 2024 Emergency-Optimizers
 */

/* external libraries */
#include <iostream>
#include <fstream>
#include <sstream>
#include <iomanip>
#include <algorithm>
#include <numeric>
#include <chrono>
#include <filesystem>
/* internal libraries */
#include "heuristics/Programs.hpp"
#include "Utils.hpp"
#include "heuristics/PopulationGA.hpp"
#include "heuristics/PopulationNSGA2.hpp"
#include "heuristics/PopulationMA.hpp"
#include "heuristics/PopulationMemeticNSGA2.hpp"
#include "Constants.hpp"
#include "file-reader/Settings.hpp"
#include "file-reader/ODMatrix.hpp"
#include "file-reader/Incidents.hpp"
#include "file-reader/Stations.hpp"
#include "simulator/AmbulanceAllocator.hpp"
#include "simulator/Simulator.hpp"
#include "simulator/strategies/DispatchEngineStrategyType.hpp"
#include "simulator/MonteCarloSimulator.hpp"

void runSimulatorOnce(
    std::vector<Event>& events,
    const bool verbose,
    const bool saveToFile,
    std::vector<std::vector<int>> allocations,
    std::string extraFileName
) {
    // set allocation
    if (allocations.empty()) {
        allocations.push_back({2, 3, 2, 2, 2, 4, 2, 3, 3, 4, 4, 4, 4, 3, 3});

        if (Settings::get<bool>("SIMULATE_DAY_SHIFT")) {
            int allocationSize = static_cast<int>(allocations.size());
            for (int allocationIndex = 0; allocationIndex < allocationSize; allocationIndex++) {
                allocations[allocationIndex].push_back(0);
                allocations[allocationIndex].push_back(0);
                allocations[allocationIndex].push_back(0);
                allocations[allocationIndex].push_back(0);
            }
        }
    }

    AmbulanceAllocator ambulanceAllocator;
    ambulanceAllocator.allocate(
        events,
        allocations,
        Settings::get<bool>("SIMULATE_DAY_SHIFT")
    );

    // simulate events
    Simulator simulator(
        ambulanceAllocator,
        Settings::get<DispatchEngineStrategyType>("DISPATCH_STRATEGY"),
        events
    );
    std::vector<Event> simulatedEvents;
    simulatedEvents = simulator.run();

    if (saveToFile) {
        // write events to file
        const std::string dirName = Settings::get<std::string>("UNIQUE_RUN_ID") + "_NONE";

        writeEvents(dirName, simulatedEvents, "events" + extraFileName);
        writeGenotype(dirName, allocations, "genotype" + extraFileName);
        writeAmbulances(dirName, ambulanceAllocator.ambulances, "ambulances" + extraFileName);
    }

    if (verbose) {
        // print metrics
        double avgResponseTimeAUrban = averageResponseTime(simulatedEvents, "A", true);
        double avgResponseTimeANonurban = averageResponseTime(simulatedEvents, "A", false);
        double avgResponseTimeHUrban = averageResponseTime(simulatedEvents, "H", true);
        double avgResponseTimeHNonurban = averageResponseTime(simulatedEvents, "H", false);
        double avgResponseTimeV1Urban = averageResponseTime(simulatedEvents, "V1", true);
        double avgResponseTimeV1Nonurban = averageResponseTime(simulatedEvents, "V1", false);

        printAmbulanceWorkload(ambulanceAllocator.ambulances);

        std::cout
            << "\nGoal:" << std::endl
            << "\t A, urban: <12 min" << std::endl
            << "\t A, non-urban: <25 min" << std::endl
            << "\t H, urban: <30 min" << std::endl
            << "\t H, non-urban: <40 min" << std::endl
            << std::endl
            << "Avg. response time (A, urban): \t\t" << avgResponseTimeAUrban << "s (" << avgResponseTimeAUrban / 60 << "m)" << std::endl
            << "Avg. response time (A, non-urban): \t" << avgResponseTimeANonurban << "s (" << avgResponseTimeANonurban / 60 << "m)" << std::endl
            << "Avg. response time (H, urban): \t\t" << avgResponseTimeHUrban << "s (" << avgResponseTimeHUrban / 60 << "m)" << std::endl
            << "Avg. response time (H, non-urban): \t" << avgResponseTimeHNonurban << "s (" << avgResponseTimeHNonurban / 60 << "m)" << std::endl
            << "Avg. response time (V1, urban): \t" << avgResponseTimeV1Urban << "s (" << avgResponseTimeV1Urban / 60 << "m)" << std::endl
            << "Avg. response time (V1, non-urban): \t" << avgResponseTimeV1Nonurban << "s (" << avgResponseTimeV1Nonurban / 60 << "m)" << std::endl
            << "Percentage violations: \t\t\t" << responseTimeViolations(simulatedEvents) * 100 << "%" << std::endl
            << "Percentage violations (U): \t\t" << responseTimeViolationsUrban(simulatedEvents, true) * 100 << "%" << std::endl
            << "Percentage violations (R): \t\t" << responseTimeViolationsUrban(simulatedEvents, false) * 100 << "%" << std::endl;
    }
}

void runGeneticAlgorithm(const std::vector<Event>& events) {
    PopulationGA population(events);
    population.evolve();
}

void runNSGA2(const std::vector<Event>& events) {
    PopulationNSGA2 population(events);
    population.evolve();
}

void runMemeticAlgorithm(const std::vector<Event>& events) {
    PopulationMA population(events);
    population.evolve();
}

void runMemeticNSGA2(const std::vector<Event>& events) {
    PopulationMemeticNSGA2 population(events);
    population.evolve();
}

void runTimeEvaluation() {
    const bool verbose = false;
    const bool saveToFile = false;
    const int N = 10;

    std::vector<int> possibleTimeSegments = { 1, 6, 12, 18, 24 };

    std::vector<DispatchEngineStrategyType> possibleStrategies = {
        DispatchEngineStrategyType::CLOSEST,
        DispatchEngineStrategyType::RANDOM,
    };

    // generate events
    MonteCarloSimulator monteCarloSim;
    std::vector<Event> events = monteCarloSim.generateEvents();

    for (auto& strategy : possibleStrategies) {
        for (auto& timeSegments : possibleTimeSegments) {
            Settings::update<int>("NUM_TIME_SEGMENTS", timeSegments);
            Settings::update<DispatchEngineStrategyType>("DISPATCH_STRATEGY", strategy);

            const bool dayShift = Settings::get<bool>("SIMULATE_DAY_SHIFT");
            const int numDepots = static_cast<int>(Stations::getInstance().getDepotIndices(dayShift).size());
            const int numAmbulances = dayShift ?
                Settings::get<int>("TOTAL_AMBULANCES_DURING_DAY") : Settings::get<int>("TOTAL_AMBULANCES_DURING_NIGHT");

            int64_t sumDurations = 0LL;
            for (int i = 0; i < N; ++i) {
                // generate random allocation
                std::mt19937 rnd(i);

                std::vector<std::vector<int>> allocations(timeSegments, std::vector<int>(numDepots, 0));
                for (int allocationIndex = 0; allocationIndex < timeSegments; allocationIndex++) {
                    for (int ambulanceIndex = 0; ambulanceIndex < numAmbulances; ambulanceIndex++) {
                        int depotIndex = getRandomInt(rnd, 0, numDepots - 1);

                        allocations[allocationIndex][depotIndex]++;
                    }
                }

                // run simulator and clock the run time
                std::vector<Event> copiedEvents = events;

                auto startClock = std::chrono::steady_clock::now();
                runSimulatorOnce(copiedEvents, verbose, saveToFile, allocations);
                auto endClock = std::chrono::steady_clock::now();

                sumDurations += std::chrono::duration_cast<std::chrono::milliseconds>(endClock - startClock).count();
            }

            std::cout
                << "Time Segments: " << timeSegments << ", "
                << "Strategy: " << (strategy == DispatchEngineStrategyType::RANDOM ? "RANDOM" : "CLOSEST") << " "
                << "= \t" << std::to_string(static_cast<double>(sumDurations) / static_cast<double>(N)) << " avg milliseconds"
                << " (N = " << N << ")" << std::endl;
        }
    }
    std::cout << std::endl;
}

void runDataValidation(std::vector<Event>& events) {
    for (int i = 1; i <= 12; i++) {
        const std::string dirName = Settings::get<std::string>("UNIQUE_RUN_ID") + "_CUSTOM_" + std::to_string(i);

        Settings::update<int>("SIMULATE_MONTH", i);

        MonteCarloSimulator monteCarloSim;
        events = monteCarloSim.generateEvents();

        for (auto& event : events) {
            if (event.utility) {
                continue;
            }

            event.updateTimer(static_cast<int>(event.secondsWaitResourcePreparingDeparture), "duration_resource_preparing_departure");

            const bool cancelledEvent = event.secondsWaitDepartureScene == -1;
            if (!cancelledEvent) {
                event.updateTimer(static_cast<int>(event.secondsWaitDepartureScene), "duration_at_scene");
                event.updateTimer(static_cast<int>(event.secondsWaitAvailable), "duration_at_hospital");
            } else {
                event.updateTimer(static_cast<int>(event.secondsWaitAvailable), "duration_at_scene");
            }
        }

        writeEvents(dirName, events);
    }
}

void runSimulationGridSearch(const std::vector<Event>& events) {
    const bool verbose = false;
    const bool saveToFile = true;

    // ...
    std::vector<DispatchEngineStrategyType> possibleStrategies = {
        DispatchEngineStrategyType::CLOSEST,
        DispatchEngineStrategyType::RANDOM,
    };

    std::vector<bool> possiblePrioritizeTriage = {
        false,
        true,
    };

    std::vector<bool> possibleResponseRestricated = {
        false,
        true,
    };

    std::vector<bool> possibleScheduleBreaks = {
        false,
        true,
    };

    for (auto& strategy : possibleStrategies) {
        for (auto prioritizeTriage : possiblePrioritizeTriage) {
            for (auto responseRestricated : possibleResponseRestricated) {
                for (auto scheduleBreaks : possibleScheduleBreaks) {
                    Settings::update<DispatchEngineStrategyType>("DISPATCH_STRATEGY", strategy);
                    Settings::update<bool>("DISPATCH_STRATEGY_PRIORITIZE_TRIAGE", prioritizeTriage);
                    Settings::update<bool>("DISPATCH_STRATEGY_RESPONSE_RESTRICTED", responseRestricated);
                    Settings::update<bool>("SCHEDULE_BREAKS", scheduleBreaks);

                    std::string extraFileName = "_strategy=";
                    extraFileName += strategy == DispatchEngineStrategyType::CLOSEST ? "closest" : "random";

                    extraFileName += "_prioritizeTriage=";
                    extraFileName += prioritizeTriage ? "true" : "false";

                    extraFileName += "_responseRestricted=";
                    extraFileName += responseRestricated ? "true" : "false";

                    extraFileName += "_scheduleBreaks=";
                    extraFileName += scheduleBreaks ? "true" : "false";

                    std::vector<Event> copiedEvents = events;
                    runSimulatorOnce(copiedEvents, verbose, saveToFile, {}, extraFileName);
                }
            }
        }
    }
}

void runExperimentTimeSegments(const std::vector<Event>& events) {
    std::vector<int> possibleTimeSegments(24, 0);
    std::iota(possibleTimeSegments.begin(), possibleTimeSegments.end(), 1);

    for (auto timeSegments : possibleTimeSegments) {
        Settings::update<int>("NUM_TIME_SEGMENTS", timeSegments);

        std::vector<Event> copiedEvents = events;

        PopulationGA population(copiedEvents);
        population.evolve(false, "_ts=" + std::to_string(timeSegments));
        std::cout << std::endl;
    }
}

void runExtremeConditionTest() {
    const bool verbose = false;
    const bool saveToFile = true;

    std::vector<double> possibleIncidentsToGenerateFactors = {
        0.50,
        1.00,
        1.50
    };

    for (auto incidentsToGenerateFactor : possibleIncidentsToGenerateFactors) {
        Settings::update<double>("INCIDENTS_TO_GENERATE_FACTOR", incidentsToGenerateFactor);

        MonteCarloSimulator monteCarloSim;
        std::vector<Event> events = monteCarloSim.generateEvents();

        std::string extraFileName = "_numIncidentsFactor=" + std::to_string(incidentsToGenerateFactor);

        runSimulatorOnce(events, verbose, saveToFile, {}, extraFileName);
    }
}

void runAmbulanceExperiment(const std::vector<Event>& events) {
    const bool verbose = false;

    std::vector<int> possibleResourceSize;
    for (int i = 30; i <= 60; i++) {
        possibleResourceSize.push_back(i);
    }

    for (auto resourceSize : possibleResourceSize) {
        Settings::update<int>("TOTAL_AMBULANCES_DURING_DAY", resourceSize);
        std::string extraFileName = "_numAmbulances=" + std::to_string(resourceSize);

        std::vector<Event> copiedEvents = events;

        // change to correct heuristic
        PopulationNSGA2 population(copiedEvents);
        population.evolve(verbose, extraFileName);

        std::cout << std::endl;
    }
}

void runExperimentHeuristics(const std::vector<Event>& events) {
    const bool verbose = false;

    std::string heuristic = Settings::get<std::string>("CUSTOM_STRING_VALUE");

    std::vector<int> possibleSeeds(10, 0);
    std::iota(possibleSeeds.begin(), possibleSeeds.end(), 0);

    for (auto seed : possibleSeeds) {
        Settings::update<int>("SEED", seed);

        std::vector<Event> copiedEvents = events;
        std::string extraFileName = "_seed=" + std::to_string(seed);

        if (heuristic == "GA") {
            PopulationGA population(copiedEvents);
            population.evolve(verbose, extraFileName);
        } else if (heuristic == "NSGA2") {
            PopulationNSGA2 population(copiedEvents);
            population.evolve(verbose, extraFileName);
        } else if (heuristic == "MA") {
            PopulationMA population(copiedEvents);
            population.evolve(verbose, extraFileName);
        } else if (heuristic == "MEMETIC_NSGA2") {
            PopulationMemeticNSGA2 population(copiedEvents);
            population.evolve(verbose, extraFileName);
        } else {
            throwError("Unknown CUSTOM_STRING_VALUE");
        }

        std::cout << std::endl;
    }
}

void runExperimentAllocations(const std::vector<Event>& events) {
    const bool verbose = false;
    const bool saveToFile = true;

    std::vector<int> possibleSeeds(10, 0);
    std::iota(possibleSeeds.begin(), possibleSeeds.end(), 0);

    std::map<std::string, std::vector<std::vector<int>>> possibleAllocations = {
        {"ACC", {{0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 45, 0, 0, 0, 0, 0, 0, 0}}},
        {"U", {{3, 3, 3, 3, 3, 3, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2}}},
        {"PP", {{1, 1, 1, 1, 3, 1, 2, 2, 3, 4, 3, 5, 3, 2, 2, 3, 3, 2, 3}}},
        {"SLS", {{2, 1, 1, 2, 2, 3, 1, 2, 2, 3, 3, 5, 2, 2, 2, 2, 2, 4, 2}}},
        {"GA", {{2, 3, 1, 2, 3, 1, 2, 2, 1, 3, 4, 4, 3, 2, 3, 3, 3, 2, 1}}},
        {"MA", {{2, 1, 1, 3, 2, 1, 0, 2, 3, 3, 3, 2, 3, 4, 5, 2, 2, 4, 3}}},
        {"OUH", {{2, 3, 2, 2, 2, 4, 2, 3, 3, 4, 4, 4, 4, 3, 3, 0, 0, 0, 0}}},
    };

    for (auto allocationInfo : possibleAllocations) {
        std::string allocationName = allocationInfo.first;
        std::vector<std::vector<int>> allocation = allocationInfo.second;

        std::cout << allocationName << std::endl;

        for (auto seed : possibleSeeds) {
            Settings::update<int>("SEED", seed);

            std::vector<Event> copiedEvents = events;
            std::string extraFileName = "_" + allocationName + "_seed=" + std::to_string(seed);

            runSimulatorOnce(copiedEvents, verbose, saveToFile, allocation, extraFileName);
        }
    }
}

void runExperimentCustomAllocations(const std::vector<Event>& events) {
    /*const bool verbose = false;
    const bool saveToFile = true;

    Settings::update<int>("NUM_TIME_SEGMENTS", 1);

    std::vector<int> possibleSeeds(10, 0);
    std::iota(possibleSeeds.begin(), possibleSeeds.end(), 0);

    std::vector<std::string> possibleAllocations {"R", "UR", "IP"};

    for (auto allocationName : possibleAllocations) {
        std::cout << allocationName << std::endl;

        for (auto seed : possibleSeeds) {
            Settings::update<int>("SEED", seed);

            std::mt19937 rd(seed);

            Individual ind(
                rd,
                45,
                1,
                19,
                true,
                true,
                {},
                {}
            );

            std::vector<std::vector<int>> allocation = {};

            if (allocationName == "R") {
                ind.randomGenotype();
            } else if (allocationName == "UR") {
                ind.uniformGenotype();
            } else if (allocationName == "IP") {
                ind.proportionateGenotype("total_incidents_cluster", true);
            }

            allocation = ind.genotype;

            std::vector<Event> copiedEvents = events;
            std::string extraFileName = "_" + allocationName + "_seed=" + std::to_string(seed);

            runSimulatorOnce(copiedEvents, verbose, saveToFile, allocation, extraFileName);
        }
    }*/
}

void runExperimentDepots(const std::vector<Event>& events) {
    const bool verbose = false;

    std::string heuristic = Settings::get<std::string>("CUSTOM_STRING_VALUE");

    std::vector<int> possibleSeeds(5, 0);
    std::iota(possibleSeeds.begin(), possibleSeeds.end(), 0);

    std::vector<int> possibleDepotToRemove(20, 0);
    std::iota(possibleDepotToRemove.begin(), possibleDepotToRemove.end(), -1);

    for (auto depotToRemove : possibleDepotToRemove) {
        for (auto seed : possibleSeeds) {
            Settings::update<int>("SEED", seed);
            Settings::update<int>("SKIP_STATION_INDEX", depotToRemove);

            std::vector<Event> copiedEvents = events;
            std::string extraFileName = "_depot=" + std::to_string(depotToRemove) + "_seed=" + std::to_string(seed);

            PopulationNSGA2 population(copiedEvents);
            population.evolve(verbose, extraFileName);

            std::cout << std::endl;
        }
        std::cout << std::endl;
    }
}

void runExperimentTimeSegmentsVerification(const std::vector<Event>& events) {
    std::vector<int> possibleSeeds(10, 0);
    std::iota(possibleSeeds.begin(), possibleSeeds.end(), 0);

    std::vector<int> possibleTimeSegments = { 1, 4 };

    for (auto timeSegments : possibleTimeSegments) {
        for (auto seed : possibleSeeds) {
            Settings::update<int>("SEED", seed);
            Settings::update<int>("NUM_TIME_SEGMENTS", timeSegments);

            std::vector<Event> copiedEvents = events;
            std::string extraFileName = "_ts=" + std::to_string(timeSegments) + "_seed=" + std::to_string(seed);

            PopulationNSGA2 population(copiedEvents);
            population.evolve(false, extraFileName);

            std::cout << std::endl;
        }
        std::cout << std::endl;
    }
}

void runExperimentPrediction(const std::vector<Event>& events) {
    const bool verbose = false;

    std::vector<int> possibleSeeds(10, 0);
    std::iota(possibleSeeds.begin(), possibleSeeds.end(), 0);

    for (auto seed : possibleSeeds) {
        Settings::update<int>("SEED", seed);

        std::vector<Event> copiedEvents = events;
        std::string extraFileName = "_seed=" + std::to_string(seed);

        PopulationNSGA2 population(copiedEvents);
        population.evolve(verbose, extraFileName);

        std::cout << std::endl;
    }
}

void runSimulationMultipleTimes(const std::vector<Event>& events) {
    const bool verbose = false;
    const bool saveToFile = true;

    std::vector<int> possibleSeeds(10, 0);
    std::iota(possibleSeeds.begin(), possibleSeeds.end(), 0);

    for (auto seed : possibleSeeds) {
        Settings::update<int>("SEED", seed);

        std::vector<Event> copiedEvents = events;
        std::string extraFileName = "_seed=" + std::to_string(seed);

        runSimulatorOnce(copiedEvents, verbose, saveToFile, {}, extraFileName);
    }
}

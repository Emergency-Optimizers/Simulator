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

void runSimulatorOnce(std::vector<Event>& events, const bool verbose, std::vector<std::vector<int>> allocations) {
    // set allocation
    if (allocations.empty()) {
        allocations.push_back({2, 4, 2, 2, 2, 4, 2, 3, 3, 3, 3, 5, 4, 3, 3});

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

    if (verbose) {
        // write events to file
        const std::string dirName = Settings::get<std::string>("UNIQUE_RUN_ID") + "_NONE";

        writeEvents(dirName, simulatedEvents);
        writeGenotype(dirName, allocations);
        writeAmbulances(dirName, ambulanceAllocator.ambulances);
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
            << "Percentage violations: \t\t\t" << responseTimeViolations(simulatedEvents) * 100 << "%" << std::endl;
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

void runGridSearch1(const std::vector<Event>& events) {
    const bool verbose = false;
    const int N = 10;

    // ...
    std::vector<int> possibleTimeSegments(24, 0);
    std::iota(possibleTimeSegments.begin(), possibleTimeSegments.end(), 1);

    std::vector<DispatchEngineStrategyType> possibleStrategies = {
        DispatchEngineStrategyType::CLOSEST,
        DispatchEngineStrategyType::RANDOM,
    };

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

                auto startClock = std::chrono::high_resolution_clock::now();
                runSimulatorOnce(copiedEvents, verbose, allocations);
                auto endClock = std::chrono::high_resolution_clock::now();

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

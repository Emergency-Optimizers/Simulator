/**
 * @file Programs.hpp
 *
 * @copyright Copyright (c) 2024 Emergency-Optimizers
 */

#pragma once

/* external libraries */
#include <vector>
#include <string>
/* internal libraries */
#include "simulator/Event.hpp"

void runSimulatorOnce(
    std::vector<Event>& events,
    const bool verbose = true,
    const bool saveToFile = true,
    std::vector<std::vector<int>> allocations = {},
    std::string extraFileName = ""
);
void runGeneticAlgorithm(const std::vector<Event>& events);
void runNSGA2(const std::vector<Event>& events);
void runMemeticAlgorithm(const std::vector<Event>& events);
void runMemeticNSGA2(const std::vector<Event>& events);
void runTimeEvaluation();
void runDataValidation(std::vector<Event>& events);
void runSimulationGridSearch(const std::vector<Event>& events);
void runExperimentTimeSegments(const std::vector<Event>& events);
void runExtremeConditionTest();

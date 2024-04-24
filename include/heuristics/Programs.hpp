/**
 * @file Programs.hpp
 *
 * @copyright Copyright (c) 2024 Emergency-Optimizers
 */

#pragma once

/* external libraries */
#include <vector>
/* internal libraries */
#include "simulator/Event.hpp"

void runSimulatorOnce(std::vector<Event>& events, const bool verbose = true, std::vector<std::vector<int>> allocations = {});
void runGeneticAlgorithm(const std::vector<Event>& events);
void runNSGA2(const std::vector<Event>& events);
void runMemeticAlgorithm(const std::vector<Event>& events);
void runMemeticNSGA2(const std::vector<Event>& events);
void runGridSearch1(const std::vector<Event>& events);

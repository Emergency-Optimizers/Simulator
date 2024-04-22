/**
 * @file PopulationMA.cpp
 *
 * @copyright Copyright (c) 2024 Emergency-Optimizers
 */

/* external libraries */
#include <string>
/* internal libraries */
#include "heuristics/PopulationMemeticNSGA2.hpp"

PopulationMemeticNSGA2::PopulationMemeticNSGA2(const std::vector<Event>& events) :
    PopulationGA(events),
    PopulationNSGA2(events),
    PopulationMA(events) { }

const std::string PopulationMemeticNSGA2::getHeuristicName() const {
    return heuristicName;
}

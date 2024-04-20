/**
 * @file PopulationMA.cpp
 *
 * @copyright Copyright (c) 2024 Emergency-Optimizers
 */

/* external libraries */
#include <string>
/* internal libraries */
#include "heuristics/PopulationMemeticNSGA2.hpp"

PopulationMemeticNSGA2::PopulationMemeticNSGA2(
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
), PopulationNSGA2(
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
), PopulationMA(
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

const std::string PopulationMemeticNSGA2::getHeuristicName() const {
    return heuristicName;
}

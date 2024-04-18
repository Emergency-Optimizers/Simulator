/**
 * @file Utils.hpp
 *
 * @copyright Copyright (c) 2024 Emergency-Optimizers
 */

#pragma once

/* external libraries */
#include <string>
#include <optional>
#include <variant>
#include <vector>
#include <random>
#include <utility>
#include <unordered_map>
#include <map>
#include <ctime>
#ifdef _WIN32
#include <cstdlib>
#endif
/* internal libraries */
#include "simulator/Ambulance.hpp"
#include "simulator/Event.hpp"
#include "simulator/strategies/DispatchEngineStrategyType.hpp"
#include "heuristics/CrossoverType.hpp"
#include "heuristics/ObjectiveTypes.hpp"

using ValueType = std::variant<
    int,
    int64_t,
    float,
    double,
    std::string,
    bool,
    std::optional<std::tm>,
    std::vector<float>,
    DispatchEngineStrategyType,
    CrossoverType,
    std::vector<ObjectiveTypes>
>;
using ToValueType = ValueType(*)(const std::string&);
using SchemaMapping = std::unordered_map<std::string, ToValueType>;

ValueType toInt(const std::string& str);
ValueType toInt64(const std::string& str);
ValueType toFloat(const std::string& str);
ValueType toDouble(const std::string& str);
ValueType toString(const std::string& str);
ValueType toBool(const std::string& str);
ValueType toDateTime(const std::string& str);
ValueType toVectorFloat(const std::string& str);
ValueType toDispatchEngineStrategyType(const std::string& str);
ValueType toCrossoverType(const std::string& str);
ValueType toVectorObjectiveType(const std::string& str);
ObjectiveTypes stringToObjectiveType(const std::string& str);
std::string tmToString(const std::tm& time);
std::string valueTypeToString(const ValueType& cell);
double timeDifferenceInSeconds(std::tm& time1, std::tm& time2);
std::vector<unsigned> getAvailableAmbulanceIndicies(
    std::vector<Ambulance>& ambulances,
    const std::vector<Event>& events,
    const time_t& currentTime,
    const std::string& currentEventTriageImpression
);
int calculateDayDifference(const std::tm& baseDate, const int targetMonth, const int targetDay);
int weightedLottery(
    std::mt19937& rnd,
    const std::vector<double>& weights,
    const std::vector<std::pair<int, int>>& ranges = {}
);
int getRandomInt(std::mt19937& rnd, const int min, const int max);
double getRandomDouble(std::mt19937& rnd, const double min = 0.0, const double max = 1.0);
bool getRandomBool(std::mt19937& rnd);
double calculateMean(const std::vector<int>& numbers);
double calculateStandardDeviation(const std::vector<int>& numbers);
double calculateEuclideanDistance(const double x1, const double y1, const double x2, const double y2);
std::pair<int, int> idToUtm(const int64_t& grid_id);
int64_t utmToId(const std::pair<int, int>& utm, const int cellSize = 1000, const int offset = 2000000);
int64_t approximateLocation(
    std::mt19937& rnd,
    const int64_t& startId,
    const int64_t& goalId,
    const time_t& timeAtStart,
    const time_t& timeNow,
    const std::string& triage
);
int findEventIndexFromId(const std::vector<Event>& events, const int id);
void writeEvents(const std::string& dirName, std::vector<Event>& events);
void saveDistributionToFile(const std::vector<std::vector<double>>& distribution, const std::string& baseFilename);
void save1dDistributionToFile(const std::vector<double>& distribution, const std::string& baseFilename);
void save2dDistributionToFile(const std::vector<std::vector<double>>& distribution, const std::string& baseFilename);
bool isDayShift(const time_t& eventTimer, const int dayShiftStart, const int dayShiftEnd);
std::string eventTypeToString(EventType eventType);
double averageResponseTime(
    std::vector<Event>& simulatedEvents,
    const std::string& triageImpression,
    const bool urban,
    const int allocationIndex = -1,
    const int depotIndex = -1
);
double responseTimeViolations(
    std::vector<Event>& simulatedEvents,
    const int allocationIndex = -1,
    const int depotIndex = -1
);
double responseTimeViolationsUrban(
    std::vector<Event>& simulatedEvents,
    const bool checkUrban,
    const int allocationIndex = -1,
    const int depotIndex = -1
);
void printTimeSegmentedAllocationTable(
    const bool dayShift,
    const int numTimeSegments,
    const std::vector<std::vector<int>>& allocations,
    std::vector<Event>& simulatedEvents,
    const std::vector<double>& allocationsFitness
);
void printAmbulanceWorkload(const std::vector<Ambulance>& ambulances);
void throwError(const std::string& msg);
void saveDataToJson(
    const std::string& dirName,
    const std::string& fileName,
    const std::map<std::string, std::vector<std::vector<double>>>& dataMap
);
void createDirectory(const std::string& dirName);
double inverseFitness(const double fitness);
double gaussian_kernel(const double x, const double mu, const double sigma);

/**
 * Get a tm struct from a time_t value in a thread-safe manner.
 * This function works on both Windows and POSIX systems.
 *
 * @param time_val A time_t value representing the time.
 * @return A tm struct corresponding to the given time_t value.
 */
std::tm getLocalTime(const time_t& time_val);

template <typename T>
int findIndex(const std::vector<T>& vec, const T& value) {
    auto it = std::find(vec.begin(), vec.end(), value);

    if (it != vec.end()) {
        return static_cast<int>(std::distance(vec.begin(), it));
    } else {
        return -1;
    }
}

template <typename T>
T getRandomElement(std::mt19937& rnd, const std::vector<T>& vec) {
    std::uniform_int_distribution<int> rndBetween(0, static_cast<int>(vec.size()) - 1);

    return vec[rndBetween(rnd)];
}

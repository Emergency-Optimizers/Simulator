/**
 * @file Utils.hpp
 *
 * @copyright Copyright (c) 2024 Emergency-Optimizers
 */

#pragma once

/* external libraries */
#include <string>
#include <ctime>
#include <optional>
#include <variant>
#include <vector>
#include <random>
#include <utility>
#include <unordered_map>
/* internal libraries */
#include "simulator/Ambulance.hpp"
#include "simulator/Event.hpp"
#include "file-reader/ODMatrix.hpp"

using ValueType = std::variant<
    int,
    int64_t,
    float,
    double,
    std::string,
    bool,
    std::optional<std::tm>
>;
using ToValueType = ValueType(*)(const std::string&);
using SchemaMapping = std::unordered_map<std::string, ToValueType>;

bool tm_less(const std::tm& lhs, const std::tm& rhs);
ValueType toInt(const std::string& str);
ValueType toInt64(const std::string& str);
ValueType toFloat(const std::string& str);
ValueType toDouble(const std::string& str);
ValueType toString(const std::string& str);
ValueType toBool(const std::string& str);
ValueType toDateTime(const std::string& str);
std::string tmToString(const std::tm& time);
std::string valueTypeToString(const ValueType& cell);
std::tm stringToTm(const std::string& str);
int compareTime(const std::tm& time_1, const std::tm& time_2);
float timeDifferenceInSeconds(std::tm& time1, std::tm& time2);
int findClosestTimeIndex(const std::tm& target, const std::vector<std::tm>& times);
std::vector<unsigned> getAvailableAmbulanceIndicies(
    std::vector<Ambulance>& ambulances,
    const std::vector<Event>& events,
    const time_t& currentTime
);
int calculateDayDifference(const std::tm& baseDate, const int targetMonth, const int targetDay);
int weightedLottery(
    std::mt19937& rnd,
    const std::vector<double>& weights,
    const std::vector<std::pair<int, int>>& ranges = {}
);
int getRandomInt(std::mt19937& rnd, const int min, const int max);
double getRandomProbability(std::mt19937& rnd);
double calculateMean(const std::vector<int>& numbers);
double calculateStandardDeviation(const std::vector<int>& numbers);
double calculateEuclideanDistance(double x1, double y1, double x2, double y2);
std::pair<int, int> idToUtm(int64_t grid_id);
int64_t utmToId(const std::pair<int, int>& utm, const int cellSize = 1000, const int offset = 2000000);
int64_t approximateLocation(
    const int64_t& startId,
    const int64_t& goalId,
    const time_t& timeAtStart,
    const time_t& timeNow,
    const std::string& triage
);
int findEventIndexFromId(const std::vector<Event>& events, const int id);
void writeMetrics(std::vector<Event>& events);
void saveDistributionToFile(const std::vector<std::vector<double>>& distribution, const std::string& baseFilename);
void save1dDistributionToFile(const std::vector<double>& distribution, const std::string& baseFilename);
void save2dDistributionToFile(const std::vector<std::vector<double>>& distribution, const std::string& baseFilename);

template <typename T>
int findIndex(const std::vector<T>& vec, const T& value) {
    auto it = std::find(vec.begin(), vec.end(), value);

    if (it != vec.end()) {
        return std::distance(vec.begin(), it);
    } else {
        return -1;
    }
}

template <typename T>
T getRandomElement(std::mt19937& rng, const std::vector<T>& vec) {
    std::uniform_int_distribution<int> rndBetween(0, vec.size() - 1);
    return vec[rndBetween(rng)];
}

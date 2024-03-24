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

class Utils {
 private:
    static bool tm_less(const std::tm& lhs, const std::tm& rhs);

 public:
    static ValueType toInt(const std::string& str);
    static ValueType toInt64(const std::string& str);
    static ValueType toFloat(const std::string& str);
    static ValueType toDouble(const std::string& str);
    static ValueType toString(const std::string& str);
    static ValueType toBool(const std::string& str);
    static ValueType toDateTime(const std::string& str);
    static std::string tmToString(const std::tm& time);
    static std::string valueTypeToString(const ValueType& cell);
    static std::tm stringToTm(const std::string& str);
    static int compareTime(const std::tm& time_1, const std::tm& time_2);
    static float timeDifferenceInSeconds(std::tm& time1, std::tm& time2);
    static int findClosestTimeIndex(const std::tm& target, const std::vector<std::tm>& times);
    static std::vector<unsigned> getAvailableAmbulanceIndicies(
        std::vector<Ambulance>& ambulances,
        const std::vector<Event>& events,
        const time_t& currentTime
    );
    static int calculateDayDifference(const std::tm& baseDate, const int targetMonth, const int targetDay);
    static int weightedLottery(
        std::mt19937& rnd,
        const std::vector<double>& weights,
        const std::vector<std::pair<int, int>>& ranges = {}
    );
    static int getRandomInt(std::mt19937& rnd, const int min, const int max);
    static double getRandomProbability(std::mt19937& rnd);
    static double calculateMean(const std::vector<int>& numbers);
    static double calculateStandardDeviation(const std::vector<int>& numbers);
    static double calculateEuclideanDistance(double x1, double y1, double x2, double y2);
    static std::pair<int, int> idToUtm(int64_t grid_id);
    static int64_t utmToId(const std::pair<int, int>& utm, const int cellSize = 1000, const int offset = 2000000);
    static int64_t approximateLocation(
        const int64_t& startId,
        const int64_t& goalId,
        const time_t& timeAtStart,
        const time_t& timeNow,
        const std::string& triage
    );
    static int findEventIndexFromId(const std::vector<Event>& events, const int id);
    static void saveMetricsToFile(std::vector<Event>& events);
    static void saveDistributionToFile(const std::vector<std::vector<double>>& distribution, const std::string& baseFilename);
    static void save1dDistributionToFile(const std::vector<double>& distribution, const std::string& baseFilename);
    static void save2dDistributionToFile(const std::vector<std::vector<double>>& distribution, const std::string& baseFilename);
    template <typename T>
    static int findIndex(const std::vector<T>& vec, const T& value) {
        auto it = std::find(vec.begin(), vec.end(), value);

        if (it != vec.end()) {
            return std::distance(vec.begin(), it);
        } else {
            return -1;
        }
    }
    template <typename T>
    static T getRandomElement(std::mt19937& rng, const std::vector<T>& vec) {
        std::uniform_int_distribution<int> rndBetween(0, vec.size() - 1);
        return vec[rndBetween(rng)];
    }
};

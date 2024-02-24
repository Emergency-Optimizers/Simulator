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

using CellType = std::variant<
    int,
    int64_t,
    float,
    std::string,
    bool,
    std::optional<std::tm>
>;

class Utils {
 private:
    static bool tm_less(const std::tm& lhs, const std::tm& rhs);

 public:
    static CellType toInt(const std::string& str);
    static CellType toInt64(const std::string& str);
    static CellType toFloat(const std::string& str);
    static CellType toString(const std::string& str);
    static CellType toBool(const std::string& str);
    static CellType toDateTime(const std::string& str);
    static std::string tmToString(const std::tm& time);
    static std::string cellTypeToString(const CellType& cell);
    static std::tm stringToTm(const std::string& str);
    static int compareTime(const std::tm& time_1, const std::tm& time_2);
    static float timeDifferenceInSeconds(std::tm& time1, std::tm& time2);
    static int findClosestTimeIndex(const std::tm& target, const std::vector<std::tm>& times);
    static std::vector<unsigned> getAvailableAmbulanceIndicies(const std::vector<Ambulance>& ambulances);
    static int calculateDayDifference(const std::tm& baseDate, const int targetMonth, const int targetDay);
    static int weightedLottery(
        std::mt19937& rnd,
        const std::vector<double>& weights,
        const std::vector<std::pair<int, int>>& ranges = {}
    );
    static int getRandomInt(std::mt19937& rnd, const int min, const int max);
    static double getRandomProbability(std::mt19937& rnd);
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
    static void saveEventsToFile(const std::vector<Event>& events);
    static void saveMetricsToFile(const std::vector<Event>& events);
    static void save1dDistributionToFile(const std::vector<double>& distribution, const std::string& baseFilename);
    static void save2dDistributionToFile(const std::vector<std::vector<double>>& distribution, const std::string& baseFilename);
};

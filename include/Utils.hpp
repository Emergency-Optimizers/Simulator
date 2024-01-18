/**
 * @file Utils.hpp
 * @author Sindre Eiklid
 * @version 1.0
 * @date 2023-11-09
 *
 * @copyright Copyright (c) 2023 Sindre Eiklid
 */

#pragma once

/* external libraries */
#include <string>
#include <ctime>
#include <optional>
#include <variant>
#include <vector>
#include <random>
/* internal libraries */
#include "simulator/Ambulance.hpp"

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

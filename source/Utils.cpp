/**
 * @file Utils.cpp
 *
 * @copyright Copyright (c) 2024 Emergency-Optimizers
 */

/* external libraries */
#include <fstream>
#include <sstream>
#include <iomanip>
#include <optional>
#include <vector>
#include <algorithm>
#include <numeric>
#include <iterator>
/* internal libraries */
#include "Utils.hpp"
#include "simulator/CSVReader.hpp"

CellType Utils::toInt(const std::string& str) {
    return std::stoi(str);
}

CellType Utils::toInt64(const std::string& str) {
    return std::stoll(str);
}

CellType Utils::toFloat(const std::string& str) {
    return std::stof(str);
}

CellType Utils::toString(const std::string& str) {
    return str;
}

CellType Utils::toBool(const std::string& str) {
    return str == "True" || str == "true";
}

CellType Utils::toDateTime(const std::string& str) {
    if (str.empty()) {
        return std::nullopt;
    }
    std::tm tm = {};
    std::istringstream ss(str);
    ss >> std::get_time(&tm, "%Y-%m-%d %H:%M:%S");

    if (ss.fail()) {
        return std::nullopt;
    }

    return tm;
}

std::string Utils::tmToString(const std::tm& time) {
    std::stringstream ss;
    ss << std::put_time(&time, "%Y-%m-%d %H:%M:%S");
    return ss.str();
}

std::string Utils::cellTypeToString(const CellType& cell) {
    return std::visit([](auto&& arg) -> std::string {
        using T = std::decay_t<decltype(arg)>;
        if constexpr (std::is_same_v<T, int>) {
            return std::to_string(arg);
        } else if constexpr (std::is_same_v<T, int64_t>) {
            return std::to_string(arg);
        } else if constexpr (std::is_same_v<T, float>) {
            return std::to_string(arg);
        } else if constexpr (std::is_same_v<T, std::string>) {
            return arg;
        } else if constexpr (std::is_same_v<T, bool>) {
            return arg ? "true" : "false";
        } else if constexpr (std::is_same_v<T, std::optional<std::tm>>) {
            return arg ? tmToString(arg.value()) : "n/a";
        }
    }, cell);
}

std::tm Utils::stringToTm(const std::string& str) {
    std::tm time{};
    std::istringstream ss(str);
    ss >> std::get_time(&time, "%Y.%m.%dT%H:%M:%S");
    return time;
}

int Utils::compareTime(const std::tm& time_1, const std::tm& time_2) {
    if (time_1.tm_year > time_2.tm_year) return 1;
    if (time_1.tm_year < time_2.tm_year) return -1;
    if (time_1.tm_mon > time_2.tm_mon) return 1;
    if (time_1.tm_mon < time_2.tm_mon) return -1;
    if (time_1.tm_mday > time_2.tm_mday) return 1;
    if (time_1.tm_mday < time_2.tm_mday) return -1;
    if (time_1.tm_hour > time_2.tm_hour) return 1;
    if (time_1.tm_hour < time_2.tm_hour) return -1;
    if (time_1.tm_min > time_2.tm_min) return 1;
    if (time_1.tm_min < time_2.tm_min) return -1;
    if (time_1.tm_sec > time_2.tm_sec) return 1;
    if (time_1.tm_sec < time_2.tm_sec) return -1;
    return 0;
}

float Utils::timeDifferenceInSeconds(std::tm& time1, std::tm& time2) {
    time_t t1 = std::mktime(&time1);
    time_t t2 = std::mktime(&time2);

    return std::difftime(t2, t1);
}

int Utils::findClosestTimeIndex(const std::tm& target, const std::vector<std::tm>& times) {
    if (times.empty()) return -1;

    auto lower = std::lower_bound(times.begin(), times.end(), target, tm_less);

    if (lower == times.end()) return times.size() - 1;
    if (lower == times.begin()) return 0;

    auto prev = lower - 1;

    auto diff1 = std::mktime(const_cast<std::tm*>(&*lower)) - std::mktime(const_cast<std::tm*>(&target));
    auto diff2 = std::mktime(const_cast<std::tm*>(&target)) - std::mktime(const_cast<std::tm*>(&*prev));

    if (diff1 < diff2) {
        return std::distance(times.begin(), lower);
    } else {
        return std::distance(times.begin(), prev);
    }
}

bool Utils::tm_less(const std::tm& lhs, const std::tm& rhs) {
    if (lhs.tm_year != rhs.tm_year) return lhs.tm_year < rhs.tm_year;
    if (lhs.tm_mon != rhs.tm_mon) return lhs.tm_mon < rhs.tm_mon;
    if (lhs.tm_mday != rhs.tm_mday) return lhs.tm_mday < rhs.tm_mday;
    if (lhs.tm_hour != rhs.tm_hour) return lhs.tm_hour < rhs.tm_hour;
    if (lhs.tm_min != rhs.tm_min) return lhs.tm_min < rhs.tm_min;
    return lhs.tm_sec < rhs.tm_sec;
}

std::vector<unsigned> Utils::getAvailableAmbulanceIndicies(const std::vector<Ambulance>& ambulances) {
    std::vector<unsigned> availableAmbulanceIndicies;

    for (int i = 0; i < ambulances.size(); i++) {
        if (ambulances[i].assignedEventIndex == -1) availableAmbulanceIndicies.push_back(i);
    }

    return availableAmbulanceIndicies;
}

int Utils::calculateDayDifference(const std::tm& baseDate, const int targetMonth, const int targetDay) {
    int year = baseDate.tm_year;
    bool isLeapYear = (year % 4 == 0) && ((year % 100 != 0) || (year % 400 == 0));
    int totalDaysInYear = isLeapYear ? 365 : 364;

    std::tm baseDateNormalized = baseDate;
    baseDateNormalized.tm_hour = 0;
    baseDateNormalized.tm_min = 0;
    baseDateNormalized.tm_sec = 0;
    mktime(&baseDateNormalized);

    // create tm structure for the target date in the same year as the base date
    std::tm targetDate = {0};
    targetDate.tm_year = year;
    targetDate.tm_mon = targetMonth - 1;
    targetDate.tm_mday = targetDay;
    targetDate.tm_hour = 0;
    targetDate.tm_min = 0;
    targetDate.tm_sec = 0;
    mktime(&targetDate);

    int targetDayOfYear = targetDate.tm_yday;
    int baseDayOfYear = baseDateNormalized.tm_yday;
    int dayDiff = 0;

    if (targetDayOfYear == baseDayOfYear) return 0;

    while (true) {
        if (targetDayOfYear == baseDayOfYear) break;
        if (++baseDayOfYear > totalDaysInYear) baseDayOfYear = 0;
        dayDiff++;
    }

    targetDayOfYear = targetDate.tm_yday;
    baseDayOfYear = baseDateNormalized.tm_yday;
    int newDayDiff = 0;

    while (dayDiff != newDayDiff) {
        if (targetDayOfYear == baseDayOfYear) break;
        if (--baseDayOfYear < 0) baseDayOfYear = totalDaysInYear;
        newDayDiff++;
    }

    if (dayDiff > newDayDiff) dayDiff = newDayDiff;

    return dayDiff;
}

int Utils::weightedLottery(
    std::mt19937& rnd,
    const std::vector<double>& weights,
    const std::vector<std::pair<int, int>>& ranges
) {
    std::vector<double> adjustedWeights;

    if (ranges.empty()) {
        // If ranges is empty, use the entire weights vector
        adjustedWeights = weights;
    } else {
        // Otherwise, process each specified range
        for (const auto& range : ranges) {
            int beginIndex = std::max(0, range.first);
            int endIndex = std::min(static_cast<int>(weights.size()) - 1, range.second);

            if (beginIndex <= endIndex) {
                adjustedWeights.insert(adjustedWeights.end(), weights.begin() + beginIndex, weights.begin() + endIndex + 1);
            }
        }
    }

    // Create a partial sum of the adjusted weights
    std::vector<double> cumulativeWeights(adjustedWeights.size());
    std::partial_sum(adjustedWeights.begin(), adjustedWeights.end(), cumulativeWeights.begin());

    // Generate a random number in the adjusted range
    std::uniform_real_distribution<> dist(0.0, cumulativeWeights.back());
    auto it = std::lower_bound(cumulativeWeights.begin(), cumulativeWeights.end(), dist(rnd));
    int indexInAdjustedWeights = std::distance(cumulativeWeights.begin(), it);

    // For the default case (full range), the adjusted index is the final result
    if (ranges.empty()) {
        return indexInAdjustedWeights;
    }

    // Adjust the index to map back to the original weights vector for specified ranges
    int originalIndex = 0;
    for (const auto& range : ranges) {
        int rangeSize = range.second - range.first + 1;
        if (indexInAdjustedWeights < rangeSize) {
            originalIndex += range.first + indexInAdjustedWeights;
            break;
        } else {
            indexInAdjustedWeights -= rangeSize;
        }
    }

    return originalIndex;
}

int Utils::getRandomInt(std::mt19937& rnd, const int min, const int max) {
    std::uniform_int_distribution<> dist(min, max);

    return dist(rnd);
}

double Utils::getRandomProbability(std::mt19937& rnd) {
    std::uniform_real_distribution<> dist(0.0, 1.0);
    return dist(rnd);
}

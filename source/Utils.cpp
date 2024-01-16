/**
 * @file Utils.cpp
 * @author Sindre Eiklid
 * @version 1.0
 * @date 2023-11-08
 *
 * @copyright Copyright (c) 2023 Sindre Eiklid
 */

/* external libraries */
#include <fstream>
#include <sstream>
#include <iomanip>
#include <optional>
#include <vector>
#include <algorithm>
/* internal libraries */
#include "Utils.hpp"
#include "CSVReader.hpp"

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
    ss >> std::get_time(&tm, "%Y.%m.%dT%H:%M:%S");

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

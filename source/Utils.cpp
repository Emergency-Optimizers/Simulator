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

CellType Utils::toInt(const std::string& str) {
    return std::stoi(str);
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

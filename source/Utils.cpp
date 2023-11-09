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
#include <iostream>
/* internal libraries */
#include "Utils.hpp"

int Utils::toInt(const std::string& str) {
    return std::stoi(str);
}

float Utils::toFloat(const std::string& str) {
    return std::stof(str);
}

std::string Utils::toString(const std::string& str) {
    return str;
}

bool Utils::toBool(const std::string& str) {
    return str == "True" || str == "true";
}

std::optional<std::tm> Utils::toDateTime(const std::string& str) {
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

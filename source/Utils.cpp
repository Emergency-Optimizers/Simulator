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

template <typename T>
static int findIndex(const std::vector<T>& vec, const T& value) {
    auto it = std::find(vec.begin(), vec.end(), value);

    if (it != vec.end()) {
        return std::distance(vec.begin(), it);
    } else {
        return -1;
    }
}

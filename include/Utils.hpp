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
#include <vector>
#include <ctime>
#include <optional>
/* internal libraries */
#include "CSVReader.hpp"

class Utils {
 public:
    static int toInt(const std::string& str);
    static float toFloat(const std::string& str);
    static std::string toString(const std::string& str);
    static bool toBool(const std::string& str);
    static std::optional<std::tm> toDateTime(const std::string& str);
};

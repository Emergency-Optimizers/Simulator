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

using CellType = std::variant<
    int,
    float,
    std::string,
    bool,
    std::optional<std::tm>
>;


class Utils {
 public:
    static CellType toInt(const std::string& str);
    static CellType toFloat(const std::string& str);
    static CellType toString(const std::string& str);
    static CellType toBool(const std::string& str);
    static CellType toDateTime(const std::string& str);
    template <typename T>
    static int findIndex(const std::vector<T>& vec, const T& value);
};

/**
 * @file Incidents.hpp
 * @author Sindre Eiklid
 * @version 1.0
 * @date 2023-11-08
 *
 * @copyright Copyright (c) 2023 Sindre Eiklid
 */

#ifndef INCIDENTS_HPP_
#define INCIDENTS_HPP_

/* external libraries */
#include <string>
#include <unordered_map>
#include <functional>
#include <variant>
#include <map>
#include <vector>
#include <ctime>
#include <optional>
/* internal libraries */
#include "CSVReader.hpp"

// Define a variant to hold any type of cell data.
using CellType = std::variant<
    int,
    float,
    std::string,
    bool,
    std::optional<std::tm>
>;

// Alias for a mapping of header names to their corresponding types.
using SchemaMapping = std::unordered_map<std::string, std::function<CellType(const std::string&)>>;

class Incidents {
 private:
    CSVReader reader;
    SchemaMapping schemaMapping;
    std::vector<std::map<std::string, CellType>> typedData;

    static int toInt(const std::string& str);
    static float toFloat(const std::string& str);
    static std::string toString(const std::string& str);
    static bool toBool(const std::string& str);
    static std::optional<std::tm> toDateTime(const std::string& str);

    void convertRowToTypedData(const CSVRow& row);

 public:
    Incidents();
    void loadFromFile(const std::string& filePath);
    void printRow(const int& index);
};

#endif  // INCIDENTS_HPP_

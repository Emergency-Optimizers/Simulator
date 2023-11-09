/**
 * @file Incidents.cpp
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
#include "Incidents.hpp"
#include "Utils.hpp"

// Constructor
Incidents::Incidents() {
    // Initialize the schema mapping here
    schemaMapping = {
        {"id", [](const std::string& str) -> CellType { return Utils::toInt(str); }},
        {"synthetic", [](const std::string& str) -> CellType { return Utils::toBool(str); }},
        {"triage_impression_during_call", [](const std::string& str) -> CellType { return Utils::toString(str); }},
        {"time_call_received", [](const std::string& str) -> CellType { return Utils::toDateTime(str); }},
        {"time_call_processed", [](const std::string& str) -> CellType { return Utils::toDateTime(str); }},
        {"time_ambulance_notified", [](const std::string& str) -> CellType { return Utils::toDateTime(str); }},
        {"time_dispatch", [](const std::string& str) -> CellType { return Utils::toDateTime(str); }},
        {"time_arrival_scene", [](const std::string& str) -> CellType { return Utils::toDateTime(str); }},
        {"time_departure_scene", [](const std::string& str) -> CellType { return Utils::toDateTime(str); }},
        {"time_arrival_hospital", [](const std::string& str) -> CellType { return Utils::toDateTime(str); }},
        {"time_available", [](const std::string& str) -> CellType { return Utils::toDateTime(str); }},
        {"response_time_sec", [](const std::string& str) -> CellType { return Utils::toFloat(str); }},
        {"longitude", [](const std::string& str) -> CellType { return Utils::toFloat(str); }},
        {"latitude", [](const std::string& str) -> CellType { return Utils::toFloat(str); }},
        {"easting", [](const std::string& str) -> CellType { return Utils::toInt(str); }},
        {"northing", [](const std::string& str) -> CellType { return Utils::toInt(str); }},
        {"grid_id", [](const std::string& str) -> CellType { return Utils::toInt(str); }},
        {"grid_row", [](const std::string& str) -> CellType { return Utils::toInt(str); }},
        {"grid_col", [](const std::string& str) -> CellType { return Utils::toInt(str); }},
        {"region", [](const std::string& str) -> CellType { return Utils::toString(str); }},
        {"urban_settlement", [](const std::string& str) -> CellType { return Utils::toBool(str); }},
    };
}

void Incidents::loadFromFile(const std::string& filePath) {
    reader.readCSV(filePath);
    for (std::size_t i = 0; i < reader.size(); ++i) {
        const auto& row = reader[i];
        convertRowToTypedData(row);
    }
}

void Incidents::convertRowToTypedData(const CSVRow& row) {
    std::map<std::string, CellType> typedRow;
    for (const auto& header : reader.getHeaders()) {
        typedRow[header] = schemaMapping[header](row[header]);
    }
    typedData.push_back(std::move(typedRow));
}

void Incidents::printRow(const int& index) {
    for (const auto& header : reader.getHeaders()) {
        const auto& cell = typedData[index][header];
        std::cout << '\t' << header << ": ";
        std::visit([](auto&& value) {
            using T = std::decay_t<decltype(value)>;
            if constexpr (std::is_same_v<T, int> ||
                          std::is_same_v<T, float> ||
                          std::is_same_v<T, std::string> ||
                          std::is_same_v<T, bool>) {
                std::cout << value;
            } else if constexpr (std::is_same_v<T, std::optional<std::tm>>) {
                if (value) {
                    // If the optional has a value, print it using the standard format
                    std::cout << std::put_time(&value.value(), "%Y-%m-%d %H:%M:%S");
                } else {
                    // If the optional is empty, print a placeholder
                    std::cout << "n/a";
                }
            }
        }, cell);
        std::cout << '\n';
    }
}

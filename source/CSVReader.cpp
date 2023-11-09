/**
 * @file CSVReader.cpp
 * @author Sindre Eiklid
 * @version 1.0
 * @date 2023-11-08
 *
 * @copyright Copyright (c) 2023 Sindre Eiklid
 */

/* external libraries */
#include <fstream>
#include <iomanip>
/* internal libraries */
#include "CSVReader.hpp"

void CSVReader::loadFromFile(const std::string& filename) {
    std::ifstream file(filename);
    if (!file.is_open()) {
        throw std::runtime_error("Could not open file");
    }

    std::string line;

    if (std::getline(file, line)) {
        std::stringstream headerStream(line);
        std::string column;
        while (std::getline(headerStream, column, ',')) {
            headers.push_back(column);
        }
    }

    while (std::getline(file, line)) {
        parseRow(line);
    }
}

void CSVReader::parseRow(const std::string& line) {
    std::stringstream ss(line);
    std::string cell;
    std::vector<CellType> row;

    std::size_t columnIndex = 0;
    while (std::getline(ss, cell, ',') && columnIndex < headers.size()) {
        const auto& header = headers[columnIndex++];
        row.push_back(schemaMapping[header](cell));
    }

    rows.push_back(std::move(row));
}

const std::vector<CellType>& CSVReader::operator[](std::size_t index) const {
    if (index < rows.size()) {
        return rows[index];
    } else {
        throw std::out_of_range("Index out of range");
    }
}

std::size_t CSVReader::size() const {
    return rows.size();
}

const std::vector<std::string>& CSVReader::getHeaders() const {
    return headers;
}

void CSVReader::printRow(const std::size_t index) {
    if (index >= size()) {
        std::cout << "Index out of range" << std::endl;
        return;
    }
    std::cout << "Row " << index << ": " << std::endl;
    const auto& row = rows[index];
    for (std::size_t i = 0; i < row.size(); ++i) {
        const auto& cell = row[i];
        std::cout << '\t' << headers[i] << ": ";
        std::visit([](auto&& value) {
            using T = std::decay_t<decltype(value)>;
            if constexpr (std::is_same_v<T, int> ||
                          std::is_same_v<T, float> ||
                          std::is_same_v<T, std::string> ||
                          std::is_same_v<T, bool>) {
                std::cout << value;
            } else if constexpr (std::is_same_v<T, std::optional<std::tm>>) {
                if (value) {
                    std::cout << std::put_time(&value.value(), "%Y-%m-%d %H:%M:%S");
                } else {
                    std::cout << "n/a";
                }
            }
        }, cell);
        std::cout << '\n';
    }
}

/**
 * @file CSVRow.cpp
 *
 * @copyright Copyright (c) 2024 Emergency-Optimizers
 */

/* internal libraries */
#include "simulator/CSVRow.hpp"

void CSVRow::readNextRow(std::istream& str, const std::vector<std::string>& headers) {
    std::string line, cell;
    std::getline(str, line);
    std::stringstream lineStream(line);
    size_t headerIndex = 0;

    while (std::getline(lineStream, cell, ',') && headerIndex < headers.size()) {
        data[headers[headerIndex++]] = cell;
    }
}

const std::string& CSVRow::operator[](const std::string& column) const {
    auto it = data.find(column);
    if (it != data.end()) {
        return it->second;
    }
    static const std::string empty_string;
    std::cerr << "Column not found: " << column << std::endl;
    return empty_string;
}

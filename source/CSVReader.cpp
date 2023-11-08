/**
 * @file CSVReader.cpp
 * @author Sindre Eiklid
 * @version 1.0
 * @date 2023-11-08
 *
 * @copyright Copyright (c) 2023 Sindre Eiklid
 */

/* internal libraries */
#include "CSVReader.hpp"

void CSVReader::readCSV(const std::string& filename) {
    std::ifstream file(filename);
    if (!file.is_open()) {
        throw std::runtime_error("Could not open file");
    }

    std::string line;
    // Read the header
    if (std::getline(file, line)) {
        std::stringstream headerStream(line);
        std::string column;
        while (std::getline(headerStream, column, ',')) {
            headers.push_back(column);
        }
    }

    // Read the data
    CSVRow row;
    while (file) {
        row.readNextRow(file, headers);
        if (!file.eof()) {  // Avoid adding a blank row at the end
            rows.push_back(row);
        }
    }
}

const CSVRow& CSVReader::operator[](std::size_t index) const {
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

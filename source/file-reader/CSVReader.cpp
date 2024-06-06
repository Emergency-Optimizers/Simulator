/**
 * @file CSVReader.cpp
 *
 * @copyright Copyright (c) 2024 Emergency-Optimizers
 */

/* external libraries */
#include <fstream>
#include <iostream>
#include <sstream>
/* internal libraries */
#include "file-reader/CSVReader.hpp"
#include "ProgressBar.hpp"

void CSVReader::loadFromFile(const std::string& filename, const std::string& printPrefix) {
    std::ifstream file(filename);
    if (!file.is_open()) {
        throwError("Could not open file at path '" + filename + "'.");
    }

    // count total lines
    size_t totalLines = 0;
    std::string tempLine;
    while (std::getline(file, tempLine)) {
        totalLines++;
    }

    // reset file to beginning
    file.clear();
    file.seekg(0, std::ios::beg);

    // setup progressBar
    ProgressBar progressBar(totalLines, printPrefix);

    // start processing file
    std::string line;
    int linesRead = 0;

    if (std::getline(file, line)) {
        std::stringstream headerStream(line);
        std::string column;
        while (std::getline(headerStream, column, ',')) {
            headers.push_back(column);
        }

        progressBar.update(++linesRead);
    }

    while (std::getline(file, line)) {
        parseRow(line);

        progressBar.update(++linesRead);
    }

    file.close();
}

void CSVReader::parseRow(const std::string& line) {
    std::stringstream ss(line);
    std::string cell;
    std::vector<ValueType> row;

    std::size_t columnIndex = 0;
    // store all data fields in the row
    while (std::getline(ss, cell, ',') && columnIndex < headers.size()) {
        const auto& header = headers[columnIndex++];
        row.push_back(schemaMapping[header](cell));
    }

    rows.push_back(std::move(row));
}

const std::vector<ValueType>& CSVReader::operator[](const int index) const {
    if (index < rows.size()) {
        return rows[index];
    } else {
        throw std::out_of_range("Index out of range");
    }
}

int CSVReader::size() const {
    return static_cast<int>(rows.size());
}

void CSVReader::print() {
    for (int i = 0; i < size(); i++) {
        printRow(i);
    }
}

void CSVReader::printRow(const int index) {
    if (index >= size()) {
        std::cout << "Index out of range" << std::endl;
        return;
    }

    std::cout << "Row " << index << ": " << std::endl;
    const auto& row = rows[index];
    for (std::size_t i = 0; i < headers.size(); ++i) {
        const auto& cell = row[i];
        std::cout << '\t' << headers[i] << ": " << valueTypeToString(cell) << std::endl;
    }
}

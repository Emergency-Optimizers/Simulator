/**
 * @file CSVReader.hpp
 *
 * @copyright Copyright (c) 2024 Emergency-Optimizers
 */

#pragma once

/* external libraries */
#include <string>
#include <vector>
/* internal libraries */
#include "Utils.hpp"

class CSVReader {
 protected:
    SchemaMapping schemaMapping;
    std::vector<std::vector<ValueType>> rows;
    std::vector<std::string> headers;

    void parseRow(const std::string& line);

 public:
    ~CSVReader() = default;
    void loadFromFile(const std::string& filename);
    const std::vector<ValueType>& operator[](const int index) const;
    int size() const;
    void print();
    void printRow(const int index);

    template <typename T>
    T get(const std::string& header, const int index) {
        return std::get<T>(rows[index][findIndex(headers, header)]);
    }
};

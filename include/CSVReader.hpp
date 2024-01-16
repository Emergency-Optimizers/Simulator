/**
 * @file CSVReader.hpp
 * @author Sindre Eiklid
 * @version 1.0
 * @date 2023-11-08
 *
 * @copyright Copyright (c) 2023 Sindre Eiklid
 */

#pragma once

/* external libraries */
#include <string>
#include <vector>
#include <variant>
#include <unordered_map>
#include <functional>
#include <optional>
/* internal libraries */
#include "CSVRow.hpp"
#include "Utils.hpp"

using ToCellType = CellType(*)(const std::string&);

using SchemaMapping = std::unordered_map<std::string, ToCellType>;

class CSVReader {
 protected:
    SchemaMapping schemaMapping;
    std::vector<std::vector<CellType>> rows;
    std::vector<std::string> headers;

    virtual void parseRow(const std::string& line);

 public:
    virtual ~CSVReader() = default;
    virtual void loadFromFile(const std::string& filename);
    const std::vector<CellType>& operator[](std::size_t index) const;
    std::size_t size() const;
    const std::vector<std::string>& getHeaders() const;
    void printRow(std::size_t index);
    void print();
    const std::vector<std::tm> getColumnOfTimes(const std::string& header);
    template <typename T>
    T get(const std::string& header, const int index) {
        return std::get<T>(rows[index][Utils::findIndex(headers, header)]);
    }
};

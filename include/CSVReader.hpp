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

using CellType = std::variant<
    int,
    float,
    std::string,
    bool,
    std::optional<std::tm>
>;

using SchemaMapping = std::unordered_map<std::string, std::function<CellType(const std::string&)>>;

class CSVReader {
 protected:
    SchemaMapping schemaMapping;
    std::vector<std::vector<CellType>> typedData;
    std::vector<std::string> headers;

    virtual void parseRow(const std::string& line);

 public:
    virtual ~CSVReader() = default;
    virtual void loadFromFile(const std::string& filename);
    const std::vector<CellType>& operator[](std::size_t index) const;
    std::size_t size() const;
    const std::vector<std::string>& getHeaders() const;
    void printRow(const int& index);
};

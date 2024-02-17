/**
 * @file CSVRow.hpp
 *
 * @copyright Copyright (c) 2024 Emergency-Optimizers
 */

#pragma once

/* external libraries */
#include <iostream>
#include <sstream>
#include <vector>
#include <string>
#include <map>

class CSVRow {
 private:
    std::map<std::string, std::string> data;

 public:
    void readNextRow(std::istream& str, const std::vector<std::string>& headers);
    const std::string& operator[](const std::string& column) const;
};

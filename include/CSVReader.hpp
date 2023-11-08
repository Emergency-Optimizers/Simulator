/**
 * @file CSVReader.hpp
 * @author Sindre Eiklid
 * @version 1.0
 * @date 2023-11-08
 *
 * @copyright Copyright (c) 2023 Sindre Eiklid
 */

#ifndef CSVREADER_HPP_
#define CSVREADER_HPP_

/* external libraries */
#include <fstream>
#include <string>
#include <vector>
/* internal libraries */
#include "CSVRow.hpp"

class CSVReader {
 private:
    std::vector<CSVRow> rows;
    std::vector<std::string> headers;

 public:
    void readCSV(const std::string& filename);
    const CSVRow& operator[](std::size_t index) const;
    std::size_t size() const;
    const std::vector<std::string>& getHeaders() const;
};

#endif  // CSVREADER_HPP_

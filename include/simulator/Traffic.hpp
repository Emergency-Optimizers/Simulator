/**
 * @file Traffic.hpp
 *
 * @copyright Copyright (c) 2024 Emergency-Optimizers
 */

#pragma once

/* external libraries */
#include <string>
/* internal libraries */
#include "simulator/CSVReader.hpp"

class Traffic : public CSVReader {
 private:
    Traffic();

 public:
    Traffic(const Traffic&) = delete;
    Traffic& operator=(const Traffic&) = delete;
    static Traffic& getInstance() {
        static Traffic instance;
        return instance;
    }
    double getTrafficFactor(const time_t& time);
};

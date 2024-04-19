/**
 * @file KDEData.hpp
 *
 * @copyright Copyright (c) 2024 Emergency-Optimizers
 */

#pragma once

/* external libraries */
#include <vector>

struct KDEData {
    std::vector<double> data;
    std::vector<double> weights;
    std::vector<double> points;
    std::vector<double> densities;
};

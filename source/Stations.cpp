/**
 * @file Stations.cpp
 * @author Sindre Eiklid
 * @version 1.0
 * @date 2023-11-08
 *
 * @copyright Copyright (c) 2023 Sindre Eiklid
 */

/* internal libraries */
#include "Stations.hpp"

Stations::Stations() {
    schemaMapping = {
        {"type", Utils::toString},
        {"static", Utils::toBool},
        {"longitude", Utils::toFloat},
        {"latitude", Utils::toFloat},
        {"easting", Utils::toInt},
        {"northing", Utils::toInt},
        {"grid_id", Utils::toInt64},
        {"grid_row", Utils::toInt},
        {"grid_col", Utils::toInt},
    };
}

std::vector<unsigned> Stations::getDepotIndices() {
    std::vector<unsigned> depotIndices;

    for (int i = 0; i < size(); i++) {
        if (get<std::string>("type", i) == "Depot") depotIndices.push_back(i);
    }

    return depotIndices;
}

std::vector<unsigned> Stations::getHospitalIndices() {
    std::vector<unsigned> hospitalIndices;

    for (int i = 0; i < size(); i++) {
        if (get<std::string>("type", i) == "Hospital") hospitalIndices.push_back(i);
    }

    return hospitalIndices;
}

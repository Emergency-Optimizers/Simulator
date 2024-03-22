/**
 * @file Stations.cpp
 *
 * @copyright Copyright (c) 2024 Emergency-Optimizers
 */

/* internal libraries */
#include "simulator/Stations.hpp"

Stations::Stations() {
    schemaMapping = {
        {"type", Utils::toString},
        {"grid_id", Utils::toInt64},
        {"x", Utils::toInt},
        {"y", Utils::toInt},
        {"longitude", Utils::toFloat},
        {"latitude", Utils::toFloat},
        {"region", Utils::toString},
        {"urban_settlement", Utils::toBool},
    };

    std::cout << "Loading Stations..." << std::flush;
    loadFromFile("../../Data-Processing/data/enhanced/oslo/depots.csv");
    std::cout << "\rLoading Stations... " << "\tDONE" << std::endl;
}

std::vector<unsigned> Stations::getDepotIndices(const bool useExtraDepots) {
    std::vector<unsigned> depotIndices;

    for (int i = 0; i < size(); i++) {
        std::string type = get<std::string>("type", i);
        if (type == "Depot" || (useExtraDepots && type == "Beredskapspunkt")) depotIndices.push_back(i);
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

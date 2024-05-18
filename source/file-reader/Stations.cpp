/**
 * @file Stations.cpp
 *
 * @copyright Copyright (c) 2024 Emergency-Optimizers
 */

/* external libraries */
#include <iostream>
/* internal libraries */
#include "file-reader/Stations.hpp"
#include "file-reader/Settings.hpp"

Stations::Stations() {
    schemaMapping = {
        {"name", toString},
        {"type", toString},
        {"grid_id", toInt64},
        {"x", toInt},
        {"y", toInt},
        {"longitude", toFloat},
        {"latitude", toFloat},
        {"region", toString},
        {"urban_settlement_ssb", toBool},
        {"urban_settlement_fhi", toBool},
        {"total_population_radius_2km", toInt},
        {"total_population_radius_5km", toInt},
        {"total_incidents_radius_2km", toInt},
        {"total_incidents_radius_5km", toInt},
        {"total_population_cluster", toInt},
        {"total_incidents_cluster", toInt},
    };

    loadFromFile("../../Data-Processing/data/enhanced/oslo/depots.csv", "Loading stations data");
}

std::vector<unsigned> Stations::getDepotIndices(const bool useExtraDepots) {
    std::vector<unsigned> depotIndices;

    for (int i = 0; i < size(); i++) {
        if (Settings::get<int>("SKIP_STATION_INDEX") == i) {
            continue;
        }

        std::string type = get<std::string>("type", i);
        if (type == "Depot" || (useExtraDepots && type == "Beredskapspunkt")) {
            depotIndices.push_back(i);
        }
    }

    return depotIndices;
}

std::vector<unsigned> Stations::getHospitalIndices() {
    std::vector<unsigned> hospitalIndices;

    for (int i = 0; i < size(); i++) {
        if (get<std::string>("type", i) == "Hospital") {
            hospitalIndices.push_back(i);
        }
    }

    return hospitalIndices;
}

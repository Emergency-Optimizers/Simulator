/**
 * @file Incidents.cpp
 *
 * @copyright Copyright (c) 2024 Emergency-Optimizers
 */

/* external libraries */
#include <iostream>
/* internal libraries */
#include "file-reader/Incidents.hpp"

Incidents::Incidents() {
    schemaMapping = {
        {"triage_impression_during_call", toString},
        {"resource_id", toString},
        {"resource_type", toString},
        {"resources_sent", toInt},
        {"time_call_received", toDateTime},
        {"time_incident_created", toDateTime},
        {"time_resource_appointed", toDateTime},
        {"time_ambulance_dispatch_to_scene", toDateTime},
        {"time_ambulance_arrived_at_scene", toDateTime},
        {"time_ambulance_dispatch_to_hospital", toDateTime},
        {"time_ambulance_arrived_at_hospital", toDateTime},
        {"time_ambulance_available", toDateTime},
        {"grid_id", toInt64},
        {"x", toInt},
        {"y", toInt},
        {"longitude", toFloat},
        {"latitude", toFloat},
        {"region", toString},
        {"urban_settlement", toBool},
        {"total_morning", toInt},
        {"total_day", toInt},
        {"total_night", toInt},
    };

    std::cout << "Loading Incidents..." << std::flush;
    loadFromFile("../../Data-Processing/data/enhanced/oslo/incidents.csv");
    std::cout << "\rLoading Incidents... " << "\tDONE" << std::endl;

    for (int i = 0; i < size(); i++) {
        int64_t grid_id = get<int64_t>("grid_id", i);
        int64_t urban_settlement = get<bool>("urban_settlement", i);

        gridIdUrban[grid_id] = urban_settlement;
    }
}

float Incidents::timeDifferenceBetweenHeaders(const std::string& header1, const std::string& header2, const int index) {
    std::tm time1 = get<std::optional<std::tm>>(header1, index).value();
    std::tm time2 = get<std::optional<std::tm>>(header2, index).value();

    return timeDifferenceInSeconds(time1, time2);
}

std::vector<int> Incidents::rowsWithinTimeFrame(const int month, const int day, const unsigned windowSize) {
    std::vector<int> indicies;

    for (std::size_t i = 0; i < rows.size(); ++i) {
        std::tm timeCallReceived = get<std::optional<std::tm>>("time_call_received", i).value();

        int dayDiff = calculateDayDifference(timeCallReceived, month, day);

        bool withinWindowSize = dayDiff <= windowSize;

        if (withinWindowSize) {
            indicies.push_back(i);
        }
    }

    return indicies;
}

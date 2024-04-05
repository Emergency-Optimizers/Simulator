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
        {"total_incidents_hour_0", toInt},
        {"total_incidents_hour_1", toInt},
        {"total_incidents_hour_2", toInt},
        {"total_incidents_hour_3", toInt},
        {"total_incidents_hour_4", toInt},
        {"total_incidents_hour_5", toInt},
        {"total_incidents_hour_6", toInt},
        {"total_incidents_hour_7", toInt},
        {"total_incidents_hour_8", toInt},
        {"total_incidents_hour_9", toInt},
        {"total_incidents_hour_10", toInt},
        {"total_incidents_hour_11", toInt},
        {"total_incidents_hour_12", toInt},
        {"total_incidents_hour_13", toInt},
        {"total_incidents_hour_14", toInt},
        {"total_incidents_hour_15", toInt},
        {"total_incidents_hour_16", toInt},
        {"total_incidents_hour_17", toInt},
        {"total_incidents_hour_18", toInt},
        {"total_incidents_hour_19", toInt},
        {"total_incidents_hour_20", toInt},
        {"total_incidents_hour_21", toInt},
        {"total_incidents_hour_22", toInt},
        {"total_incidents_hour_23", toInt},
    };

    loadFromFile("../../Data-Processing/data/enhanced/oslo/incidents.csv", "Loading incidents data");

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

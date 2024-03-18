/**
 * @file Incidents.cpp
 *
 * @copyright Copyright (c) 2024 Emergency-Optimizers
 */

/* external libraries */
#include <ctime>
#include <cmath>
#include <chrono>
/* internal libraries */
#include "simulator/Incidents.hpp"

Incidents::Incidents() {
    schemaMapping = {
        {"triage_impression_during_call", Utils::toString},
        {"resource_id", Utils::toString},
        {"resource_type", Utils::toString},
        {"resources_sent", Utils::toInt},
        {"time_call_received", Utils::toDateTime},
        {"time_incident_created", Utils::toDateTime},
        {"time_resource_appointed", Utils::toDateTime},
        {"time_ambulance_dispatch_to_scene", Utils::toDateTime},
        {"time_ambulance_arrived_at_scene", Utils::toDateTime},
        {"time_ambulance_dispatch_to_hospital", Utils::toDateTime},
        {"time_ambulance_arrived_at_hospital", Utils::toDateTime},
        {"time_ambulance_available", Utils::toDateTime},
        {"grid_id", Utils::toInt64},
        {"x", Utils::toInt},
        {"y", Utils::toInt},
        {"longitude", Utils::toFloat},
        {"latitude", Utils::toFloat},
        {"region", Utils::toString},
        {"urban_settlement", Utils::toBool},
        {"total_morning", Utils::toInt},
        {"total_day", Utils::toInt},
        {"total_night", Utils::toInt}
    };

    loadFromFile("../../Data-Processing/data/enhanced/oslo/incidents.csv");

    for (int i = 0; i < size(); i++) {
        int64_t grid_id = get<int64_t>("grid_id", i);
        int64_t urban_settlement = get<bool>("urban_settlement", i);

        gridIdUrban[grid_id] = urban_settlement;
    }
}

float Incidents::timeDifferenceBetweenHeaders(const std::string& header1, const std::string& header2, unsigned index) {
    std::tm time1 = get<std::optional<std::tm>>(header1, index).value();
    std::tm time2 = get<std::optional<std::tm>>(header2, index).value();

    return Utils::timeDifferenceInSeconds(time1, time2);
}

std::vector<int> Incidents::rowsWithinTimeFrame(const int month, const int day, const unsigned windowSize) {
    std::vector<int> indicies;

    for (std::size_t i = 0; i < rows.size(); ++i) {
        std::tm timeCallReceived = get<std::optional<std::tm>>("time_call_received", i).value();

        int dayDiff = Utils::calculateDayDifference(timeCallReceived, month, day);

        bool withinWindowSize = dayDiff <= windowSize;

        if (withinWindowSize) {
            indicies.push_back(i);
        }
    }

    return indicies;
}

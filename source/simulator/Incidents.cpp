/**
 * @file Incidents.cpp
 * @author Sindre Eiklid
 * @version 1.0
 * @date 2023-11-08
 *
 * @copyright Copyright (c) 2023 Sindre Eiklid
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
        {"time_call_processed", Utils::toDateTime},
        {"time_ambulance_notified", Utils::toDateTime},
        {"time_dispatch", Utils::toDateTime},
        {"time_arrival_scene", Utils::toDateTime},
        {"time_departure_scene", Utils::toDateTime},
        {"time_arrival_hospital", Utils::toDateTime},
        {"time_available", Utils::toDateTime},
        {"grid_id", Utils::toInt64},
        {"x", Utils::toInt},
        {"y", Utils::toInt},
        {"longitude", Utils::toFloat},
        {"latitude", Utils::toFloat},
        {"region", Utils::toString},
        {"urban_settlement", Utils::toBool}
    };
}

float Incidents::timeDifferenceBetweenHeaders(const std::string& header1, const std::string& header2, unsigned index) {
    std::tm time1 = get<std::optional<std::tm>>(header1, index).value();
    std::tm time2 = get<std::optional<std::tm>>(header2, index).value();

    return Utils::timeDifferenceInSeconds(time1, time2);
}

Incidents Incidents::rowsWithinTimeFrame(const int month, const int day, const unsigned windowSize) {
    Incidents filteredIncidents;

    // setup target date for comparison using a common year for all calculations
    std::tm targetTm = {};
    targetTm.tm_year = 120;  // uear 2020, a leap year
    targetTm.tm_mon = month - 1;
    targetTm.tm_mday = day;
    targetTm.tm_hour = 0;
    targetTm.tm_min = 0;
    targetTm.tm_sec = 0;
    targetTm.tm_isdst = -1;  // prevent DST affecting the calculation
    mktime(&targetTm);

    int targetDayOfYear = targetTm.tm_yday;

    for (std::size_t i = 0; i < rows.size(); ++i) {
        std::tm timeCallReceived = get<std::optional<std::tm>>("time_call_received", i).value();

        timeCallReceived.tm_year = 120;  // normalize year for comparison
        timeCallReceived.tm_isdst = -1;  // prevent DST affecting the calculation
        mktime(&timeCallReceived);

        int incidentDayOfYear = timeCallReceived.tm_yday;

        int dayDiff = std::abs(incidentDayOfYear - targetDayOfYear);

        bool withinWindowSize = dayDiff <= windowSize;

        if (withinWindowSize) {
            filteredIncidents.rows.push_back(rows[i]);
        }
    }

    // copy headers and schemaMapping from the original Incidents to the filteredIncidents
    filteredIncidents.headers = this->headers;
    filteredIncidents.schemaMapping = this->schemaMapping;

    return filteredIncidents;
}

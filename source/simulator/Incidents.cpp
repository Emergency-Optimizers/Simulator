/**
 * @file Incidents.cpp
 * @author Sindre Eiklid
 * @version 1.0
 * @date 2023-11-08
 *
 * @copyright Copyright (c) 2023 Sindre Eiklid
 */

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

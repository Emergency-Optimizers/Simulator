/**
 * @file Incidents.cpp
 * @author Sindre Eiklid
 * @version 1.0
 * @date 2023-11-08
 *
 * @copyright Copyright (c) 2023 Sindre Eiklid
 */

/* internal libraries */
#include "Incidents.hpp"
#include "Utils.hpp"

Incidents::Incidents() {
    schemaMapping = {
        {"id", Utils::toInt},
        {"synthetic", Utils::toBool},
        {"triage_impression_during_call", Utils::toString},
        {"time_call_received", Utils::toDateTime},
        {"time_call_processed", Utils::toDateTime},
        {"time_ambulance_notified", Utils::toDateTime},
        {"time_dispatch", Utils::toDateTime},
        {"time_arrival_scene", Utils::toDateTime},
        {"time_departure_scene", Utils::toDateTime},
        {"time_arrival_hospital", Utils::toDateTime},
        {"time_available", Utils::toDateTime},
        {"response_time_sec", Utils::toFloat},
        {"longitude", Utils::toFloat},
        {"latitude", Utils::toFloat},
        {"easting", Utils::toInt},
        {"northing", Utils::toInt},
        {"grid_id", Utils::toInt},
        {"grid_row", Utils::toInt},
        {"grid_col", Utils::toInt},
        {"region", Utils::toString},
        {"urban_settlement", Utils::toBool},
    };
}

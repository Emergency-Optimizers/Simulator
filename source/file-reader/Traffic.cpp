/**
 * @file Traffic.cpp
 *
 * @copyright Copyright (c) 2024 Emergency-Optimizers
 */

/* external libraries */
#include <iostream>
/* internal libraries */
#include "file-reader/Traffic.hpp"

Traffic::Traffic() {
    // define the schema: header and function that converts string to specific type
    schemaMapping = {
        {"Monday", toDouble},
        {"Tuesday", toDouble},
        {"Wednesday", toDouble},
        {"Thursday", toDouble},
        {"Friday", toDouble},
        {"Saturday", toDouble},
        {"Sunday", toDouble},
    };

    loadFromFile("../../Data-Processing/data/oslo/traffic.csv", "Loading traffic data");
}

double Traffic::getTrafficFactor(const time_t& time) {
    std::tm localTime = getLocalTime(time);

    const char* daysOfWeek[] = {"Sunday", "Monday", "Tuesday", "Wednesday", "Thursday", "Friday", "Saturday"};

    // returns the traffic factor based on day of week, and hour of day
    // traffic factor is used with the OD cost matrix to increase realism
    return get<double>(daysOfWeek[localTime.tm_wday], localTime.tm_hour);
}

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

    return get<double>(daysOfWeek[localTime.tm_wday], localTime.tm_hour);
}

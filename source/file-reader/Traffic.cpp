/**
 * @file Traffic.cpp
 *
 * @copyright Copyright (c) 2024 Emergency-Optimizers
 */

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

    std::cout << "Loading Traffic..." << std::flush;
    loadFromFile("../../Data-Processing/data/oslo/traffic.csv");
    std::cout << "\rLoading Traffic... " << "\tDONE" << std::endl;
}

double Traffic::getTrafficFactor(const time_t& time) {
    std::tm* localTime = std::localtime(&time);

    const char* daysOfWeek[] = {"Sunday", "Monday", "Tuesday", "Wednesday", "Thursday", "Friday", "Saturday"};

    return get<double>(daysOfWeek[localTime->tm_wday], localTime->tm_hour);
}

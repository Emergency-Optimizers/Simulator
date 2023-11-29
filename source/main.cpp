/**
 * @file main.cpp
 * @author Sindre Eiklid
 * @version 1.0
 * @date 2023-11-07
 *
 * @copyright Copyright (c) 2023 Sindre Eiklid
 */

/* external libraries */
#include <iostream>
/* internal libraries */
#include "ODMatrix.hpp"
#include "Incidents.hpp"
#include "Stations.hpp"
#include "EventHandler.hpp"

/**
 * Main program.
 */
int main() {
    ODMatrix odMatrix;
    odMatrix.loadFromFile("../../Data-Processing/data/oslo/od_matrix.txt");

    Stations stations;
    stations.loadFromFile("../../Data-Processing/data/enhanced/oslo/depots.csv");

    Incidents incidents;
    incidents.loadFromFile("../../Data-Processing/data/enhanced/oslo/incidents.csv");

    EventHandler eventHandler;
    eventHandler.populate(incidents, "2016.11.29T11:01:07", "2016.11.29T12:00:50");

    const std::vector<std::tm> times = incidents.getColumnOfTimes("time_call_received");
    int index = Utils::findClosestTimeIndex(Utils::stringToTm("2016.11.29T11:02:00"), times);
    incidents.printRow(index);

    return 0;
}

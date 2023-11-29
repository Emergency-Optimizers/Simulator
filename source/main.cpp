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
    std::cout << "Travel time from id1 to id2: " << odMatrix.getTravelTime(-1269655260, 852853940) << std::endl;

    Stations stations;
    stations.loadFromFile("../../Data-Processing/data/enhanced/oslo/depots.csv");
    stations.printRow(2);

    Incidents incidents;
    incidents.loadFromFile("../../Data-Processing/data/enhanced/oslo/incidents.csv");
    incidents.printRow(99);

    EventHandler eventHandler;
    eventHandler.populate(incidents, "2016.11.29T11:01:07", "2016.11.29T12:00:50");

    return 0;
}

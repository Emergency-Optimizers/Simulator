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

/**
 * Main program.
 */
int main() {
    ODMatrix odMatrix;
    odMatrix.loadFromFile("../../Data-Processing/data/oslo/od_matrix.txt");
    std::cout << "Travel time from id1 to id2: " << odMatrix.getTravelTime(-1269655260, 852853940) << std::endl;

    Incidents incidents;
    incidents.loadFromFile("../../Data-Processing/data/enhanced/oslo/incidents.csv");
    incidents.printRow(99);

    return 0;
}

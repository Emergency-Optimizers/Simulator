/**
 * @file Stations.cpp
 * @author Sindre Eiklid
 * @version 1.0
 * @date 2023-11-08
 *
 * @copyright Copyright (c) 2023 Sindre Eiklid
 */

/* internal libraries */
#include "Stations.hpp"

Stations::Stations() {
    schemaMapping = {
        {"type", Utils::toString},
        {"static", Utils::toBool},
        {"longitude", Utils::toFloat},
        {"latitude", Utils::toFloat},
        {"easting", Utils::toInt},
        {"northing", Utils::toInt},
        {"grid_id", Utils::toInt},
        {"grid_row", Utils::toInt},
        {"grid_col", Utils::toInt},
    };
}

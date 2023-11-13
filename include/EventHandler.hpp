/**
 * @file EventHandler.hpp
 * @author Sindre Eiklid
 * @version 1.0
 * @date 2023-11-13
 *
 * @copyright Copyright (c) 2023 Sindre Eiklid
 */

#pragma once

/* external libraries */
#include <string>
#include <vector>
/* internal libraries */
#include "Incidents.hpp"

class EventHandler {
 protected:
    std::vector<int> events;

 public:
    ~EventHandler() = default;
    void populate(Incidents& incidents, const std::string& start, const std::string& end);
};

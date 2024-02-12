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
#include "simulator/Incidents.hpp"
#include "simulator/EventOld.hpp"

class EventHandler {
 private:
    int currentIndex;

 public:
    ~EventHandler() = default;
    std::vector<EventOld> events;
    void populate(Incidents& incidents, const std::string& start, const std::string& end);
    int getNextEventIndex();
    void sortEvent(size_t eventIndex);
};

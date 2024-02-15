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
#include "simulator/Event.hpp"

class EventHandler {
 private:
    int currentIndex = 0;

 public:
    std::vector<Event> events;

    ~EventHandler() = default;
    void populate(std::vector<Event> newEvents);
    int getNextEventIndex();
    void sortEvent(size_t eventIndex);
};

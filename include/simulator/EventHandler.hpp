/**
 * @file EventHandler.hpp
 *
 * @copyright Copyright (c) 2024 Emergency-Optimizers
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

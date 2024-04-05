/**
 * @file EventHandler.hpp
 *
 * @copyright Copyright (c) 2024 Emergency-Optimizers
 */

#pragma once

/* external libraries */
#include <vector>
/* internal libraries */
#include "simulator/Event.hpp"

class EventHandler {
 private:
    int currentIndex = 0;

 public:
    std::vector<Event>& events = std::vector<Event>(0);

    EventHandler();
    ~EventHandler() = default;
    void populate(std::vector<Event> newEvents);
    int getNextEventIndex();
    void sortEvent(size_t eventIndex);
    void sortEvents();
};

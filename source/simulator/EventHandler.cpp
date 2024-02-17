/**
 * @file EventHandler.cpp
 *
 * @copyright Copyright (c) 2024 Emergency-Optimizers
 */

/* external libraries */
#include <ctime>
/* internal libraries */
#include "simulator/EventHandler.hpp"
#include "Utils.hpp"

void EventHandler::populate(std::vector<Event> newEvents) {
    events = newEvents;
}

int EventHandler::getNextEventIndex() {
    for (; currentIndex < events.size(); currentIndex++) {
        if (events[currentIndex].type != EventType::NONE) return currentIndex;
    }

    return -1;
}

void EventHandler::sortEvent(size_t eventIndex) {
    std::vector<Event>::iterator newPosition = std::lower_bound(
        events.begin() + eventIndex,
        events.end(),
        events[eventIndex],
        [](const Event& a, const Event& b) { return a.timer < b.timer; }
    );

    if (newPosition != events.begin() + eventIndex) {
        std::rotate(events.begin() + eventIndex, events.begin() + eventIndex + 1, newPosition);
    }
}

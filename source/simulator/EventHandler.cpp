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

EventHandler::EventHandler(std::vector<Event>& events) : events(events) {
    sortEvents();
}

int EventHandler::getNextEventIndex() {
    // iterate through queue until event which isn't finsihed appears
    for (; currentIndex < events.size(); currentIndex++) {
        if (events[currentIndex].type != EventType::NONE) return currentIndex;
    }

    return -1;
}

void EventHandler::sortEvent(size_t eventIndex) {
    // only sort specific event by moving it backwards in queue if needed
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

void EventHandler::sortEvents() {
    // sort all events
    std::sort(events.begin(), events.end(), [](const Event& a, const Event& b) {
        return a.timer < b.timer;
    });

    for (int i = 0; i < events.size(); i++) {
        if (events[i].type != EventType::NONE) {
            currentIndex = i;
            break;
        }
    }
}

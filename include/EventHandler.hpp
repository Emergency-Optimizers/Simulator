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
#include "Event.hpp"

class EventHandler {
 private:
   std::vector<Event> events;
   int currentIndex;

 public:
   ~EventHandler() = default;
   void populate(Incidents& incidents, const std::string& start, const std::string& end);
   int getNextEventIndex();
   void sort(size_t eventIndex);
};

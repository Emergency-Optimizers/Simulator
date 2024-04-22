/**
 * @file Utils.cpp
 *
 * @copyright Copyright (c) 2024 Emergency-Optimizers
 */

/* external libraries */
#include <iostream>
#include <fstream>
#include <sstream>
#include <iomanip>
#include <algorithm>
#include <numeric>
#include <chrono>
#include <filesystem>
/* internal libraries */
#include "Utils.hpp"
#include "Constants.hpp"
#include "file-reader/ODMatrix.hpp"
#include "file-reader/Incidents.hpp"
#include "file-reader/Stations.hpp"

ValueType toInt(const std::string& str) {
    return std::stoi(str);
}

ValueType toInt64(const std::string& str) {
    return std::stoll(str);
}

ValueType toFloat(const std::string& str) {
    return std::stof(str);
}

ValueType toDouble(const std::string& str) {
    return std::stod(str);
}

ValueType toString(const std::string& str) {
    return str;
}

ValueType toBool(const std::string& str) {
    return str == "True" || str == "true";
}

ValueType toDateTime(const std::string& str) {
    if (str.empty()) {
        return std::nullopt;
    }

    std::tm tm = {};
    std::istringstream ss(str);
    ss >> std::get_time(&tm, "%Y-%m-%d %H:%M:%S");

    if (ss.fail()) {
        return std::nullopt;
    }

    return tm;
}

ValueType toVectorFloat(const std::string& str) {
    std::vector<float> result;
    std::stringstream ss(str);

    std::string item;
    while (getline(ss, item, ',')) {
        // trim spaces
        item.erase(0, item.find_first_not_of(" \t"));
        item.erase(item.find_last_not_of(" \t") + 1);

        try {
            float value = std::stof(item);
            result.push_back(value);
        } catch (const std::exception& e) {
            std::cerr << "Error parsing float from settings: " << e.what() << std::endl;
        }
    }

    return result;
}

ValueType toDispatchEngineStrategyType(const std::string& str) {
    if (str == "RANDOM") {
        return DispatchEngineStrategyType::RANDOM;
    } else if (str == "CLOSEST") {
        return DispatchEngineStrategyType::CLOSEST;
    } else {
        std::cout << "Unknown dispatch engine type, defaulting to random" << std::endl;
        return DispatchEngineStrategyType::RANDOM;
    }
}

ValueType toCrossoverType(const std::string& str) {
    if (str == "SINGLE_POINT") {
        return CrossoverType::SINGLE_POINT;
    } else {
        std::cout << "Unknown crossover type, defaulting to single point" << std::endl;
        return CrossoverType::SINGLE_POINT;
    }
}

ValueType toVectorObjectiveType(const std::string& str) {
    std::vector<ObjectiveTypes> result;
    std::stringstream ss(str);
    std::string item;

    while (getline(ss, item, ',')) {
        item.erase(0, item.find_first_not_of(" \t"));
        item.erase(item.find_last_not_of(" \t") + 1);

        ObjectiveTypes type = stringToObjectiveType(item);
        result.push_back(type);
    }

    return result;
}

ObjectiveTypes stringToObjectiveType(const std::string& str) {
    if (str == "AVG_RESPONSE_TIME_URBAN_A") {
        return ObjectiveTypes::AVG_RESPONSE_TIME_URBAN_A;
    } else if (str == "AVG_RESPONSE_TIME_URBAN_H") {
        return ObjectiveTypes::AVG_RESPONSE_TIME_URBAN_H;
    } else if (str == "AVG_RESPONSE_TIME_URBAN_V1") {
        return ObjectiveTypes::AVG_RESPONSE_TIME_URBAN_V1;
    } else if (str == "AVG_RESPONSE_TIME_RURAL_A") {
        return ObjectiveTypes::AVG_RESPONSE_TIME_RURAL_A;
    } else if (str == "AVG_RESPONSE_TIME_RURAL_H") {
        return ObjectiveTypes::AVG_RESPONSE_TIME_RURAL_H;
    } else if (str == "AVG_RESPONSE_TIME_RURAL_V1") {
        return ObjectiveTypes::AVG_RESPONSE_TIME_RURAL_V1;
    } else if (str == "PERCENTAGE_VIOLATIONS") {
        return ObjectiveTypes::PERCENTAGE_VIOLATIONS;
    } else if (str == "PERCENTAGE_VIOLATIONS_URBAN") {
        return ObjectiveTypes::PERCENTAGE_VIOLATIONS_URBAN;
    } else if (str == "PERCENTAGE_VIOLATIONS_RURAL") {
        return ObjectiveTypes::PERCENTAGE_VIOLATIONS_RURAL;
    } else {
        throwError("Unknown objective type: '" + str + "'");
        return ObjectiveTypes::PERCENTAGE_VIOLATIONS;
    }
}

std::string objectiveTypeToString(const ObjectiveTypes objective) {
    switch (objective) {
        case ObjectiveTypes::AVG_RESPONSE_TIME_URBAN_A:
            return "AVG_RESPONSE_TIME_URBAN_A";
        case ObjectiveTypes::AVG_RESPONSE_TIME_URBAN_H:
            return "AVG_RESPONSE_TIME_URBAN_H";
        case ObjectiveTypes::AVG_RESPONSE_TIME_URBAN_V1:
            return "AVG_RESPONSE_TIME_URBAN_V1";
        case ObjectiveTypes::AVG_RESPONSE_TIME_RURAL_A:
            return "AVG_RESPONSE_TIME_RURAL_A";
        case ObjectiveTypes::AVG_RESPONSE_TIME_RURAL_H:
            return "AVG_RESPONSE_TIME_RURAL_H";
        case ObjectiveTypes::AVG_RESPONSE_TIME_RURAL_V1:
            return "AVG_RESPONSE_TIME_RURAL_V1";
        case ObjectiveTypes::PERCENTAGE_VIOLATIONS:
            return "PERCENTAGE_VIOLATIONS";
        case ObjectiveTypes::PERCENTAGE_VIOLATIONS_URBAN:
            return "PERCENTAGE_VIOLATIONS_URBAN";
        case ObjectiveTypes::PERCENTAGE_VIOLATIONS_RURAL:
            return "PERCENTAGE_VIOLATIONS_RURAL";
        default:
            return "UNKNOWN";
    }
}

ValueType toHeuristicType(const std::string& str) {
    if (str == "NONE") {
        return HeuristicType::GA;
    } else if (str == "GA") {
        return HeuristicType::NSGA2;
    } else if (str == "NSGA2") {
        return HeuristicType::MA;
    } else if (str == "MA") {
        return HeuristicType::MEMETIC_NSGA2;
    } else if (str == "MEMETIC_NSGA2") {
        return HeuristicType::MEMETIC_NSGA2;
    } else if (str == "CUSTOM") {
        return HeuristicType::CUSTOM;
    } else {
        throwError("Unknown heuristic type: '" + str + "'");
        return HeuristicType::NONE;
    }
}

std::string tmToString(const std::tm& time) {
    std::stringstream ss;
    ss << std::put_time(&time, "%Y-%m-%d %H:%M:%S");

    return ss.str();
}

std::string valueTypeToString(const ValueType& cell) {
    return std::visit([](auto&& arg) -> std::string {
        using T = std::decay_t<decltype(arg)>;
        if constexpr (std::is_same_v<T, int>) {
            return std::to_string(arg);
        } else if constexpr (std::is_same_v<T, int64_t>) {
            return std::to_string(arg);
        } else if constexpr (std::is_same_v<T, float>) {
            return std::to_string(arg);
        } else if constexpr (std::is_same_v<T, double>) {
            return std::to_string(arg);
        } else if constexpr (std::is_same_v<T, std::string>) {
            return arg;
        } else if constexpr (std::is_same_v<T, bool>) {
            return arg ? "true" : "false";
        } else if constexpr (std::is_same_v<T, std::optional<std::tm>>) {
            return arg ? tmToString(arg.value()) : "n/a";
        } else if constexpr (std::is_same_v<T, std::vector<float>>) {
            std::string result;
            for (size_t i = 0; i < arg.size(); ++i) {
                result += std::to_string(arg[i]);
                if (i < arg.size() - 1) {
                    result += ", ";
                }
            }
            return result;
        } else if constexpr (std::is_same_v<T, DispatchEngineStrategyType>) {
            if (arg == DispatchEngineStrategyType::RANDOM) {
                return "RANDOM";
            } else if (arg == DispatchEngineStrategyType::CLOSEST) {
                return "CLOSEST";
            } else {
                return "UNKNOWN";
            }
        } else if constexpr(std::is_same_v<T, CrossoverType>) {
            if (arg == CrossoverType::SINGLE_POINT) {
                return "SINGLE_POINT";
            } else {
                return "UNKNOWN";
            }
        } else if constexpr (std::is_same_v<T, std::vector<ObjectiveTypes>>) {
            std::string result;
            for (size_t i = 0; i < arg.size(); ++i) {
                result += objectiveTypeToString(arg[i]);
                if (i < arg.size() - 1) {
                    result += ", ";
                }
            }
            return result;
        } else if constexpr(std::is_same_v<T, HeuristicType>) {
            if (arg == HeuristicType::GA) {
                return "GA";
            } else {
                return "UNKNOWN";
            }
        }
    }, cell);
}

double timeDifferenceInSeconds(std::tm& time1, std::tm& time2) {
    time_t t1 = std::mktime(&time1);
    time_t t2 = std::mktime(&time2);

    return std::difftime(t2, t1);
}

std::vector<unsigned> getAvailableAmbulanceIndicies(
    std::vector<Ambulance>& ambulances,
    const std::vector<Event>& events,
    const time_t& currentTime,
    const std::string& currentEventTriageImpression
) {
    std::vector<unsigned> availableAmbulanceIndicies;

    for (unsigned i = 0; i < ambulances.size(); i++) {
        int eventIndex = -1;

        // find event index if ambulance is assigned to an event
        if (ambulances[i].assignedEventId != -1) {
            eventIndex = findEventIndexFromId(events, ambulances[i].assignedEventId);
        }

        if (ambulances[i].isAvailable(events, ambulances, eventIndex, currentTime, currentEventTriageImpression)) {
            availableAmbulanceIndicies.push_back(i);
        }
    }

    return availableAmbulanceIndicies;
}

int calculateDayDifference(const std::tm& baseDate, const int targetMonth, const int targetDay) {
    int year = baseDate.tm_year;
    bool isLeapYear = (year % 4 == 0) && ((year % 100 != 0) || (year % 400 == 0));
    int totalDaysInYear = isLeapYear ? 365 : 364;

    std::tm baseDateNormalized = baseDate;
    baseDateNormalized.tm_hour = 0;
    baseDateNormalized.tm_min = 0;
    baseDateNormalized.tm_sec = 0;
    mktime(&baseDateNormalized);

    // create tm structure for the target date in the same year as the base date
    std::tm targetDate = {0};
    targetDate.tm_year = year;
    targetDate.tm_mon = targetMonth - 1;
    targetDate.tm_mday = targetDay;
    targetDate.tm_hour = 0;
    targetDate.tm_min = 0;
    targetDate.tm_sec = 0;
    mktime(&targetDate);

    int targetDayOfYear = targetDate.tm_yday;
    int baseDayOfYear = baseDateNormalized.tm_yday;
    int dayDiff = 0;

    if (targetDayOfYear == baseDayOfYear) {
        return 0;
    }

    while (true) {
        if (targetDayOfYear == baseDayOfYear) {
            break;
        }
        if (++baseDayOfYear > totalDaysInYear) {
            baseDayOfYear = 0;
        }
        dayDiff++;
    }

    targetDayOfYear = targetDate.tm_yday;
    baseDayOfYear = baseDateNormalized.tm_yday;
    int newDayDiff = 0;

    while (dayDiff != newDayDiff) {
        if (targetDayOfYear == baseDayOfYear) {
            break;
        }
        if (--baseDayOfYear < 0) {
            baseDayOfYear = totalDaysInYear;
        }
        newDayDiff++;
    }

    if (dayDiff > newDayDiff) {
        dayDiff = newDayDiff;
    }

    return dayDiff;
}

int weightedLottery(
    std::mt19937& rnd,
    const std::vector<double>& weights,
    const std::vector<std::pair<int, int>>& ranges
) {
    if (weights.empty()) {
        return -1;
    }

    if (weights.size() == 1) {
        return 0;
    }

    std::vector<double> adjustedWeights;

    if (ranges.empty()) {
        // if ranges is empty, use the entire weights vector
        adjustedWeights = weights;
    } else {
        // otherwise, process each specified range
        for (const auto& range : ranges) {
            int beginIndex = std::max(0, range.first);
            int endIndex = std::min(static_cast<int>(weights.size()) - 1, range.second);

            if (beginIndex <= endIndex) {
                adjustedWeights.insert(
                    adjustedWeights.end(),
                    weights.begin() + beginIndex,
                    weights.begin() + endIndex + 1
                );
            }
        }
    }

    // create a partial sum of the adjusted weights
    std::vector<double> cumulativeWeights(adjustedWeights.size());
    std::partial_sum(adjustedWeights.begin(), adjustedWeights.end(), cumulativeWeights.begin());

    // generate a random number in the adjusted range
    std::uniform_real_distribution<> dist(0.0, cumulativeWeights.back());
    auto it = std::lower_bound(cumulativeWeights.begin(), cumulativeWeights.end(), dist(rnd));
    int indexInAdjustedWeights = static_cast<int>(std::distance(cumulativeWeights.begin(), it));

    // for the default case (full range), the adjusted index is the final result
    if (ranges.empty()) {
        return indexInAdjustedWeights;
    }

    // adjust the index to map back to the original weights vector for specified ranges
    int originalIndex = 0;
    for (const auto& range : ranges) {
        int rangeSize = range.second - range.first + 1;
        if (indexInAdjustedWeights < rangeSize) {
            originalIndex += range.first + indexInAdjustedWeights;
            break;
        } else {
            indexInAdjustedWeights -= rangeSize;
        }
    }

    return originalIndex;
}

int getRandomInt(std::mt19937& rnd, const int min, const int max) {
    std::uniform_int_distribution<> dist(min, max);

    return dist(rnd);
}

void writeEvents(const std::string& dirName, std::vector<Event>& events) {
    createDirectory(dirName);
    std::string filename = "../data/" + dirName + "/" + "events" + ".csv";
    std::ofstream outFile(filename);

    // check if the file stream is open before proceeding
    if (!outFile.is_open()) {
        std::cerr << "Failed to open file: " << filename << std::endl;
        return;
    }

    // sort events
    std::sort(events.begin(), events.end(), [](const Event& a, const Event& b) {
        std::tm aTimeStruct = a.callReceived;
        std::tm bTimeStruct = b.callReceived;

        time_t aTime = std::mktime(&aTimeStruct);
        time_t bTime = std::mktime(&bTimeStruct);

        return aTime < bTime;
    });

    // write CSV header
    outFile
        << "time_call_received" << ","
        << "triage_impression_during_call" << ","
        << "grid_id" << ","
        << "urban" << ","
        << "dispatched_from_depot_index" << ","
        << "duration_incident_creation" << ","
        << "duration_resource_appointment" << ","
        << "duration_resource_preparing_departure" << ","
        << "duration_dispatching_to_scene" << ","
        << "duration_at_scene" << ","
        << "duration_dispatching_to_hospital" << ","
        << "duration_at_hospital" << ","
        << "duration_dispatching_to_depot" << std::endl;

    for (Event& event : events) {
        if (event.utility) {
            continue;
        }
        // write each metric to the CSV
        outFile
            << tmToString(event.callReceived) << ","
            << event.triageImpression << ","
            << std::to_string(event.incidentGridId) << ","
            << (Incidents::getInstance().gridIdUrban[event.incidentGridId] ? "True" : "False") << ","
            << std::to_string(event.depotIndexResponsible) << ","
            << std::to_string(event.metrics["duration_incident_creation"]) << ","
            << std::to_string(event.metrics["duration_resource_appointment"]) << ","
            << std::to_string(event.metrics["duration_resource_preparing_departure"]) << ","
            << std::to_string(event.metrics["duration_dispatching_to_scene"]) << ","
            << std::to_string(event.metrics["duration_at_scene"]) << ","
            << std::to_string(event.metrics["duration_dispatching_to_hospital"]) << ","
            << std::to_string(event.metrics["duration_at_hospital"]) << ","
            << std::to_string(event.metrics["duration_dispatching_to_depot"]) << std::endl;
    }

    outFile.close();
}


void saveDistributionToFile(const std::vector<std::vector<double>>& distribution, const std::string& baseFilename) {
    auto now = std::chrono::system_clock::now();
    std::time_t now_time = std::chrono::system_clock::to_time_t(now);

    // format current time as string (YYYY-MM-DD_HH-MM-SS)
    std::tm bt = getLocalTime(now_time);
    std::ostringstream oss;
    oss << std::put_time(&bt, "%Y-%m-%d_%H-%M-%S");

    // construct filename with the current date and time
    std::string filename =  "../data/distributions/" + baseFilename + "_" + oss.str() + ".csv";

    std::ofstream outFile(filename);
    if (!outFile.is_open()) {
        std::cerr << "Failed to open file: " << filename << std::endl;
        return;
    }

    // write header
    outFile << "A,H,V1\n";

    // write vector to file without trailing commas
    for (const auto& row : distribution) {
        for (size_t i = 0; i < row.size(); ++i) {
            outFile << row[i];
            if (i < row.size() - 1) outFile << ",";
        }
        outFile << "\n";
    }
}

void save1dDistributionToFile(const std::vector<double>& distribution, const std::string& baseFilename) {
    auto now = std::chrono::system_clock::now();
    std::time_t now_time = std::chrono::system_clock::to_time_t(now);

    // format current time as a string (YYYY-MM-DD_HH-MM-SS)
    std::tm bt = getLocalTime(now_time);
    std::ostringstream oss;
    oss << std::put_time(&bt, "%Y-%m-%d_%H-%M-%S");

    // construct filename with the current date and time
    std::string filename = "../data/distributions/" + baseFilename + "_" + oss.str() + ".csv";

    // write vector to file
    std::ofstream outFile(filename);
    if (!outFile.is_open()) {
        std::cerr << "Failed to open file: " << filename << std::endl;
        return;
    }
    for (const auto& value : distribution) {
        outFile << value << ",";
    }
}

void save2dDistributionToFile(const std::vector<std::vector<double>>& distribution, const std::string& baseFilename) {
    auto now = std::chrono::system_clock::now();
    std::time_t now_time = std::chrono::system_clock::to_time_t(now);

    // format current time as string (YYYY-MM-DD_HH-MM-SS)
    std::tm bt = getLocalTime(now_time);
    std::ostringstream oss;
    oss << std::put_time(&bt, "%Y-%m-%d_%H-%M-%S");

    // construct filename with the current date and time
    std::string filename =  "../data/distributions/" + baseFilename + "_" + oss.str() + ".csv";

    // write vector to file
    std::ofstream outFile(filename);
    if (!outFile.is_open()) {
        std::cerr << "Failed to open file: " << filename << std::endl;
        return;
    }
    for (const auto& row : distribution) {
        for (const auto& value : row) {
            outFile << value << ",";
        }
        outFile << "\n";
    }
}

double getRandomDouble(std::mt19937& rnd, const double min, const double max) {
    std::uniform_real_distribution<> dist(min, max);

    return dist(rnd);
}

bool getRandomBool(std::mt19937& rnd) {
    return getRandomDouble(rnd) < 0.5;
}

double calculateMean(const std::vector<int>& numbers) {
    double sum = 0.0;

    for (int num : numbers) {
        sum += num;
    }

    return numbers.empty() ? 0.0 : sum / static_cast<double>(numbers.size());
}

double calculateStandardDeviation(const std::vector<int>& numbers) {
    double mean = calculateMean(numbers);
    double varianceSum = 0.0;

    for (int num : numbers) {
        varianceSum += std::pow(static_cast<double>(num) - mean, 2.0);
    }
    double variance = numbers.empty() ? 0.0 : varianceSum / static_cast<double>(numbers.size());

    return std::sqrt(variance);
}

double calculateEuclideanDistance(const double x1, const double y1, const double x2, const double y2) {
    return std::sqrt(std::pow(x2 - x1, 2.0) + std::pow(y2 - y1, 2.0));
}

std::pair<int, int> idToUtm(const int64_t& grid_id) {
    int x = static_cast<int>(std::floor(static_cast<double>(grid_id) * std::pow(10, -7)) - (2 * std::pow(10, 6)));
    int y = static_cast<int>(static_cast<double>(grid_id) - (std::floor(static_cast<double>(grid_id) * std::pow(10, -7)) * std::pow(10, 7)));

    return std::make_pair(x, y);
}

int64_t utmToId(const std::pair<int, int>& utm, const int cellSize, const int offset) {
    int64_t xCorner = static_cast<int64_t>(std::floor((utm.first + offset) / cellSize) * cellSize - offset);
    int64_t yCorner = static_cast<int64_t>(std::floor(utm.second / cellSize) * cellSize);

    return 20000000000000 + (xCorner * 10000000) + yCorner;
}

int64_t approximateLocation(
    std::mt19937& rnd,
    const int64_t& startId,
    const int64_t& goalId,
    const time_t& timeAtStart,
    const time_t& timeNow,
    const std::string& triage,
    const EventType eventType
) {
    // if ambulance is dispatching to depot, apply traffic, otherwise check triage
    bool forceTrafficFactor = true;
    if (eventType == EventType::DISPATCHING_TO_SCENE) {
        forceTrafficFactor = false;
    }

    int timeToReachGoal = ODMatrix::getInstance().getTravelTime(
        rnd,
        startId,
        goalId,
        forceTrafficFactor,
        triage,
        timeAtStart
    );

    time_t timeTravelled = timeNow - timeAtStart;

    double proportion = static_cast<double>(timeTravelled) / static_cast<double>(timeToReachGoal);

    std::pair<int, int> utmStart = idToUtm(startId);
    std::pair<int, int> utmGoal = idToUtm(goalId);

    std::pair<int, int> utmInterpolated = {
        static_cast<int>(static_cast<double>(utmStart.first) + static_cast<double>(utmGoal.first - utmStart.first) * proportion),
        static_cast<int>(static_cast<double>(utmStart.second) + static_cast<double>(utmGoal.second - utmStart.second) * proportion)
    };

    int64_t approximatedGridId = utmToId(utmInterpolated);

    /*std::cout
        << startId << " -> " << goalId << " = " << approximatedGridId << " ("
        << proportion * 100 << "% ("
        << timeTravelled << "->" << timeToReachGoal
        << "))" << std::endl;*/

    return approximatedGridId;
}

int findEventIndexFromId(const std::vector<Event>& events, const int id) {
    for (int i = 0; i < events.size(); i++) {
        if (events[i].id == id) {
            return i;
        }
    }

    return -1;
}

bool isDayShift(const time_t& eventTimer, const int dayShiftStart, const int dayShiftEnd) {
    std::tm timeInfo = getLocalTime(eventTimer);
    int hour = timeInfo.tm_hour;

    return hour >= dayShiftStart && hour <= dayShiftEnd;
}

std::string eventTypeToString(EventType eventType) {
    switch (eventType) {
        case EventType::NONE: return "NONE";
        case EventType::RESOURCE_APPOINTMENT: return "RESOURCE_APPOINTMENT";
        case EventType::DISPATCHING_TO_SCENE: return "DISPATCHING_TO_SCENE";
        case EventType::DISPATCHING_TO_HOSPITAL: return "DISPATCHING_TO_HOSPITAL";
        case EventType::PREPARING_DISPATCH_TO_DEPOT: return "PREPARING_DISPATCH_TO_DEPOT";
        case EventType::DISPATCHING_TO_DEPOT: return "DISPATCHING_TO_DEPOT";
        case EventType::REALLOCATE: return "REALLOCATE";
        default: return "UNKNOWN";
    }
}

double averageResponseTime(
    std::vector<Event>& simulatedEvents,
    const std::string& triageImpression,
    const bool urban,
    const int allocationIndex,
    const int depotIndex
) {
    int totalEvents = 0;
    int totalResponseTime = 0;

    int eventIndex = 0;
    int maxEventIndex = static_cast<int>(simulatedEvents.size());

    if (allocationIndex != -1) {
        int allocationCount = 0;

        for (int i = 0; i < simulatedEvents.size(); i++) {
            Event event = simulatedEvents[i];

            if (event.reallocation.empty()) {
                continue;
            }

            if (allocationIndex == allocationCount) {
                maxEventIndex = i + 1;

                break;
            } else {
                allocationCount++;
                eventIndex = i + 1;
            }
        }
    }

    for (; eventIndex < maxEventIndex; eventIndex++) {
        Event event = simulatedEvents[eventIndex];

        if (event.utility) {
            continue;
        }

        if (depotIndex != -1 && event.depotIndexResponsible != depotIndex) {
            continue;
        }

        if (event.triageImpression != triageImpression || Incidents::getInstance().gridIdUrban[event.incidentGridId] != urban) {
            continue;
        }

        totalResponseTime += event.getResponseTime();
        totalEvents++;
    }

    if (totalEvents == 0) {
        return 0;
    }

    return static_cast<double>(totalResponseTime) / static_cast<double>(totalEvents);
}

double responseTimeViolations(
    std::vector<Event>& simulatedEvents,
    const int allocationIndex,
    const int depotIndex
) {
    int eventIndex = 0;
    int maxEventIndex = static_cast<int>(simulatedEvents.size());

    if (allocationIndex != -1) {
        int allocationCount = 0;

        for (int i = 0; i < simulatedEvents.size(); i++) {
            Event event = simulatedEvents[i];

            if (event.reallocation.empty()) {
                continue;
            }

            if (allocationIndex == allocationCount) {
                maxEventIndex = i + 1;

                break;
            } else {
                allocationCount++;
                eventIndex = i + 1;
            }
        }
    }

    double totalEvents = 0.0;
    double totalViolations = 0.0;

    const int urbanAcuteResponseTimeGoalSeconds = 720;
    const int ruralAcuteResponseTimeGoalSeconds = 1500;
    const int urbanUrgentResponseTimeGoalSeconds = 1800;
    const int ruralUrgentResponseTimeGoalSeconds = 2400;

    for (; eventIndex < maxEventIndex; eventIndex++) {
        if (simulatedEvents[eventIndex].utility) {
            continue;
        }

        if (depotIndex != -1 && simulatedEvents[eventIndex].depotIndexResponsible != depotIndex) {
            totalEvents++;
            continue;
        }

        int responseTime = simulatedEvents[eventIndex].getResponseTime();

        bool urban = Incidents::getInstance().gridIdUrban[simulatedEvents[eventIndex].incidentGridId];
        std::string triage = simulatedEvents[eventIndex].triageImpression;

        if (triage == "A") {
            if (urban && responseTime > urbanAcuteResponseTimeGoalSeconds) {
                totalViolations++;
            } else if (!urban && responseTime > ruralAcuteResponseTimeGoalSeconds) {
                totalViolations++;
            }
        } else if (triage == "H") {
            if (urban && responseTime > urbanUrgentResponseTimeGoalSeconds) {
                totalViolations++;
            } else if (!urban && responseTime > ruralUrgentResponseTimeGoalSeconds) {
                totalViolations++;
            }
        }

        totalEvents++;
    }

    if (totalEvents == 0.0) {
        return 0.0;
    }

    return totalViolations / totalEvents;
}

double responseTimeViolationsUrban(
    std::vector<Event>& simulatedEvents,
    const bool checkUrban,
    const int allocationIndex,
    const int depotIndex
) {
    int eventIndex = 0;
    int maxEventIndex = static_cast<int>(simulatedEvents.size());

    if (allocationIndex != -1) {
        int allocationCount = 0;

        for (int i = 0; i < simulatedEvents.size(); i++) {
            Event event = simulatedEvents[i];

            if (event.reallocation.empty()) {
                continue;
            }

            if (allocationIndex == allocationCount) {
                maxEventIndex = i + 1;

                break;
            } else {
                allocationCount++;
                eventIndex = i + 1;
            }
        }
    }

    double totalEvents = 0.0;
    double totalViolations = 0.0;

    const int urbanAcuteResponseTimeGoalSeconds = 720;
    const int ruralAcuteResponseTimeGoalSeconds = 1500;
    const int urbanUrgentResponseTimeGoalSeconds = 1800;
    const int ruralUrgentResponseTimeGoalSeconds = 2400;

    for (; eventIndex < maxEventIndex; eventIndex++) {
        if (simulatedEvents[eventIndex].utility) {
            continue;
        }

        bool urban = Incidents::getInstance().gridIdUrban[simulatedEvents[eventIndex].incidentGridId];
        if (urban != checkUrban) {
            continue;
        }

        if (depotIndex != -1 && simulatedEvents[eventIndex].depotIndexResponsible != depotIndex) {
            totalEvents++;
            continue;
        }

        int responseTime = simulatedEvents[eventIndex].getResponseTime();
        std::string triage = simulatedEvents[eventIndex].triageImpression;

        if (triage == "A") {
            if (urban && responseTime > urbanAcuteResponseTimeGoalSeconds) {
                totalViolations++;
            } else if (!urban && responseTime > ruralAcuteResponseTimeGoalSeconds) {
                totalViolations++;
            }
        } else if (triage == "H") {
            if (urban && responseTime > urbanUrgentResponseTimeGoalSeconds) {
                totalViolations++;
            } else if (!urban && responseTime > ruralUrgentResponseTimeGoalSeconds) {
                totalViolations++;
            }
        }

        totalEvents++;
    }

    if (totalEvents == 0.0) {
        return 0.0;
    }

    return totalViolations / totalEvents;
}

void printTimeSegmentedAllocationTable(
    const bool dayShift,
    const int numTimeSegments,
    const std::vector<std::vector<int>>& allocations,
    std::vector<Event>& simulatedEvents,
    const std::vector<double>& allocationsFitness
) {
    std::cout << "\n";
    std::vector<std::string> depotNames;

    std::vector<unsigned int> depotIndicies = Stations::getInstance().getDepotIndices(dayShift);
    for (int i = 0; i < depotIndicies.size(); i++) {
        std::string depotName;

        // hard coded due to norwegian characters
        if (depotIndicies[i] == 3) {
            depotName = "Aurskog-Holand";
        } else if (depotIndicies[i] == 5) {
            depotName = "Lorenskog";
        } else if (depotIndicies[i] == 7) {
            depotName = "Baerum";
        } else if (depotIndicies[i] == 9) {
            depotName = "Ullevaal";
        } else if (depotIndicies[i] == 14) {
            depotName = "Sondre Follo";
        } else {
            depotName = Stations::getInstance().get<std::string>("name", depotIndicies[i]);
        }

        depotNames.push_back(depotName);
    }

    // print header
    std::cout << std::left << std::setw(6) << "Depot" << "|";
    for (int d = 0; d < depotIndicies.size(); ++d) {
        std::cout << std::right << std::setw(3) << d;
    }
    std::cout << std::setw(9) << " Fitness" << std::endl;

    std::cout << std::string(6, '-') << "+" << std::string((3 * depotIndicies.size()) + 9, '-') << std::endl;

    // print rows
    for (size_t t = 0; t < allocations.size(); ++t) {
        std::cout << std::right << std::setw(5) << "T" + std::to_string(t + 1) << " |";
        for (size_t d = 0; d < depotIndicies.size(); ++d) {
            std::cout << std::setw(3) << allocations[t][d];
        }

        // print fitness cell
        std::cout << std::fixed << std::setprecision(2) << std::setw(9) << allocationsFitness[t] << std::endl;
    }
}

void printAmbulanceWorkload(const std::vector<Ambulance>& ambulances) {
    const double TWELVE_HOURS = 43200;

    double totalHours = 0;
    std::vector<int> times;

    std::cout << std::endl;

    for (int i = 0; i < ambulances.size(); i++) {
        totalHours += (static_cast<double>(ambulances[i].timeUnavailable) / 60.0) / 60.0;
        times.push_back(ambulances[i].timeUnavailable);

        std::cout
            << "Ambulance " << std::setw(3) << (i + 1) << " UHU: "
            << std::fixed << std::setprecision(2) << std::setw(6)
            << ((static_cast<double>(ambulances[i].timeUnavailable) / TWELVE_HOURS) * 100.0) << "%";

        if ((i + 1) % 2 == 0 || i >= ambulances.size() - 1) {
            std::cout << std::endl;
        } else {
            std::cout << "  |  ";
        }
    }

    std::cout
        << std::endl << "Total: " << std::setprecision(2) << totalHours << " hours, "
        << "Standard deviation: " << calculateStandardDeviation(times)
        << std::endl << std::endl;
}

void throwError(const std::string& msg) {
    std::cerr << std::endl << "Error: " << msg << std::endl;

    // exit program gracefully (call destructors)
    std::exit(0);
}

void saveDataToJson(
    const std::string& dirName,
    const std::string& fileName,
    const std::map<std::string, std::vector<std::vector<double>>>& dataMap
) {
    createDirectory(dirName);
    std::ofstream outFile("../data/" + dirName + "/" + fileName + ".json");

    // set to the classic "C" locale to ensure numbers are formatted without thousands separators
    outFile.imbue(std::locale("C"));

    // begin JSON object
    outFile << "{" << std::endl;

    // iterator over map for serialization
    for (auto it = dataMap.begin(); it != dataMap.end(); ++it) {
        outFile << "  \"" << it->first << "\": [";
        for (size_t i = 0; i < it->second.size(); ++i) {
            outFile << "[";
            for (size_t j = 0; j < it->second[i].size(); ++j) {
                outFile << it->second[i][j];
                if (j < it->second[i].size() - 1) outFile << ", ";
            }
            outFile << "]";
            if (i < it->second.size() - 1) outFile << ", ";
        }
        outFile << "]";
        if (std::next(it) != dataMap.end()) outFile << ",";
        outFile << std::endl;
    }

    // end JSON object
    outFile << "}" << std::endl;

    outFile.close();
}

void createDirectory(const std::string& dirName) {
    std::filesystem::path dirPath("../data/" + dirName);

    if (!std::filesystem::exists(dirPath)) {
        if (!std::filesystem::create_directories(dirPath)) {
            throwError("Failed to create directory '../data/" + dirName + "'.");
        }
    }
}

double inverseFitness(const double fitness) {
    return 1.0 / (fitness + std::numeric_limits<double>::epsilon());
}

std::tm getLocalTime(const time_t& time_val) {
    std::tm buf;
#ifdef _WIN32
    // Windows-specific thread-safe localtime function
    localtime_s(&buf, &time_val);
#else
    // POSIX-specific thread-safe localtime function
    localtime_r(&time_val, &buf);
#endif
    return buf;
}

double gaussian_kernel(const double x, const double mu, const double sigma) {
    return exp(-0.5 * pow((x - mu) / sigma, 2)) / (sigma * sqrt(2.0 * PI));
}

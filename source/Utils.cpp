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
/* internal libraries */
#include "Utils.hpp"
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
        }
    }, cell);
}

float timeDifferenceInSeconds(std::tm& time1, std::tm& time2) {
    time_t t1 = std::mktime(&time1);
    time_t t2 = std::mktime(&time2);

    return static_cast<float>(std::difftime(t2, t1));
}

std::vector<unsigned> getAvailableAmbulanceIndicies(
    std::vector<Ambulance>& ambulances,
    const std::vector<Event>& events,
    const time_t& currentTime
) {
    std::vector<unsigned> availableAmbulanceIndicies;

    for (unsigned i = 0; i < ambulances.size(); i++) {
        int eventIndex = -1;

        if (ambulances[i].assignedEventId != -1) {
            eventIndex = findEventIndexFromId(events, ambulances[i].assignedEventId);
        }

        if (ambulances[i].isAvailable(events, eventIndex, currentTime)) {
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
    int indexInAdjustedWeights = std::distance(cumulativeWeights.begin(), it);

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

void writeMetrics(std::vector<Event>& events) {
    // get current time
    auto now = std::chrono::system_clock::now();
    std::time_t now_time = std::chrono::system_clock::to_time_t(now);

    // format current time as a string (YYYY-MM-DD_HH-MM-SS)
    std::tm bt = *std::localtime(&now_time);
    std::ostringstream oss;
    oss << std::put_time(&bt, "%Y-%m-%d_%H-%M-%S");
    std::string timestamp = oss.str();

    // construct filename with the current date and time
    std::string filename = "../data/metrics/metrics_" + timestamp + ".csv";
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
            << std::to_string(event.metrics["duration_incident_creation"]) << ","
            << std::to_string(event.metrics["duration_resource_appointment"]) << ","
            << std::to_string(event.metrics["duration_resource_preparing_departure"]) << ","
            << std::to_string(event.metrics["duration_dispatching_to_scene"]) << ","
            << std::to_string(event.metrics["duration_at_scene"]) << ","
            << std::to_string(event.metrics["duration_dispatching_to_hospital"]) << ","
            << std::to_string(event.metrics["duration_at_hospital"]) << ","
            << std::to_string(event.metrics["duration_dispatching_to_depot"]) << std::endl;
    }
}


void saveDistributionToFile(const std::vector<std::vector<double>>& distribution, const std::string& baseFilename) {
    auto now = std::chrono::system_clock::now();
    std::time_t now_time = std::chrono::system_clock::to_time_t(now);

    // format current time as string (YYYY-MM-DD_HH-MM-SS)
    std::tm bt = *std::localtime(&now_time);
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
    std::tm bt = *std::localtime(&now_time);
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
    std::tm bt = *std::localtime(&now_time);
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

double getRandomProbability(std::mt19937& rnd) {
    std::uniform_real_distribution<> dist(0.0, 1.0);

    return dist(rnd);
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
    int64_t xCorner = std::floor((utm.first + offset) / cellSize) * cellSize - offset;
    int64_t yCorner = std::floor(utm.second / cellSize) * cellSize;

    return 20000000000000 + (xCorner * 10000000) + yCorner;
}

int64_t approximateLocation(
    const int64_t& startId,
    const int64_t& goalId,
    const time_t& timeAtStart,
    const time_t& timeNow,
    const std::string& triage
) {
    int timeToReachGoal = ODMatrix::getInstance().getTravelTime(
        startId,
        goalId,
        true,
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
    std::tm* timeInfo = std::localtime(&eventTimer);
    int hour = timeInfo->tm_hour;

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

double averageResponseTime(std::vector<Event>& simulatedEvents, const std::string& triageImpression, const bool urban) {
    int totalEvents = 0;
    float totalResponseTime = 0;
    for (int i = 0; i < simulatedEvents.size(); i++) {
        Event event = simulatedEvents[i];

        if (event.utility) {
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

double responseTimeViolations(std::vector<Event>& simulatedEvents) {
    int totalViolations = 0;

    for (int i = 0; i < simulatedEvents.size(); i++) {
        int responseTime = simulatedEvents[i].getResponseTime();

        bool urban = Incidents::getInstance().gridIdUrban[simulatedEvents[i].incidentGridId];
        std::string triage = simulatedEvents[i].triageImpression;

        if (triage == "A") {
            if (urban && responseTime > 720) {
                totalViolations++;
            } else if (!urban && responseTime > 1500) {
                totalViolations++;
            }
        } else if (triage == "H") {
            if (urban && responseTime > 1800) {
                totalViolations++;
            } else if (!urban && responseTime > 2400) {
                totalViolations++;
            }
        }
    }

    return static_cast<double>(totalViolations);
}

void printTimeSegmentedAllocationTable(
    const bool dayShift,
    const int numTimeSegments,
    const std::vector<std::vector<int>>& allocations
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

    // TODO: Placeholder
    auto calculateFitnessForSegment = [&](int segmentIndex) -> double {
        return 33.0;
    };

    // Print the header
    std::cout << std::left << std::setw(20) << "Time Segment" << "|";
    for (int t = 0; t < numTimeSegments; ++t) {
        std::cout << std::setw(1) << "T" << t + 1;
        if (t + 1 < 10) {
            std::cout << " ";
        }
    }

    int headerStart = 24;
    int timeSegmentWidth = 5;
    int headerEnd = 12;
    int totalWidth = headerStart + (numTimeSegments * timeSegmentWidth) - 1 + headerEnd;


    std::cout << "|" << std::setw(2) << "Fitness\n";
    std::cout << std::string(totalWidth, '-') << "\n";

    // iterate over depots and print each row
    for (size_t d = 0; d < depotNames.size(); ++d) {
        std::cout << std::right << std::setw(19) << depotNames[d] << std::setw(1) << "|";
        for (size_t t = 0; t < numTimeSegments; ++t) {
            std::cout << std::right << std::setw(2) << allocations[t][d] << std::setw(2);
        }
        std::cout << "|" << std::setw(6) << calculateFitnessForSegment(d) << "\n";
    }
}

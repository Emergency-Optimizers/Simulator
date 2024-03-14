/**
 * @file Utils.cpp
 *
 * @copyright Copyright (c) 2024 Emergency-Optimizers
 */

/* external libraries */
#include <fstream>
#include <sstream>
#include <iomanip>
#include <optional>
#include <vector>
#include <algorithm>
#include <numeric>
#include <iterator>
#include <chrono>
#include <ctime>
/* internal libraries */
#include "Utils.hpp"
#include "simulator/CSVReader.hpp"

CellType Utils::toInt(const std::string& str) {
    return std::stoi(str);
}

CellType Utils::toInt64(const std::string& str) {
    return std::stoll(str);
}

CellType Utils::toFloat(const std::string& str) {
    return std::stof(str);
}

CellType Utils::toString(const std::string& str) {
    return str;
}

CellType Utils::toBool(const std::string& str) {
    return str == "True" || str == "true";
}

CellType Utils::toDateTime(const std::string& str) {
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

std::string Utils::tmToString(const std::tm& time) {
    std::stringstream ss;
    ss << std::put_time(&time, "%Y-%m-%d %H:%M:%S");
    return ss.str();
}

std::string Utils::cellTypeToString(const CellType& cell) {
    return std::visit([](auto&& arg) -> std::string {
        using T = std::decay_t<decltype(arg)>;
        if constexpr (std::is_same_v<T, int>) {
            return std::to_string(arg);
        } else if constexpr (std::is_same_v<T, int64_t>) {
            return std::to_string(arg);
        } else if constexpr (std::is_same_v<T, float>) {
            return std::to_string(arg);
        } else if constexpr (std::is_same_v<T, std::string>) {
            return arg;
        } else if constexpr (std::is_same_v<T, bool>) {
            return arg ? "true" : "false";
        } else if constexpr (std::is_same_v<T, std::optional<std::tm>>) {
            return arg ? tmToString(arg.value()) : "n/a";
        }
    }, cell);
}

std::tm Utils::stringToTm(const std::string& str) {
    std::tm time{};
    std::istringstream ss(str);
    ss >> std::get_time(&time, "%Y.%m.%dT%H:%M:%S");
    return time;
}

int Utils::compareTime(const std::tm& time_1, const std::tm& time_2) {
    if (time_1.tm_year > time_2.tm_year) return 1;
    if (time_1.tm_year < time_2.tm_year) return -1;
    if (time_1.tm_mon > time_2.tm_mon) return 1;
    if (time_1.tm_mon < time_2.tm_mon) return -1;
    if (time_1.tm_mday > time_2.tm_mday) return 1;
    if (time_1.tm_mday < time_2.tm_mday) return -1;
    if (time_1.tm_hour > time_2.tm_hour) return 1;
    if (time_1.tm_hour < time_2.tm_hour) return -1;
    if (time_1.tm_min > time_2.tm_min) return 1;
    if (time_1.tm_min < time_2.tm_min) return -1;
    if (time_1.tm_sec > time_2.tm_sec) return 1;
    if (time_1.tm_sec < time_2.tm_sec) return -1;
    return 0;
}

float Utils::timeDifferenceInSeconds(std::tm& time1, std::tm& time2) {
    time_t t1 = std::mktime(&time1);
    time_t t2 = std::mktime(&time2);

    return std::difftime(t2, t1);
}

int Utils::findClosestTimeIndex(const std::tm& target, const std::vector<std::tm>& times) {
    if (times.empty()) return -1;

    auto lower = std::lower_bound(times.begin(), times.end(), target, tm_less);

    if (lower == times.end()) return times.size() - 1;
    if (lower == times.begin()) return 0;

    auto prev = lower - 1;

    auto diff1 = std::mktime(const_cast<std::tm*>(&*lower)) - std::mktime(const_cast<std::tm*>(&target));
    auto diff2 = std::mktime(const_cast<std::tm*>(&target)) - std::mktime(const_cast<std::tm*>(&*prev));

    if (diff1 < diff2) {
        return std::distance(times.begin(), lower);
    } else {
        return std::distance(times.begin(), prev);
    }
}

bool Utils::tm_less(const std::tm& lhs, const std::tm& rhs) {
    if (lhs.tm_year != rhs.tm_year) return lhs.tm_year < rhs.tm_year;
    if (lhs.tm_mon != rhs.tm_mon) return lhs.tm_mon < rhs.tm_mon;
    if (lhs.tm_mday != rhs.tm_mday) return lhs.tm_mday < rhs.tm_mday;
    if (lhs.tm_hour != rhs.tm_hour) return lhs.tm_hour < rhs.tm_hour;
    if (lhs.tm_min != rhs.tm_min) return lhs.tm_min < rhs.tm_min;
    return lhs.tm_sec < rhs.tm_sec;
}

std::vector<unsigned> Utils::getAvailableAmbulanceIndicies(const std::vector<Ambulance>& ambulances) {
    std::vector<unsigned> availableAmbulanceIndicies;

    for (int i = 0; i < ambulances.size(); i++) {
        if (ambulances[i].assignedEventIndex == -1) availableAmbulanceIndicies.push_back(i);
    }

    return availableAmbulanceIndicies;
}

int Utils::calculateDayDifference(const std::tm& baseDate, const int targetMonth, const int targetDay) {
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

    if (targetDayOfYear == baseDayOfYear) return 0;

    while (true) {
        if (targetDayOfYear == baseDayOfYear) break;
        if (++baseDayOfYear > totalDaysInYear) baseDayOfYear = 0;
        dayDiff++;
    }

    targetDayOfYear = targetDate.tm_yday;
    baseDayOfYear = baseDateNormalized.tm_yday;
    int newDayDiff = 0;

    while (dayDiff != newDayDiff) {
        if (targetDayOfYear == baseDayOfYear) break;
        if (--baseDayOfYear < 0) baseDayOfYear = totalDaysInYear;
        newDayDiff++;
    }

    if (dayDiff > newDayDiff) dayDiff = newDayDiff;

    return dayDiff;
}

int Utils::weightedLottery(
    std::mt19937& rnd,
    const std::vector<double>& weights,
    const std::vector<std::pair<int, int>>& ranges
) {
    std::vector<double> adjustedWeights;

    if (ranges.empty()) {
        // If ranges is empty, use the entire weights vector
        adjustedWeights = weights;
    } else {
        // Otherwise, process each specified range
        for (const auto& range : ranges) {
            int beginIndex = std::max(0, range.first);
            int endIndex = std::min(static_cast<int>(weights.size()) - 1, range.second);

            if (beginIndex <= endIndex) {
                adjustedWeights.insert(adjustedWeights.end(), weights.begin() + beginIndex, weights.begin() + endIndex + 1);
            }
        }
    }

    // Create a partial sum of the adjusted weights
    std::vector<double> cumulativeWeights(adjustedWeights.size());
    std::partial_sum(adjustedWeights.begin(), adjustedWeights.end(), cumulativeWeights.begin());

    // Generate a random number in the adjusted range
    std::uniform_real_distribution<> dist(0.0, cumulativeWeights.back());
    auto it = std::lower_bound(cumulativeWeights.begin(), cumulativeWeights.end(), dist(rnd));
    int indexInAdjustedWeights = std::distance(cumulativeWeights.begin(), it);

    // For the default case (full range), the adjusted index is the final result
    if (ranges.empty()) {
        return indexInAdjustedWeights;
    }

    // Adjust the index to map back to the original weights vector for specified ranges
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

int Utils::getRandomInt(std::mt19937& rnd, const int min, const int max) {
    std::uniform_int_distribution<> dist(min, max);

    return dist(rnd);
}

void Utils::saveEventsToFile(const std::vector<Event>& events) {
    // get current time
    auto now = std::chrono::system_clock::now();
    std::time_t now_time = std::chrono::system_clock::to_time_t(now);

    // format current time as a string (YYYY-MM-DD_HH-MM-SS)
    std::tm bt = *std::localtime(&now_time);
    std::ostringstream oss;
    oss << std::put_time(&bt, "%Y-%m-%d_%H-%M-%S");
    std::string timestamp = oss.str();

    // construct filename with the current date and time
    std::string filename = "../data/events/events_" + timestamp + ".csv";
    std::ofstream outFile(filename);

    // check if the file stream is open before proceeding
    if (!outFile.is_open()) {
        std::cerr << "Failed to open file: " << filename << std::endl;
        return;
    }

    // write CSV header
    outFile << "time_call_received,triage_impression_during_call,grid_id,wait_time_incident_created,wait_time_ambulance_dispatch_to_hospital,wait_time_ambulance_available\n";

    // write each event to the CSV
    for (const auto& event : events) {
        std::ostringstream callReceivedOss;
        callReceivedOss << std::put_time(&event.callReceived, "%Y-%m-%d %H:%M:%S");
        std::string callReceivedStr = callReceivedOss.str();

        outFile << callReceivedStr << ","
                << event.triageImpression << ","
                << event.gridId << ","
                << event.secondsWaitCallAnswered << ","
                << event.secondsWaitDepartureScene << ","
                << event.secondsWaitAvailable << "\n";
    }
}

void Utils::saveMetricsToFile(const std::vector<Event>& events) {
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

    // write CSV header
    outFile << "time_call_received,triage_impression_during_call,grid_id,call_processed_time,dispatch_to_scene_time,arrival_at_scene_time,dispatch_to_hospital_time,arrival_at_hospital_time,dispatch_to_depot_time,waiting_for_ambulance_time\n";

    for (const auto& event : events) {
        std::string callReceivedStr = event.tmToString(event.callReceived);

        // write each metric to the CSV
        outFile << callReceivedStr << ","
                << event.triageImpression << ","
                << event.gridId << ","
                << event.metrics.callProcessedTime << ","
                << event.metrics.dispatchToSceneTime << ","
                << event.metrics.arrivalAtSceneTime << ","
                << event.metrics.dispatchToHospitalTime << ","
                << event.metrics.arrivalAtHospitalTime << ","
                << event.metrics.dispatchToDepotTime << ","
                << event.metrics.waitingForAmbulanceTime << "\n";
    }
}


void Utils::saveDistributionToFile(const std::vector<std::vector<double>>& distribution, const std::string& baseFilename) {
    auto now = std::chrono::system_clock::now();
    std::time_t now_time = std::chrono::system_clock::to_time_t(now);

    // Format current time as string (YYYY-MM-DD_HH-MM-SS)
    std::tm bt = *std::localtime(&now_time);
    std::ostringstream oss;
    oss << std::put_time(&bt, "%Y-%m-%d_%H-%M-%S");

    // Construct filename with the current date and time
    std::string filename =  "../data/distributions/" + baseFilename + "_" + oss.str() + ".csv";

    std::ofstream outFile(filename);
    if (!outFile.is_open()) {
        std::cerr << "Failed to open file: " << filename << std::endl;
        return;
    }

    // Write header
    outFile << "A,H,V1\n";

    // Write vector to file without trailing commas
    for (const auto& row : distribution) {
        for (size_t i = 0; i < row.size(); ++i) {
            outFile << row[i];
            if (i < row.size() - 1) outFile << ",";
        }
        outFile << "\n";
    }
}

void Utils::save1dDistributionToFile(const std::vector<double>& distribution, const std::string& baseFilename) {
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

void Utils::save2dDistributionToFile(const std::vector<std::vector<double>>& distribution, const std::string& baseFilename) {
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

double Utils::getRandomProbability(std::mt19937& rnd) {
    std::uniform_real_distribution<> dist(0.0, 1.0);
    return dist(rnd);
}

double Utils::calculateMean(const std::vector<int>& numbers) {
    double sum = 0.0;

    for (int num : numbers) {
        sum += num;
    }

    return numbers.empty() ? 0.0 : sum / numbers.size();
}

double Utils::calculateStandardDeviation(const std::vector<int>& numbers) {
    double mean = calculateMean(numbers);
    double varianceSum = 0.0;

    for (int num : numbers) {
        varianceSum += std::pow(num - mean, 2);
    }
    double variance = numbers.empty() ? 0.0 : varianceSum / numbers.size();

    return std::sqrt(variance);
}

double Utils::calculateEuclideanDistance(double x1, double y1, double x2, double y2) {
    return std::sqrt(std::pow(x2 - x1, 2) + std::pow(y2 - y1, 2));
}

std::pair<int, int> Utils::idToUtm(int64_t grid_id) {
    int x = std::floor(grid_id * std::pow(10, -7)) - (2 * static_cast<int>(std::pow(10, 6)));
    int y = grid_id - (std::floor(grid_id * std::pow(10, -7)) * static_cast<int64_t>(std::pow(10, 7)));
    return std::make_pair(x, y);
}

int64_t Utils::utmToId(const std::pair<int, int>& utm, int cellSize, int offset) {
   int64_t xCorner = std::floor((utm.first + offset) / cellSize) * cellSize - offset;
   int64_t yCorner = std::floor(utm.second / cellSize) * cellSize;

   return 20000000000000 + (xCorner * 10000000) + yCorner;
}

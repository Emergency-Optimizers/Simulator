/**
 * @file MonteCarloSimulator.cpp
 *
 * @copyright Copyright (c) 2024 Emergency-Optimizers
 */

/* external libraries */
#include <algorithm>
/* internal libraries */
#include "simulator/MonteCarloSimulator.hpp"
#include "Constants.hpp"

MonteCarloSimulator::MonteCarloSimulator(
    std::mt19937& rnd,
    Incidents& incidents,
    const int year,
    const int month,
    const int day,
    const bool dayShift,
    const unsigned windowSize
) : rnd(rnd), incidents(incidents), windowSize(windowSize), year(year), month(month), day(day), dayShift(dayShift) {
    filteredIncidents = incidents.rowsWithinTimeFrame(month, day, windowSize);

    weights = generateWeights(windowSize);
    for (int i = 0, indexGridId = 0; i < incidents.size(); i++) {
        int64_t gridId = incidents.get<int64_t>("grid_id", i);

        if (gridIdToIndexMapping.count(gridId)) continue;

        indexToGridIdMapping[indexGridId] = gridId;

        gridIdToIndexMapping[gridId] = indexGridId;

        indexGridId++;
    }

    generateHourlyIncidentProbabilityDistribution();
    generateMinuteIncidentProbabilityDistribution();
    generateTriageProbabilityDistribution();
    generateCanceledProbabilityDistribution();
    generateLocationProbabilityDistribution();
    generateWaitTimeHistograms();
}

std::vector<double> MonteCarloSimulator::generateWeights(int windowSize, double sigma) {
    std::vector<double> weights;
    double centralWeight = std::exp(0);

    for (int i = 0; i <= windowSize; ++i) {
        double weight = std::exp(-i * i / (2 * sigma * sigma)) / centralWeight;
        weights.push_back(weight);
    }

    // normalize weights so the central day has a weight of 1
    double normalizationFactor = weights[0];
    for (double& weight : weights) {
        weight /= normalizationFactor;
    }

    return weights;
}

void MonteCarloSimulator::generateHourlyIncidentProbabilityDistribution() {
    std::vector<double> newHourlyIncidentProbabilityDistribution(24, 0);

    std::vector<double> totalIncidentsPerHour(24, 0);
    double totalIncidents = 0;

    // get total incidents per hour for each row in the filtered dataset
    for (int i = 0; i < filteredIncidents.size(); i++) {
        std::tm timeCallReceived = filteredIncidents.get<std::optional<std::tm>>("time_call_received", i).value();

        // calculate weight based on how far away the incident is from the target
        int dayDiff = Utils::calculateDayDifference(timeCallReceived, month, day);
        double weight = weights[dayDiff];

        totalIncidentsPerHour[timeCallReceived.tm_hour] += weight;
        totalIncidents += weight;
    }

    // get the probability per hour
    for (int i = 0; i < totalIncidentsPerHour.size(); i++) {
        newHourlyIncidentProbabilityDistribution[i] = totalIncidentsPerHour[i] / totalIncidents;
    }

    hourlyIncidentProbabilityDistribution = newHourlyIncidentProbabilityDistribution;
}

void MonteCarloSimulator::generateMinuteIncidentProbabilityDistribution() {
    std::vector<std::vector<double>> newMinuteIncidentProbabilityDistribution(24, std::vector<double>(60, 0));

    std::vector<std::vector<double>> totalIncidentsPerMinute(24, std::vector<double>(60, 0));
    std::vector<double> totalIncidents(24, 0);

    for (int i = 0; i < filteredIncidents.size(); i++) {
        std::tm timeCallReceived = filteredIncidents.get<std::optional<std::tm>>("time_call_received", i).value();

        int dayDiff = Utils::calculateDayDifference(timeCallReceived, month, day);
        double weight = weights[dayDiff];

        totalIncidentsPerMinute[timeCallReceived.tm_hour][timeCallReceived.tm_min] += weight;
        totalIncidents[timeCallReceived.tm_hour] += weight;
    }

    for (int indexHour = 0; indexHour < 24; indexHour++) {
        for (int indexMinute = 0; indexMinute < 60; indexMinute++) {
            double minuteIncidentProbability = totalIncidentsPerMinute[indexHour][indexMinute] / totalIncidents[indexHour];
            newMinuteIncidentProbabilityDistribution[indexHour][indexMinute] = minuteIncidentProbability;
        }
    }

    minuteIncidentProbabilityDistribution = newMinuteIncidentProbabilityDistribution;
}

void MonteCarloSimulator::generateTriageProbabilityDistribution() {
    std::vector<std::vector<double>> newTriageProbabilityDistribution(24, std::vector<double>(3, 0));

    std::vector<std::vector<double>> totalIncidentsPerTriage(24, std::vector<double>(3, 0));
    std::vector<double> totalIncidents(24, 0);

    for (int i = 0; i < filteredIncidents.size(); i++) {
        std::tm timeCallReceived = filteredIncidents.get<std::optional<std::tm>>("time_call_received", i).value();

        int dayDiff = Utils::calculateDayDifference(timeCallReceived, month, day);
        double weight = weights[dayDiff];

        std::string triageImpression = filteredIncidents.get<std::string>("triage_impression_during_call", i);

        int indexTriage;
        if (triageImpression == "A") {
            indexTriage = 0;
        } else if (triageImpression == "H") {
            indexTriage = 1;
        } else if (triageImpression == "V1") {
            indexTriage = 2;
        }

        totalIncidentsPerTriage[timeCallReceived.tm_hour][indexTriage] += weight;
        totalIncidents[timeCallReceived.tm_hour] += weight;
    }

    for (int indexHour = 0; indexHour < 24; indexHour++) {
        for (int indexTriage = 0; indexTriage < 3; indexTriage++) {
            double triageIncidentProbability = totalIncidentsPerTriage[indexHour][indexTriage] / totalIncidents[indexHour];
            newTriageProbabilityDistribution[indexHour][indexTriage] = triageIncidentProbability;
        }
    }

    triageProbabilityDistribution = newTriageProbabilityDistribution;
}

void MonteCarloSimulator::generateCanceledProbabilityDistribution() {
    std::vector<std::vector<double>> newCanceledProbability(3, std::vector<double>(2, 0));

    std::vector<std::vector<double>> totalIncidentsPer(3, std::vector<double>(2, 0));
    std::vector<std::vector<double>> totalIncidents(3, std::vector<double>(2, 0));

    std::vector<double> weightsYear = generateWeights(365);

    for (int i = 0; i < incidents.size(); i++) {
        std::tm timeCallReceived = incidents.get<std::optional<std::tm>>("time_call_received", i).value();
        int dayDiff = Utils::calculateDayDifference(timeCallReceived, month, day);
        double weight = weightsYear[dayDiff];

        std::string triageImpression = incidents.get<std::string>("triage_impression_during_call", i);

        bool canceled = !incidents.get<std::optional<std::tm>>("time_departure_scene", i).has_value();

        int indexTriage;
        if (triageImpression == "A") {
            indexTriage = 0;
        } else if (triageImpression == "H") {
            indexTriage = 1;
        } else if (triageImpression == "V1") {
            indexTriage = 2;
        }

        int indexShift = timeCallReceived.tm_hour >= DAY_SHIFT_START && timeCallReceived.tm_hour <= DAY_SHIFT_END ? 0 : 1;

        if (canceled) totalIncidentsPer[indexTriage][indexShift] += weight;
        totalIncidents[indexTriage][indexShift] += weight;
    }

    for (int indexTriage = 0; indexTriage < 3; indexTriage++) {
        for (int indexShift = 0; indexShift < 2; indexShift++) {
            double totalIncidentsCanceled = totalIncidentsPer[indexTriage][indexShift];
            if (totalIncidentsCanceled != 0) {
                double canceledIncidentProbability = totalIncidentsCanceled / totalIncidents[indexTriage][indexShift];
                newCanceledProbability[indexTriage][indexShift] = canceledIncidentProbability;
            }
        }
    }

    canceledProbability = newCanceledProbability;
}

void MonteCarloSimulator::generateLocationProbabilityDistribution() {
    int gridIdSize = indexToGridIdMapping.size();

    std::vector<std::vector<std::vector<double>>> newLocationProbabilityDistribution(
        3,
        std::vector<std::vector<double>>(2, std::vector<double>(gridIdSize, 0))
    );

    std::vector<std::vector<std::vector<double>>> totalIncidentsPerLocation(
        3,
        std::vector<std::vector<double>>(2, std::vector<double>(gridIdSize, 0))
    );
    std::vector<std::vector<double>> totalIncidents(3, std::vector<double>(2, 0));

    std::vector<double> weightsYear = generateWeights(365);

    for (int i = 0; i < incidents.size(); i++) {
        std::tm timeCallReceived = incidents.get<std::optional<std::tm>>("time_call_received", i).value();
        int dayDiff = Utils::calculateDayDifference(timeCallReceived, month, day);
        double weight = weightsYear[dayDiff];

        std::string triageImpression = incidents.get<std::string>("triage_impression_during_call", i);

        int64_t gridId = incidents.get<int64_t>("grid_id", i);

        int indexTriage;
        if (triageImpression == "A") {
            indexTriage = 0;
        } else if (triageImpression == "H") {
            indexTriage = 1;
        } else if (triageImpression == "V1") {
            indexTriage = 2;
        }

        int indexShift = timeCallReceived.tm_hour >= DAY_SHIFT_START && timeCallReceived.tm_hour <= DAY_SHIFT_END ? 0 : 1;

        totalIncidentsPerLocation[indexTriage][indexShift][gridIdToIndexMapping[gridId]] += weight;
        totalIncidents[indexTriage][indexShift] += weight;
    }

    for (int indexTriage = 0; indexTriage < 3; indexTriage++) {
        for (int indexShift = 0; indexShift < 2; indexShift++) {
            for (int indexGridId = 0; indexGridId < gridIdSize; indexGridId++) {
                double totalIncidentsLocation = totalIncidentsPerLocation[indexTriage][indexShift][indexGridId];
                double locationIncidentProbability = totalIncidentsLocation / totalIncidents[indexTriage][indexShift];
                newLocationProbabilityDistribution[indexTriage][indexShift][indexGridId] = locationIncidentProbability;
            }
        }
    }

    locationProbabilityDistribution = newLocationProbabilityDistribution;
}

void MonteCarloSimulator::generateWaitTimeHistograms() {
    generateWaitTimeHistogram("time_call_received", "time_call_answered", 10);
    generateWaitTimeHistogram("time_call_answered", "time_ambulance_notified", 10);
    generateWaitTimeHistogram("time_ambulance_notified", "time_dispatch", 10);
    generateWaitTimeHistogram("time_arrival_scene", "time_departure_scene", 10);
    generateWaitTimeHistogram("time_arrival_hospital", "time_available", 10);

    // custom histogram for cancelled incidents
    std::vector<std::string> triageImpressions = { "A", "H", "V1" };

    for (std::string triageImpression : triageImpressions) {
        std::vector<float> data;

        for (int i = 0; i < filteredIncidents.size(); i++) {
            // filter by triage impression
            if (filteredIncidents.get<std::string>("triage_impression_during_call", i) != triageImpression) continue;
            // filter out rows not cancelled
            if (!filteredIncidents.get<std::optional<std::tm>>("time_departure_scene", i).has_value()) continue;

            float timeDiff = filteredIncidents.timeDifferenceBetweenHeaders("time_arrival_scene", "time_available", i);
            data.push_back(timeDiff);
        }
        waitTimesHistograms[std::pair("time_arrival_scene", "time_available")][triageImpression] = createHistogram(data, 10);
    }
}

void MonteCarloSimulator::generateWaitTimeHistogram(
    const std::string fromEventColumn,
    const std::string toEventColumn,
    const int binSize
) {
    std::vector<std::string> triageImpressions = { "A", "H", "V1" };

    for (std::string triageImpression : triageImpressions) {
        std::vector<float> data;

        for (int i = 0; i < filteredIncidents.size(); i++) {
            // filter by triage impression
            if (filteredIncidents.get<std::string>("triage_impression_during_call", i) != triageImpression) continue;
            // filter out rows with NaN values
            if (!filteredIncidents.get<std::optional<std::tm>>(fromEventColumn, i).has_value()) continue;
            if (!filteredIncidents.get<std::optional<std::tm>>(toEventColumn, i).has_value()) continue;

            float timeDiff = filteredIncidents.timeDifferenceBetweenHeaders(fromEventColumn, toEventColumn, i);
            data.push_back(timeDiff);
        }
        waitTimesHistograms[std::pair(fromEventColumn, toEventColumn)][triageImpression] = createHistogram(data, binSize);
    }
}

std::map<std::pair<float, float>, double> MonteCarloSimulator::createHistogram(const std::vector<float>& data, int numBins) {
    auto [minElem, maxElem] = std::minmax_element(data.begin(), data.end());
    float minVal = *minElem;
    float maxVal = *maxElem;
    float range = maxVal - minVal;
    float binSize = range / numBins;

    std::map<std::pair<float, float>, double> histogram;

    for (float value : data) {
        int binIndex;
        if (value == maxVal) {
            binIndex = numBins - 1;
        } else {
            binIndex = (value - minVal) / binSize;
        }

        float binStart = minVal + binIndex * binSize;
        float binEnd = binStart + binSize;

        if (binIndex == numBins - 1) {
            binEnd = maxVal;
        }
        std::pair<float, float> binRange(binStart, binEnd);

        histogram[binRange]++;
    }

    double cumulativeProbability = 0.0;
    int totalIncidents = data.size();

    for (const auto& bin : histogram) {
        double probability = bin.second / totalIncidents;
        cumulativeProbability += probability;
        histogram[bin.first] = cumulativeProbability;
        // std::cout << "(" << "<" << bin.first.first << ", " << bin.first.second << ">" << ": " << bin.second << "), ";
    }

    return histogram;
}

float MonteCarloSimulator::generateRandomWaitTimeFromHistogram(const std::map<std::pair<float, float>, double>& histogram) {
    float start = 0;
    float end = 0;

    std::uniform_real_distribution<> dis(0, 1);
    double randomValue = dis(rnd);

    for (const auto& bin : histogram) {
        if (randomValue <= bin.second) {
            start = bin.first.first;
            end = bin.first.second;
            break;
        }
    }

    std::uniform_real_distribution<> valueDis(start, end);
    return valueDis(rnd);
}

int MonteCarloSimulator::getTotalIncidentsToGenerate() {
    std::tm date = {0};
    date.tm_year = 119;
    date.tm_mon = month - 1;
    date.tm_mday = day;
    mktime(&date);

    int totalMorning = -1;
    int totalDay = -1;
    int totalNight = -1;

    for (int i = 0; i < incidents.size(); i++) {
        std::tm timeCallReceived = incidents.get<std::optional<std::tm>>("time_call_received", i).value();
        mktime(&timeCallReceived);

        if (timeCallReceived.tm_yday == date.tm_yday - 1 && totalNight == -1) totalNight = incidents.get<int>("total_night", i);
        if (timeCallReceived.tm_yday == date.tm_yday) {
            totalMorning = incidents.get<int>("total_morning", i);
            totalDay = incidents.get<int>("total_day", i);
            break;
        }
    }

    return dayShift ? totalDay : totalMorning + totalNight;
}

std::vector<Event> MonteCarloSimulator::generateEvents() {
    std::vector<Event> events;

    int totalEvents = getTotalIncidentsToGenerate();
    std::vector<std::string> triageImpressions = { "A", "H", "V1" };
    int indexShift = dayShift ? 0 : 1;
    std::vector<std::pair<int, int>> indexRangesHour = dayShift ?
        std::vector<std::pair<int, int>>{{7, 21}} :
        std::vector<std::pair<int, int>>{{0, 6}, {22, 23}};

    for (int i = 0; i < totalEvents; i++) {
        Event event;

        // get call received
        int callReceivedHour = Utils::weightedLottery(rnd, hourlyIncidentProbabilityDistribution, indexRangesHour);
        int callReceivedMin = Utils::weightedLottery(rnd, minuteIncidentProbabilityDistribution[callReceivedHour]);
        int callReceivedSec = Utils::getRandomInt(rnd, 0, 59);

        event.callReceived = {0};
        event.callReceived.tm_year = year - 1900;
        event.callReceived.tm_mon = month - 1;
        event.callReceived.tm_mday = day;
        event.callReceived.tm_hour = callReceivedHour;
        event.callReceived.tm_min = callReceivedMin;
        event.callReceived.tm_sec = callReceivedSec;
        mktime(&event.callReceived);

        // get triage impression
        int indexTriage = Utils::weightedLottery(rnd, triageProbabilityDistribution[callReceivedHour]);
        event.triageImpression = triageImpressions[indexTriage];

        // check if it should be canceled
        bool canceled = canceledProbability[indexTriage][indexShift] > Utils::getRandomProbability(rnd);

        // location
        event.gridId = indexToGridIdMapping[Utils::weightedLottery(rnd, locationProbabilityDistribution[indexTriage][indexShift])];

        // wait times
        event.secondsWaitCallAnswered = generateRandomWaitTimeFromHistogram(
            waitTimesHistograms[std::pair("time_call_received", "time_call_answered")][event.triageImpression]
        );
        event.secondsWaitAmbulanceNotified = generateRandomWaitTimeFromHistogram(
            waitTimesHistograms[std::pair("time_call_answered", "time_ambulance_notified")][event.triageImpression]
        );
        event.secondsWaitAmbulanceDispatch = generateRandomWaitTimeFromHistogram(
            waitTimesHistograms[std::pair("time_ambulance_notified", "time_dispatch")][event.triageImpression]
        );

        if (!canceled) {
            event.secondsWaitDepartureScene = generateRandomWaitTimeFromHistogram(
                waitTimesHistograms[std::pair("time_arrival_scene", "time_departure_scene")][event.triageImpression]
            );
            event.secondsWaitAvailable = generateRandomWaitTimeFromHistogram(
                waitTimesHistograms[std::pair("time_arrival_hospital", "time_available")][event.triageImpression]
            );
        }

        // setup timer
        event.timer = std::mktime(&event.callReceived);

        events.push_back(event);
    }

    std::sort(events.begin(), events.end(), [](const Event& a, const Event& b) {
        return a.timer < b.timer;
    });

    return events;
}

/**
 * @file MonteCarloSimulator.cpp
 *
 * @copyright Copyright (c) 2024 Emergency-Optimizers
 */

/* external libraries */
#include <algorithm>
/* internal libraries */
#include "simulator/MonteCarloSimulator.hpp"

MonteCarloSimulator::MonteCarloSimulator(
    std::mt19937& rnd,
    Incidents& incidents,
    const int month,
    const int day,
    const unsigned windowSize
) : rnd(rnd), incidents(incidents), windowSize(windowSize), month(month), day(day) {
    filteredIncidents = incidents.rowsWithinTimeFrame(month, day, windowSize);

    weights = generateWeights(windowSize);

    generateHourlyIncidentProbabilityDistribution();
    generateMinuteIncidentProbabilityDistribution();
    generateTriageProbabilityDistribution();
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
    std::vector<float> newHourlyIncidentProbabilityDistribution(24, 0);

    std::vector<float> totalIncidentsPerHour(24, 0);
    int totalIncidents = 0;

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
    std::vector<std::vector<float>> newMinuteIncidentProbabilityDistribution(24, std::vector<float>(60, 0.0));

    std::vector<std::vector<float>> totalIncidentsPerMinute(24, std::vector<float>(60, 0.0));
    std::vector<float> totalIncidents(24, 0.0);

    for (int i = 0; i < filteredIncidents.size(); i++) {
        std::tm timeCallReceived = filteredIncidents.get<std::optional<std::tm>>("time_call_received", i).value();

        int dayDiff = Utils::calculateDayDifference(timeCallReceived, month, day);
        double weight = weights[dayDiff];

        totalIncidentsPerMinute[timeCallReceived.tm_hour][timeCallReceived.tm_min] += weight;
        totalIncidents[timeCallReceived.tm_hour] += weight;
    }

    for (int indexHour = 0; indexHour < 24; indexHour++) {
        for (int indexMinute = 0; indexMinute < 60; indexMinute++) {
            float minuteIncidentProbability = totalIncidentsPerMinute[indexHour][indexMinute] / totalIncidents[indexHour];
            newMinuteIncidentProbabilityDistribution[indexHour][indexMinute] = minuteIncidentProbability;
        }
    }

    minuteIncidentProbabilityDistribution = newMinuteIncidentProbabilityDistribution;
}

void MonteCarloSimulator::generateTriageProbabilityDistribution() {
    std::vector<std::vector<float>> newTriageProbabilityDistribution(24, std::vector<float>(3, 0.0));

    std::vector<std::vector<float>> totalIncidentsPerTriage(24, std::vector<float>(3, 0.0));
    std::vector<float> totalIncidents(24, 0.0);

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
        std::cout << "(";
        for (int indexTriage = 0; indexTriage < 3; indexTriage++) {
            float triageIncidentProbability = totalIncidentsPerTriage[indexHour][indexTriage] / totalIncidents[indexHour];
            newTriageProbabilityDistribution[indexHour][indexTriage] = triageIncidentProbability;
            std::cout << triageIncidentProbability << " ";
        }
        std::cout << "), ";
    }

    triageProbabilityDistribution = newTriageProbabilityDistribution;
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

std::map<std::pair<float, float>, float> MonteCarloSimulator::createHistogram(const std::vector<float>& data, int numBins) {
    auto [minElem, maxElem] = std::minmax_element(data.begin(), data.end());
    float minVal = *minElem;
    float maxVal = *maxElem;
    float range = maxVal - minVal;
    float binSize = range / numBins;

    std::map<std::pair<float, float>, float> histogram;

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

    float cumulativeProbability = 0.0;
    int totalIncidents = data.size();

    for (const auto& bin : histogram) {
        float probability = bin.second / totalIncidents;
        cumulativeProbability += probability;
        histogram[bin.first] = cumulativeProbability;
        // std::cout << "(" << "<" << bin.first.first << ", " << bin.first.second << ">" << ": " << bin.second << "), ";
    }

    return histogram;
}

float MonteCarloSimulator::generateRandomWaitTimeFromHistogram(const std::map<std::pair<float, float>, float>& histogram) {
    float start = 0;
    float end = 0;

    std::uniform_real_distribution<> dis(0, 1);
    float randomValue = dis(rnd);

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

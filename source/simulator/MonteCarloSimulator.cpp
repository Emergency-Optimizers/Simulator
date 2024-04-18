/**
 * @file MonteCarloSimulator.cpp
 *
 * @copyright Copyright (c) 2024 Emergency-Optimizers
 */

/* external libraries */
#include <algorithm>
#include <iomanip>
#include <numeric>
/* internal libraries */
#include "simulator/MonteCarloSimulator.hpp"
#include "Constants.hpp"
#include "ProgressBar.hpp"
#include "Utils.hpp"
#include "file-reader/Settings.hpp"
#include "file-reader/Incidents.hpp"

MonteCarloSimulator::MonteCarloSimulator(
    std::mt19937& rnd,
    const int year,
    const int month,
    const int day,
    const bool dayShift,
    const unsigned windowSize
) : rnd(rnd), windowSize(windowSize), year(year), month(month), day(day), dayShift(dayShift) {
    ProgressBar progressBar(12, "Generating MCS");
    size_t progress = 0;

    filteredIncidents = Incidents::getInstance().rowsWithinTimeFrame(month, day, windowSize);
    progressBar.update(++progress);

    weights = generateWeights(windowSize);
    for (int i = 0, indexGridId = 0; i < Incidents::getInstance().size(); i++) {
        int64_t gridId = Incidents::getInstance().get<int64_t>("grid_id", i);

        if (gridIdToIndexMapping.count(gridId)) continue;

        indexToGridIdMapping[indexGridId] = gridId;

        gridIdToIndexMapping[gridId] = indexGridId;

        indexGridId++;
    }
    progressBar.update(++progress);

    generateHourlyIncidentProbabilityDistribution();
    progressBar.update(++progress);

    generateTriageProbabilityDistribution();
    progressBar.update(++progress);

    generateCanceledProbabilityDistribution();
    progressBar.update(++progress);

    generateLocationProbabilityDistribution();
    progressBar.update(++progress);

    generateDurationsData("time_call_received", "time_incident_created");
    progressBar.update(++progress);

    generateDurationsData("time_incident_created", "time_resource_appointed");
    progressBar.update(++progress);

    generateDurationsData("time_resource_appointed", "time_ambulance_dispatch_to_scene");
    progressBar.update(++progress);

    generateDurationsData("time_ambulance_arrived_at_scene", "time_ambulance_dispatch_to_hospital");
    progressBar.update(++progress);

    generateDurationsData("time_ambulance_arrived_at_hospital", "time_ambulance_available");
    progressBar.update(++progress);

    const bool filterToCancelledEvents = true;
    generateDurationsData("time_ambulance_arrived_at_scene", "time_ambulance_available", filterToCancelledEvents);
    progressBar.update(++progress);
}

std::vector<double> MonteCarloSimulator::generateWeights(int weigthSize, double sigma) {
    std::vector<double> newWeights;
    double centralWeight = std::exp(0);

    for (int i = 0; i <= weigthSize; ++i) {
        double weight = std::exp(-i * i / (2 * sigma * sigma)) / centralWeight;
        newWeights.push_back(weight);
    }

    // normalize weights so the central day has a weight of 1
    double normalizationFactor = newWeights[0];
    for (double& weight : newWeights) {
        weight /= normalizationFactor;
    }

    return newWeights;
}

void MonteCarloSimulator::generateHourlyIncidentProbabilityDistribution() {
    std::vector<double> newHourlyIncidentProbabilityDistribution(24, 0);

    std::vector<double> totalIncidentsPerHour(24, 0);
    double totalIncidents = 0;

    // get total incidents per hour for each row in the filtered dataset
    for (int i = 0; i < filteredIncidents.size(); i++) {
        std::tm timeCallReceived = Incidents::getInstance().get<std::optional<std::tm>>(
            "time_call_received",
            filteredIncidents[i]
        ).value();

        // calculate weight based on how far away the incident is from the target
        int dayDiff = calculateDayDifference(timeCallReceived, month, day);
        double weight = weights[dayDiff];

        totalIncidentsPerHour[timeCallReceived.tm_hour] += weight;
        totalIncidents += weight;
    }

    // get the probability per hour
    for (int i = 0; i < totalIncidentsPerHour.size(); i++) {
        newHourlyIncidentProbabilityDistribution[i] = totalIncidentsPerHour[i] / totalIncidents;
    }

    hourlyIncidentProbabilityDistribution = newHourlyIncidentProbabilityDistribution;
    // save1dDistributionToFile(hourlyIncidentProbabilityDistribution, "hourly_incident_probability_distribution");
}

void MonteCarloSimulator::generateTriageProbabilityDistribution() {
    std::vector<std::vector<double>> newTriageProbabilityDistribution(24, std::vector<double>(3, 0));

    std::vector<std::vector<double>> totalIncidentsPerTriage(24, std::vector<double>(3, 0));
    std::vector<double> totalIncidents(24, 0);
    int dayOfYearFinished = -1;

    std::vector<std::string> triageImpressions = {"A", "H", "V1"};

    for (int i = 0; i < filteredIncidents.size(); i++) {
        std::tm timeCallReceived = Incidents::getInstance().get<std::optional<std::tm>>(
            "time_call_received",
            filteredIncidents[i]
        ).value();

        if (timeCallReceived.tm_yday == dayOfYearFinished) {
            continue;
        } else {
            dayOfYearFinished = timeCallReceived.tm_yday;
        }

        int dayDiff = calculateDayDifference(timeCallReceived, month, day);
        double weight = weights[dayDiff];

        for (int indexHour = 0; indexHour < 24; indexHour++) {
            for (int indexTriage = 0; indexTriage < 3; indexTriage++) {
                int numIncidents = Incidents::getInstance().get<int>(
                    "total_" + triageImpressions[indexTriage] + "_incidents_hour_" + std::to_string(indexHour),
                    filteredIncidents[i]
                );

                double weightedNum = static_cast<double>(numIncidents) * weight;

                totalIncidentsPerTriage[indexHour][indexTriage] += weightedNum;
                totalIncidents[indexHour] += weightedNum;
            }
        }
    }

    for (int indexHour = 0; indexHour < 24; indexHour++) {
        for (int indexTriage = 0; indexTriage < 3; indexTriage++) {
            double triageIncidentProbability = totalIncidentsPerTriage[indexHour][indexTriage] / totalIncidents[indexHour];
            newTriageProbabilityDistribution[indexHour][indexTriage] = triageIncidentProbability;
        }
    }

    triageProbabilityDistribution = newTriageProbabilityDistribution;
    // saveDistributionToFile(triageProbabilityDistribution, "triage_probability_distribution");
}

void MonteCarloSimulator::generateCanceledProbabilityDistribution() {
    std::vector<std::vector<double>> newCanceledProbability(3, std::vector<double>(2, 0));

    std::vector<std::vector<double>> totalIncidentsPer(3, std::vector<double>(2, 0));
    std::vector<std::vector<double>> totalIncidents(3, std::vector<double>(2, 0));

    std::vector<double> weightsYear = generateWeights(365);

    for (int i = 0; i < Incidents::getInstance().size(); i++) {
        std::tm timeCallReceived = Incidents::getInstance().get<std::optional<std::tm>>("time_call_received", i).value();
        int dayDiff = calculateDayDifference(timeCallReceived, month, day);
        double weight = weightsYear[dayDiff];

        std::string triageImpression = Incidents::getInstance().get<std::string>("triage_impression_during_call", i);

        bool canceled = !Incidents::getInstance().get<std::optional<std::tm>>("time_ambulance_dispatch_to_hospital", i).has_value();

        int indexTriage = -1;
        if (triageImpression == "A") {
            indexTriage = 0;
        } else if (triageImpression == "H") {
            indexTriage = 1;
        } else if (triageImpression == "V1") {
            indexTriage = 2;
        }

        const bool eventAfterDayShiftStart = timeCallReceived.tm_hour >= Settings::get<int>("DAY_SHIFT_START");
        const bool eventBeforeDayShiftEnd = timeCallReceived.tm_hour <= Settings::get<int>("DAY_SHIFT_END");
        int indexShift = eventAfterDayShiftStart && eventBeforeDayShiftEnd ? 0 : 1;

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
    int gridIdSize = static_cast<int>(indexToGridIdMapping.size());

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

    for (int i = 0; i < Incidents::getInstance().size(); i++) {
        std::tm timeCallReceived = Incidents::getInstance().get<std::optional<std::tm>>("time_call_received", i).value();
        int dayDiff = calculateDayDifference(timeCallReceived, month, day);
        double weight = weightsYear[dayDiff];

        std::string triageImpression = Incidents::getInstance().get<std::string>("triage_impression_during_call", i);

        int64_t gridId = Incidents::getInstance().get<int64_t>("grid_id", i);

        int indexTriage = -1;
        if (triageImpression == "A") {
            indexTriage = 0;
        } else if (triageImpression == "H") {
            indexTriage = 1;
        } else if (triageImpression == "V1") {
            indexTriage = 2;
        }

        const bool eventAfterDayShiftStart = timeCallReceived.tm_hour >= Settings::get<int>("DAY_SHIFT_START");
        const bool eventBeforeDayShiftEnd = timeCallReceived.tm_hour <= Settings::get<int>("DAY_SHIFT_END");
        int indexShift = eventAfterDayShiftStart && eventBeforeDayShiftEnd ? 0 : 1;

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

void MonteCarloSimulator::generateDurationsData(
    const std::string& fromEventColumn,
    const std::string& toEventColumn,
    const bool filterToCancelledEvents
) {
    preProcessedKDEData[std::pair(fromEventColumn, toEventColumn)] = std::vector<std::vector<KDEData>>(TRIAGES.size(), std::vector<KDEData>(2));

    for (size_t indexTriage = 0; indexTriage < TRIAGES.size(); indexTriage++) {
        for (size_t indexShift = 0; indexShift < 2; indexShift++) {
            KDEData kdeData;

            for (size_t filteredIncidentsIndex = 0; filteredIncidentsIndex < filteredIncidents.size(); filteredIncidentsIndex++) {
                const bool differentTriage = Incidents::getInstance().get<std::string>(
                    "triage_impression_during_call",
                    filteredIncidents[filteredIncidentsIndex]
                ) != TRIAGES[indexTriage];

                if (differentTriage) {
                    continue;
                }

                std::tm timeCallReceived = Incidents::getInstance().get<std::optional<std::tm>>(
                    "time_call_received",
                    filteredIncidents[filteredIncidentsIndex]
                ).value();

                const bool eventAfterDayShiftStart = timeCallReceived.tm_hour >= Settings::get<int>("DAY_SHIFT_START");
                const bool eventBeforeDayShiftEnd = timeCallReceived.tm_hour <= Settings::get<int>("DAY_SHIFT_END");
                int eventIndexShift = eventAfterDayShiftStart && eventBeforeDayShiftEnd ? 0 : 1;

                if (eventIndexShift != indexShift) {
                    continue;
                }

                const bool noValueInFromEventColumn = !Incidents::getInstance().get<std::optional<std::tm>>(
                    fromEventColumn,
                    filteredIncidents[filteredIncidentsIndex]
                ).has_value();

                if (noValueInFromEventColumn) {
                    continue;
                }

                const bool noValueInToEventColumn = !Incidents::getInstance().get<std::optional<std::tm>>(
                    toEventColumn,
                    filteredIncidents[filteredIncidentsIndex]
                ).has_value();

                if (noValueInToEventColumn) {
                    continue;
                }

                const bool cancelledEvent = !Incidents::getInstance().get<std::optional<std::tm>>(
                    "time_ambulance_dispatch_to_hospital",
                    filteredIncidents[filteredIncidentsIndex]
                ).has_value();

                if (filterToCancelledEvents && !cancelledEvent) {
                    continue;
                }

                double duration = Incidents::getInstance().timeDifferenceBetweenHeaders(
                    fromEventColumn,
                    toEventColumn,
                    filteredIncidents[filteredIncidentsIndex]
                );

                kdeData.data.push_back(duration);
            }

            precomputeKDE(kdeData);

            preProcessedKDEData[std::pair(fromEventColumn, toEventColumn)][indexTriage][indexShift] = std::move(kdeData);
        }
    }
}

void MonteCarloSimulator::precomputeKDE(KDEData& kdeData) {
    const auto& data = kdeData.data;
    if (data.empty()) {
        return;
    }

    double std_dev = std::sqrt(
        std::inner_product(
            data.begin(), data.end(), data.begin(), 0.0
        ) / data.size() - pow(
            std::accumulate(data.begin(), data.end(), 0.0) / data.size(), 2
        )
    );
    double bandwidth = 1.06 * std_dev * pow(data.size(), -1.0 / 5.0);

    double min_val = *std::min_element(data.begin(), data.end());
    double max_val = *std::max_element(data.begin(), data.end());

    for (double i = min_val; i <= max_val; i += 1.0) {
        kdeData.points.push_back(i);
    }

    kdeData.densities.resize(kdeData.points.size(), 0.0);
    for (size_t i = 0; i < kdeData.points.size(); ++i) {
        for (const double& val : data) {
            kdeData.densities[i] += gaussian_kernel(kdeData.points[i], val, bandwidth);
        }
        kdeData.densities[i] /= data.size();
    }
}

double MonteCarloSimulator::sampleFromData(const KDEData& kdeData) {
    if (kdeData.points.empty()) {
        return 0.0;
    }

    std::discrete_distribution<> dist(kdeData.densities.begin(), kdeData.densities.end());

    return kdeData.points[dist(rnd)];
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

    for (int i = 0; i < Incidents::getInstance().size(); i++) {
        std::tm timeCallReceived = Incidents::getInstance().get<std::optional<std::tm>>("time_call_received", i).value();
        mktime(&timeCallReceived);

        // limit us to year 2018 (latest in dataset)
        if (timeCallReceived.tm_year != 118) {
            continue;
        }

        if (timeCallReceived.tm_yday == date.tm_yday - 1 && totalNight == -1) {
            int hour = Settings::get<int>("DAY_SHIFT_END") + 1 - static_cast<int>(Settings::get<bool>("SIMULATE_1_HOUR_BEFORE"));
            totalNight = 0;

            for (; hour < 24; hour++) {
                totalNight += Incidents::getInstance().get<int>(
                    "total_A_incidents_hour_" + std::to_string(hour),
                    i
                );
                totalNight += Incidents::getInstance().get<int>(
                    "total_H_incidents_hour_" + std::to_string(hour),
                    i
                );
                totalNight += Incidents::getInstance().get<int>(
                    "total_V1_incidents_hour_" + std::to_string(hour),
                    i
                );
            }
        }
        if (timeCallReceived.tm_yday == date.tm_yday) {
            int nightShiftEnds = Settings::get<int>("DAY_SHIFT_START") - 1;
            totalMorning = 0;

            for (int hour = 0; hour < nightShiftEnds + 1; hour++) {
                totalMorning += Incidents::getInstance().get<int>(
                    "total_A_incidents_hour_" + std::to_string(hour),
                    i
                );
                totalMorning += Incidents::getInstance().get<int>(
                    "total_H_incidents_hour_" + std::to_string(hour),
                    i
                );
                totalMorning += Incidents::getInstance().get<int>(
                    "total_V1_incidents_hour_" + std::to_string(hour),
                    i
                );
            }

            int hour = Settings::get<int>("DAY_SHIFT_START") - static_cast<int>(Settings::get<bool>("SIMULATE_1_HOUR_BEFORE"));
            int dayShiftEnds = Settings::get<int>("DAY_SHIFT_END");
            totalDay = 0;

            for (; hour < dayShiftEnds + 1; hour++) {
                totalDay += Incidents::getInstance().get<int>(
                    "total_A_incidents_hour_" + std::to_string(hour),
                    i
                );
                totalDay += Incidents::getInstance().get<int>(
                    "total_H_incidents_hour_" + std::to_string(hour),
                    i
                );
                totalDay += Incidents::getInstance().get<int>(
                    "total_V1_incidents_hour_" + std::to_string(hour),
                    i
                );
            }

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

    int warmupHour = static_cast<int>(Settings::get<bool>("SIMULATE_1_HOUR_BEFORE"));
    std::vector<std::pair<int, int>> indexRangesHour = dayShift ?
        std::vector<std::pair<int, int>>{{Settings::get<int>("DAY_SHIFT_START") - warmupHour, Settings::get<int>("DAY_SHIFT_END")}} :
        std::vector<std::pair<int, int>>{{0, Settings::get<int>("DAY_SHIFT_START") - 1}, {Settings::get<int>("DAY_SHIFT_END") + 1 - warmupHour, 23}};

    ProgressBar progressBar(totalEvents, "Generating events");
    for (int i = 0; i < totalEvents; i++) {
        Event event;

        event.id = i;

        // get call received
        int callReceivedHour = weightedLottery(rnd, hourlyIncidentProbabilityDistribution, indexRangesHour);
        int callReceivedMin = getRandomInt(rnd, 0, 59);
        int callReceivedSec = getRandomInt(rnd, 0, 59);

        event.callReceived = {0};
        event.callReceived.tm_year = year - 1900;
        event.callReceived.tm_mon = month - 1;
        event.callReceived.tm_mday = day;
        event.callReceived.tm_hour = callReceivedHour;
        event.callReceived.tm_min = callReceivedMin;
        event.callReceived.tm_sec = callReceivedSec;
        mktime(&event.callReceived);

        if (Settings::get<bool>("SIMULATE_1_HOUR_BEFORE")) {
            bool eventHappensDuringDayShiftWarmup = dayShift && callReceivedHour == Settings::get<int>("DAY_SHIFT_START") - warmupHour;
            bool eventHappensDuringNightShiftWarmup = !dayShift && callReceivedHour == Settings::get<int>("DAY_SHIFT_END") + 1 - warmupHour;

            if (eventHappensDuringDayShiftWarmup || eventHappensDuringNightShiftWarmup) {
                event.utility = true;
            }
        }

        // get triage impression
        int indexTriage = weightedLottery(rnd, triageProbabilityDistribution[callReceivedHour]);
        event.triageImpression = triageImpressions[indexTriage];

        // check if it should be canceled
        bool canceled = canceledProbability[indexTriage][indexShift] > getRandomDouble(rnd);

        // location
        event.gridId = indexToGridIdMapping[weightedLottery(rnd, locationProbabilityDistribution[indexTriage][indexShift])];

        // wait times
        event.secondsWaitCallAnswered = sampleFromData(
            preProcessedKDEData[std::pair("time_call_received", "time_incident_created")][indexTriage][indexShift]
        );
        event.secondsWaitAppointingResource = sampleFromData(
            preProcessedKDEData[std::pair("time_incident_created", "time_resource_appointed")][indexTriage][indexShift]
        );
        event.secondsWaitResourcePreparingDeparture = sampleFromData(
            preProcessedKDEData[std::pair("time_resource_appointed", "time_ambulance_dispatch_to_scene")][indexTriage][indexShift]
        );

        if (!canceled) {
            event.secondsWaitDepartureScene = sampleFromData(
                preProcessedKDEData[std::pair("time_ambulance_arrived_at_scene", "time_ambulance_dispatch_to_hospital")][indexTriage][indexShift]
            );
            event.secondsWaitAvailable = sampleFromData(
                preProcessedKDEData[std::pair("time_ambulance_arrived_at_hospital", "time_ambulance_available")][indexTriage][indexShift]
            );
        } else {
            event.secondsWaitAvailable = sampleFromData(
                preProcessedKDEData[std::pair("time_ambulance_arrived_at_scene", "time_ambulance_available")][indexTriage][indexShift]
            );
        }

        // setup timer
        event.timer = std::mktime(&event.callReceived);

        // remove event.secondsWaitAppointingResource
        event.updateTimer(static_cast<int>(event.secondsWaitCallAnswered), "duration_incident_creation");
        event.updateTimer(static_cast<int>(event.secondsWaitAppointingResource), "duration_resource_appointment");

        event.incidentGridId = event.gridId;

        events.push_back(event);

        progressBar.update(i + 1);
    }

    return events;
}

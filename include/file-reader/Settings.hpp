/**
 * @file Settings.hpp
 *
 * @copyright Copyright (c) 2024 Emergency-Optimizers
 */

#pragma once

/* external libraries */
#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <unordered_map>
#include <iomanip>
/* internal libraries */
#include "Utils.hpp"
#include "ProgressBar.hpp"

class Settings {
 private:
    static inline std::unordered_map<std::string, ValueType> configValues {};
    static inline SchemaMapping schema = {
        {"POPULATION_SIZE", &toInt},
        {"TOTAL_AMBULANCES_DURING_DAY", &toInt},
        {"TOTAL_AMBULANCES_DURING_NIGHT", &toInt},
        {"MUTATION_PROBABILITY", &toFloat},
        {"SIMULATE_YEAR", &toInt},
        {"SIMULATE_MONTH", &toInt},
        {"SIMULATE_DAY", &toInt},
        {"SIMULATE_DAY_SHIFT", &toBool},
        {"SIMULATION_GENERATION_WINDOW_SIZE", &toInt},
        {"DAY_SHIFT_START", &toInt},
        {"DAY_SHIFT_END", &toInt},
        {"HEURISTIC", &toString},
        {"USE_NSGA_FRONTS", &toBool},
        {"NSGA_WEIGHTS", &toVectorFloat},
        {"DISPATCH_STRATEGY", &toDispatchEngineStrategyType},
        {"NUM_TIME_SEGMENTS", &toInt},
        {"CROSSOVER_PROBABILITY", &toFloat},
        {"SEED", &toInt},
        {"SIMULATE_1_HOUR_BEFORE", &toBool},
        {"CROSSOVER", &toCrossoverType},
        {"GENOTYPE_INIT_TICKETS_RANDOM", &toDouble},
        {"GENOTYPE_INIT_TICKETS_UNIFORM", &toDouble},
        {"GENOTYPE_INIT_TICKETS_POPULATION_PROPORTIONATE_2KM", &toDouble},
        {"GENOTYPE_INIT_TICKETS_POPULATION_PROPORTIONATE_5KM", &toDouble},
        {"GENOTYPE_INIT_TICKETS_INCIDENT_PROPORTIONATE_2KM", &toDouble},
        {"GENOTYPE_INIT_TICKETS_INCIDENT_PROPORTIONATE_5KM", &toDouble},
        {"MUTATION_TICKETS_REDISTRIBUTE", &toDouble},
        {"MUTATION_TICKETS_SCRAMBLE", &toDouble},
        {"MUTATION_TICKETS_NEIGHBOR_DUPLICATION", &toDouble},
        {"OBJECTIVE_WEIGHT_AVG_RESPONSE_TIME_URBAN_A", &toDouble},
        {"OBJECTIVE_WEIGHT_AVG_RESPONSE_TIME_URBAN_H", &toDouble},
        {"OBJECTIVE_WEIGHT_AVG_RESPONSE_TIME_URBAN_V1", &toDouble},
        {"OBJECTIVE_WEIGHT_AVG_RESPONSE_TIME_RURAL_A", &toDouble},
        {"OBJECTIVE_WEIGHT_AVG_RESPONSE_TIME_RURAL_H", &toDouble},
        {"OBJECTIVE_WEIGHT_AVG_RESPONSE_TIME_RURAL_V1", &toDouble},
        {"OBJECTIVE_WEIGHT_PERCENTAGE_VIOLATIONS", &toDouble},
        {"OBJECTIVE_WEIGHT_PERCENTAGE_VIOLATIONS_URBAN", &toDouble},
        {"OBJECTIVE_WEIGHT_PERCENTAGE_VIOLATIONS_RURAL", &toDouble},
        {"CROSSOVER_TICKETS_SINGLE_POINT", &toDouble},
        {"CROSSOVER_TICKETS_SEGMENT_SWAP", &toDouble},
        {"CROSSOVER_TICKETS_BEST_ALLOCATION", &toDouble},
        {"PARENT_SELECTION_TICKETS_TOURNAMENT", &toDouble},
        {"PARENT_SELECTION_TICKETS_ROULETTE_WHEEL", &toDouble},
        {"PARENT_SELECTION_TICKETS_ELITISM", &toDouble},
        {"PARENT_SELECTION_TICKETS_RANK", &toDouble},
        {"SURVIVOR_SELECTION_TICKETS_TOURNAMENT", &toDouble},
        {"SURVIVOR_SELECTION_TICKETS_ROULETTE_WHEEL", &toDouble},
        {"SURVIVOR_SELECTION_TICKETS_ELITISM", &toDouble},
        {"SURVIVOR_SELECTION_TICKETS_RANK", &toDouble},
        {"PARENT_SELECTION_TOURNAMENT_SIZE", &toInt},
        {"SURVIVOR_SELECTION_TOURNAMENT_SIZE", &toInt},
        {"PARENT_SELECTION_RANK_SELECTION_PRESSURE", &toDouble},
        {"SURVIVOR_SELECTION_RANK_SELECTION_PRESSURE", &toDouble},
        {"SURVIVOR_SELECTION_KEEP_N_BEST", &toInt},
        {"DISPATCH_STRATEGY_PRIORITIZE_TRIAGE", &toBool},
        {"SCHEDULE_BREAKS", &toBool},
        {"OBJECTIVES", &toVectorObjectiveType},
        {"UNIQUE_RUN_ID", &toString},
        {"MULTI_THREAD", &toBool},
        {"STOPPING_CRITERIA_TIME_MIN", &toFloat},
        {"STOPPING_CRITERIA_MAX_GENERATIONS", &toInt},
        {"DISPATCH_STRATEGY_RESPONSE_RESTRICTED", &toBool},
    };

 public:
    static void LoadSettings() {
        const std::string& filename = "../settings.txt";
        std::ifstream file(filename);

        // count total lines
        size_t totalLines = 0;
        std::string tempLine;
        while (std::getline(file, tempLine)) {
            totalLines++;
        }

        // reset file to beginning
        file.clear();
        file.seekg(0, std::ios::beg);

        // setup progressBar
        ProgressBar progressBar(totalLines, "Loading settings");

        std::string line;
        int linesRead = 0;

        while (getline(file, line)) {
            progressBar.update(++linesRead);

            if (line == "" || line[0] == '/') {
                continue;
            }

            std::istringstream is_line(line);
            std::string key;
            if (getline(is_line, key, ':')) {
                std::string value_str;
                if (getline(is_line, value_str)) {
                    // remove leading and trailing whitespace
                    value_str.erase(0, value_str.find_first_not_of(" \t"));
                    value_str.erase(value_str.find_last_not_of(" \t") + 1);
                    // convert and store the value according to the schema
                    auto it = schema.find(key);
                    if (it != schema.end()) {
                        configValues[key] = it->second(value_str);
                    }
                }
            }
        }

        file.close();

        // add current time as unique run ID for file saving
        std::chrono::system_clock::time_point now = std::chrono::system_clock::now();
        std::time_t now_time = std::chrono::system_clock::to_time_t(now);

        std::tm bt = getLocalTime(now_time);
        std::ostringstream oss;
        oss << std::put_time(&bt, "%Y_%m_%d_%H_%M_%S");
        std::string timestamp = oss.str();

        // convert and store the value according to the schema
        auto it = schema.find("UNIQUE_RUN_ID");
        if (it != schema.end()) {
            configValues["UNIQUE_RUN_ID"] = it->second(timestamp);
        }
    }

    template<typename T>
    static const T get(const std::string& key) {
        auto it = configValues.find(key);
        if (it == configValues.end()) {
            throwError("Variable not found in settings '" + key + "'.");
        }

        const auto& value = it->second;
        if (!std::holds_alternative<T>(value)) {
            throwError("Requested variable type doesn't match the variable in settings.");
            throw std::bad_variant_access();
        }

        return std::get<T>(value);
    }
};

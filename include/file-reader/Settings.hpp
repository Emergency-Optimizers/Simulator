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
/* internal libraries */
#include "Utils.hpp"
#include "ProgressBar.hpp"

class Settings {
 private:
    static inline std::unordered_map<std::string, ValueType> configValues {};
    static inline SchemaMapping schema = {
        {"POPULATION_SIZE", &toInt},
        {"GENERATION_SIZE", &toInt},
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
        {"GENOTYPE_INIT_TICKETS_EVEN", &toDouble},
        {"MUTATION_TICKETS_REDISTRIBUTE", &toDouble},
        {"OBJECTIVE_WEIGHT_AVG_RESPONSE_TIME_URBAN_A", &toDouble},
        {"OBJECTIVE_WEIGHT_AVG_RESPONSE_TIME_URBAN_H", &toDouble},
        {"OBJECTIVE_WEIGHT_AVG_RESPONSE_TIME_URBAN_V1", &toDouble},
        {"OBJECTIVE_WEIGHT_AVG_RESPONSE_TIME_RURAL_A", &toDouble},
        {"OBJECTIVE_WEIGHT_AVG_RESPONSE_TIME_RURAL_H", &toDouble},
        {"OBJECTIVE_WEIGHT_AVG_RESPONSE_TIME_RURAL_V1", &toDouble},
        {"OBJECTIVE_WEIGHT_NUM_VIOLATIONS", &toDouble},
        {"CROSSOVER_TICKETS_SINGLE_POINT", &toDouble},
        {"PARENT_SELECTION_TICKETS_TOURNAMENT", &toDouble},
        {"PARENT_SELECTION_TICKETS_ROULETTE_WHEEL", &toDouble},
        {"PARENT_SELECTION_TICKETS_ELITISM", &toDouble},
        {"PARENT_SELECTION_TICKETS_RANK", &toDouble},
    };

 public:
    static void LoadSettings() {
        const std::string& filename = "../settings.txt";
        std::ifstream file(filename);

        // count total lines
        size_t totalLines = 0;
        double progress = 0;
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
    }

    template<typename T>
    static const T get(const std::string& key) {
        auto it = configValues.find(key);
        if (it == configValues.end()) {
            throw std::runtime_error("Key not found");
        }

        const auto& value = it->second;
        if (!std::holds_alternative<T>(value)) {
            throw std::bad_variant_access();
        }

        return std::get<T>(value);
    }
};

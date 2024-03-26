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
    };

 public:
    static void LoadSettings() {
        std::cout << "Loading Settings..." << std::flush;

        const std::string& filename = "../settings.txt";
        std::ifstream file(filename);
        std::string line;

        while (getline(file, line)) {
            if (line == "") {
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

        std::cout << "\rLoading Settings... " << "\tDONE" << std::endl;
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

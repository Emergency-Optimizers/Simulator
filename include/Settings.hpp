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
        {"POPULATION_SIZE", &Utils::toInt},
        {"GENERATION_SIZE", &Utils::toInt},
        {"TOTAL_AMBULANCES_DURING_DAY", &Utils::toInt},
        {"TOTAL_AMBULANCES_DURING_NIGHT", &Utils::toInt},
        {"MUTATION_PROBABILITY", &Utils::toFloat},
        {"SIMULATE_YEAR", &Utils::toInt},
        {"SIMULATE_MONTH", &Utils::toInt},
        {"SIMULATE_DAY", &Utils::toInt},
        {"SIMULATE_DAY_SHIFT", &Utils::toBool},
        {"SIMULATION_GENERATION_WINDOW_SIZE", &Utils::toInt}
    };

 public:
    static void LoadSettings() {
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

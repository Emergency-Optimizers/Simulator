/**
 * @file ProgressBar.hpp
 *
 * @copyright Copyright (c) 2024 Emergency-Optimizers
 */

#pragma once

/* external libraries */
#include <string>
#include <chrono>

#define PBSTR "||||||||||||||||||||||||||||||||||||||||"
#define PBWIDTH 40

class ProgressBar {
 private:
    const size_t maxProgress;
    std::string prefix;
    double prevPercentage = -1.0;
    const size_t prefixWidth = 30;
    std::chrono::time_point<std::chrono::steady_clock> startTime;

    std::string formatDuration(std::chrono::seconds duration);

 public:
    ProgressBar(
        const size_t maxProgress = 0,
        const std::string prefix = "",
        const std::string& postfix = ""
    );

    void update(const size_t currentProgress, const std::string& postfix = "", const bool autoStop = true, const bool lastPrint = false);
};

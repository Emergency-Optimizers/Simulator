/**
 * @file ProgressBar.cpp
 *
 * @copyright Copyright (c) 2024 Emergency-Optimizers
 */

/* external libraries */
#include <sstream>
#include <iomanip>
#include <iostream>
/* internal libraries */
#include "ProgressBar.hpp"

ProgressBar::ProgressBar(
    const size_t maxProgress,
    const std::string prefix,
    const std::string& postfix
) : maxProgress(maxProgress), prefix(prefix), startTime(std::chrono::steady_clock::now()) {
    if (prefix.length() > prefixWidth) {
        this->prefix = prefix.substr(0, prefixWidth);
    } else {
        this->prefix += std::string(prefixWidth - prefix.length(), ' ');
    }

    update(0, postfix);
}

void ProgressBar::update(const size_t currentProgress, const std::string& postfix) {
    auto now = std::chrono::steady_clock::now();
    auto elapsed = now - startTime;
    double percentage = static_cast<double>(currentProgress) / static_cast<double>(maxProgress);

    if (percentage != 1.0 && prevPercentage != -1 && percentage - prevPercentage < 0.01) {
        return;
    } else {
        prevPercentage = percentage;
    }

    // ensure the percentage is within the valid range
    if (percentage > 1.0) {
        percentage = 1.0;
    }
    if (percentage < 0.0) {
        percentage = 0.0;
    }

    int val = static_cast<int>(percentage * 100);
    int lpad = static_cast<int>(percentage * PBWIDTH);
    int rpad = PBWIDTH - lpad;

    // calculate elapsed and estimated total time
    auto elapsedSeconds = std::chrono::duration_cast<std::chrono::seconds>(elapsed).count();

    std::string timeInfo = "(" + formatDuration(std::chrono::seconds(elapsedSeconds)) + ")";

    // create a stringstream to build the progress bar string
    std::stringstream ss;
    ss << "\r" << prefix << " ["
       << std::string(lpad, '|') << std::string(rpad, ' ')
       << "] " << std::setw(3) << val << "% " << timeInfo << " " << postfix;

    // convert stringstream to string for its length
    std::string progressBarString = ss.str();

    std::cout << progressBarString;

    if (percentage == 1.0) {
        std::cout << std::endl;
    }
}

std::string ProgressBar::formatDuration(std::chrono::seconds duration) {
    int hours = std::chrono::duration_cast<std::chrono::hours>(duration).count();
    int minutes = std::chrono::duration_cast<std::chrono::minutes>(duration % std::chrono::hours(1)).count();
    int seconds = std::chrono::duration_cast<std::chrono::seconds>(duration % std::chrono::minutes(1)).count();

    std::stringstream ss;
    // format as hh:mm:ss
    ss << std::setw(2) << std::setfill('0') << hours << ":"
       << std::setw(2) << std::setfill('0') << minutes << ":"
       << std::setw(2) << std::setfill('0') << seconds;

    return ss.str();
}

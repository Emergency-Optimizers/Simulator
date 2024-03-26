/**
 * @file ProgressBar.hpp
 *
 * @copyright Copyright (c) 2024 Emergency-Optimizers
 */

#pragma once

/* external libraries */
#include <string>

#define PBSTR "||||||||||||||||||||||||||||||||||||||||"
#define PBWIDTH 40

class ProgressBar {
 private:
    const size_t maxProgress;
    const std::string prefix;
    const std::string postfix;
    double prevPercentage = -1.0;

 public:
    ProgressBar(
        const size_t maxProgress = 0,
        const std::string prefix = "",
        const std::string postfix = ""
    );

    void update(size_t currentProgress);
};

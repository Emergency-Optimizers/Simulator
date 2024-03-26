/**
 * @file ProgressBar.cpp
 *
 * @copyright Copyright (c) 2024 Emergency-Optimizers
 */

/* external libraries */
#include <stdio.h>
#include <string.h>
/* internal libraries */
#include "ProgressBar.hpp"

ProgressBar::ProgressBar(
    const size_t maxProgress,
    const std::string prefix,
    const std::string postfix
) : maxProgress(maxProgress), prefix(prefix), postfix(postfix) {
    update(0);
}

void ProgressBar::update(size_t currentProgress) {
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

    // print the progress bar
    printf("\r%s %3d%% [%.*s%*s] %s", prefix.c_str(), val, lpad, PBSTR, rpad, "", postfix.c_str());

    if (percentage != 1.0) {
        fflush(stdout);
    } else {
        printf("\n");
    }
}

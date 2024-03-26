/**
 * @file PopulationNSGA.cpp
 *
 * @copyright Copyright (c) 2024 Emergency-Optimizers
 */

#pragma once

/* external libraries */
#include <stdio.h>
#include <string.h>

#define PBSTR "||||||||||||||||||||||||||||||||||||||||"
#define PBWIDTH 40

/**
 * Prints a progress bar with an optional prefix text.
 * Ensures that the progress bar doesn't flush at 100%.
 * 
 * @param percentage The progress percentage between 0.0 and 1.0.
 * @param prefix Optional text to display after the progress bar.
 */
void printProgress(double percentage, const char* prefix = "", const char* postfix = "") {
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
    printf("\r%s %3d%% [%.*s%*s] %s", prefix, val, lpad, PBSTR, rpad, "", postfix);

    if (percentage != 1.0) {
        fflush(stdout);
    } else {
        printf("\n");
    }
}

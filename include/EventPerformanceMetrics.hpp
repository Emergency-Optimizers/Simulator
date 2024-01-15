/**
 * @file EventPerformanceMetrics.hpp
 * @author Sindre Eiklid
 * @version 1.0
 * @date 2024-01-15
 *
 * @copyright Copyright (c) 2023 Sindre Eiklid
 */

#pragma once

struct EventPerformanceMetrics {
    int callProcessedTime;
    int dispatchToSceneTime;
    int arrivalAtSceneTime;
    int dispatchToHospitalTime;
    int arrivalAtHospitalTime;
    int dispatchToDepotTime;

    void print() const {
        std::cout << "Call Processed Time: " << callProcessedTime << " seconds\n";
        std::cout << "Dispatch to Scene Time: " << dispatchToSceneTime << " seconds\n";
        std::cout << "Arrival at Scene Time: " << arrivalAtSceneTime << " seconds\n";
        std::cout << "Dispatch to Hospital Time: " << dispatchToHospitalTime << " seconds\n";
        std::cout << "Arrival at Hospital Time: " << arrivalAtHospitalTime << " seconds\n";
        std::cout << "Dispatch to Depot Time: " << dispatchToDepotTime << " seconds\n";
    }
};

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
};

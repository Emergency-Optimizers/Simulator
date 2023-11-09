/**
 * @file IncidentStatus.hpp
 * @author Sindre Eiklid
 * @version 1.0
 * @date 2023-11-09
 *
 * @copyright Copyright (c) 2023 Sindre Eiklid
 */

#pragma once

enum class IncidentStatus {
    STANDBY,
    TRAVELLING_TO_SCENE,
    ARRIVED_AT_SCENE,
    TRAVELLING_TO_HOSPITAL,
    ARRIVED_AT_HOSPITAL,
    TRAVELLING_TO_DEPOT,
};

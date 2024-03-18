/**
 * @file EventType.hpp
 *
 * @copyright Copyright (c) 2024 Emergency-Optimizers
 */

#pragma once

enum class EventType {
    NONE,
    ASSIGNING_AMBULANCE,
    DISPATCHING_TO_SCENE,
    DISPATCHING_TO_HOSPITAL,
    DISPATCHING_TO_DEPOT,
    FINISHED
};

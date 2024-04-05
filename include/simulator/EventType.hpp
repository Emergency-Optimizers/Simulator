/**
 * @file EventType.hpp
 *
 * @copyright Copyright (c) 2024 Emergency-Optimizers
 */

#pragma once

enum class EventType {
    NONE,
    RESOURCE_APPOINTMENT,
    DISPATCHING_TO_SCENE,
    DISPATCHING_TO_HOSPITAL,
    PREPARING_DISPATCH_TO_DEPOT,
    DISPATCHING_TO_DEPOT,
    REALLOCATE
};

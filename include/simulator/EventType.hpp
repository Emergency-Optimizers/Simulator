/**
 * @file EventType.hpp
 *
 * @copyright Copyright (c) 2024 Emergency-Optimizers
 */

#pragma once

enum class EventType {
    NONE,
    CALL_PROCESSED,
    DISPATCH_TO_SCENE,
    ARRIVED_AT_SCENE,
    DISPATCH_TO_HOSPITAL,
    ARRIVED_AT_HOSPITAL,
    DISPATCH_TO_DEPOT,

    ASSIGNING_AMBULANCE,
    DISPATCHING_TO_SCENE,
    DISPATCHING_TO_HOSPITAL,
    DISPATCHING_TO_DEPOT,
    FINISHED
};

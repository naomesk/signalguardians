#pragma once

// Include this adapter only in the application's configured FreeRTOS build.
#include "FreeRTOS.h"
#include "task.h"

#include "signalguardians/scheduling.hpp"

#if configUSE_PREEMPTION != 1
#error "Signal Guardians requires configUSE_PREEMPTION == 1"
#endif

// FreeRTOS enables time slicing by default when this setting is omitted.
#if defined(configUSE_TIME_SLICING) && configUSE_TIME_SLICING != 1
#error "Signal Guardians requires configUSE_TIME_SLICING == 1"
#endif

#if defined(configNUMBER_OF_CORES) && configNUMBER_OF_CORES != 1
#error "This initial scheduling policy targets a single-core FreeRTOS build"
#endif

namespace signalguardians {

static_assert(configMAX_PRIORITIES >= TaskPriorities::RequiredLevels,
              "Signal Guardians requires at least five task priority levels");

// Pass these constants as uxPriority to xTaskCreate/xTaskCreateStatic.
// This header validates the configuration; it does not create tasks.
struct FreeRtosTaskPriorities {
    static constexpr UBaseType_t GeneralSender = TaskPriorities::GeneralSender;
    static constexpr UBaseType_t SystemSender = TaskPriorities::SystemSender;
    static constexpr UBaseType_t UrgentSender = TaskPriorities::UrgentSender;
    static constexpr UBaseType_t Receiver = TaskPriorities::Receiver;
    static constexpr UBaseType_t Retry = TaskPriorities::Retry;
    static constexpr UBaseType_t Logger = TaskPriorities::Logger;
};

} // namespace signalguardians

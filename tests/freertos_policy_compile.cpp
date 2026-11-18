// Compile-only integration check against real FreeRTOS headers and the target's
// configuration/port includes. This file does not run the kernel.
#include "signalguardians/freertos_task_priorities.hpp"

#include <array>
#include <type_traits>

using namespace signalguardians;

static_assert(std::is_same_v<decltype(FreeRtosTaskPriorities::Receiver), const UBaseType_t>);
static_assert(FreeRtosTaskPriorities::GeneralSender > tskIDLE_PRIORITY);
static_assert(FreeRtosTaskPriorities::UrgentSender < configMAX_PRIORITIES);

void checkTaskCreationPriorities(TaskFunction_t task, configSTACK_DEPTH_TYPE stackDepth) {
    constexpr std::array<UBaseType_t, 6> priorities{
        FreeRtosTaskPriorities::GeneralSender, FreeRtosTaskPriorities::SystemSender,
        FreeRtosTaskPriorities::UrgentSender, FreeRtosTaskPriorities::Receiver,
        FreeRtosTaskPriorities::Retry, FreeRtosTaskPriorities::Logger
    };
    for (const auto priority : priorities) {
        // Type-check the real API only. This function is never linked or called.
        (void)xTaskCreate(task, "PolicyCheck", stackDepth, nullptr, priority, nullptr);
    }
}

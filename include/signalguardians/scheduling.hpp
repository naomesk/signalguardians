#pragma once

#include "signalguardians/message.hpp"

#include <cstddef>
#include <optional>

namespace signalguardians {

// Counts must describe one synchronized snapshot of the pending message queues.
struct PendingMessageCounts {
    std::size_t general = 0;
    std::size_t system = 0;
    std::size_t urgent = 0;
};

// Select the next class at each dispatch boundary. The queue module preserves
// FIFO order within a class and synchronizes selection with removal.
std::optional<MessageType> selectNextMessageType(const PendingMessageCounts& pending) noexcept;

// Initial single-core policy; these are task priorities, not message ranks.
// All values are absolute FreeRTOS priorities (idle is 0).
struct TaskPriorities {
    static constexpr unsigned int GeneralSender = 1;
    static constexpr unsigned int SystemSender = 3;
    static constexpr unsigned int UrgentSender = 4;
    static constexpr unsigned int Receiver = 2;
    static constexpr unsigned int Retry = 3;
    static constexpr unsigned int Logger = 1;
    static constexpr unsigned int RequiredLevels = UrgentSender + 1;
};

} // namespace signalguardians

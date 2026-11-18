#include "signalguardians/scheduling.hpp"

#include <array>
#include <iostream>
#include <limits>
#include <stdexcept>

using namespace signalguardians;

void require(bool condition, const char* description) {
    if (!condition) {
        throw std::runtime_error(description);
    }
}

int main() {
    // Cover all combinations, including no pending work.
    const std::array<PendingMessageCounts, 8> pending{
        PendingMessageCounts{0, 0, 0}, {1, 0, 0}, {0, 1, 0}, {1, 1, 0},
        {0, 0, 1}, {1, 0, 1}, {0, 1, 1}, {1, 1, 1}
    };
    const std::array<std::optional<MessageType>, 8> expected{
        std::nullopt, MessageType::GENERAL, MessageType::SYSTEM, MessageType::SYSTEM,
        MessageType::URGENT, MessageType::URGENT, MessageType::URGENT, MessageType::URGENT
    };
    for (std::size_t i = 0; i < pending.size(); ++i) {
        require(selectNextMessageType(pending[i]) == expected[i], "Incorrect dispatch choice");
    }

    PendingMessageCounts backlog{100, 0, 0};
    require(selectNextMessageType(backlog) == MessageType::GENERAL, "General messages must make progress alone");
    ++backlog.urgent;
    require(selectNextMessageType(backlog) == MessageType::URGENT, "Late urgent arrival must outrank the general backlog");
    --backlog.urgent;
    ++backlog.system;
    require(selectNextMessageType(backlog) == MessageType::SYSTEM, "System must run after urgent drains");
    --backlog.system;
    require(selectNextMessageType(backlog) == MessageType::GENERAL, "General must resume after higher classes drain");
    backlog.general = 0;
    require(!selectNextMessageType(backlog), "Empty queues must report no work");

    const auto maximum = std::numeric_limits<std::size_t>::max();
    require(selectNextMessageType({maximum, maximum, 1}) == MessageType::URGENT,
            "Large counts must not overflow dispatch selection");

    static_assert(TaskPriorities::GeneralSender > 0);
    static_assert(TaskPriorities::UrgentSender > TaskPriorities::SystemSender);
    static_assert(TaskPriorities::SystemSender > TaskPriorities::GeneralSender);
    static_assert(TaskPriorities::UrgentSender > TaskPriorities::Receiver);
    static_assert(TaskPriorities::SystemSender > TaskPriorities::Receiver);
    static_assert(TaskPriorities::Receiver > TaskPriorities::GeneralSender);
    static_assert(TaskPriorities::Retry < TaskPriorities::UrgentSender);
    static_assert(TaskPriorities::Logger <= TaskPriorities::GeneralSender);
    static_assert(TaskPriorities::RequiredLevels > TaskPriorities::UrgentSender);

    std::cout << "All scheduling policy tests passed.\n";
}

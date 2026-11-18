#include "signalguardians/scheduling.hpp"

namespace signalguardians {

std::optional<MessageType> selectNextMessageType(const PendingMessageCounts& pending) noexcept {
    if (pending.urgent != 0) {
        return MessageType::URGENT;
    }
    if (pending.system != 0) {
        return MessageType::SYSTEM;
    }
    if (pending.general != 0) {
        return MessageType::GENERAL;
    }
    return std::nullopt;
}

} // namespace signalguardians

#include "signalguardians/message.hpp"

#include <cstring>
#include <limits>
#include <stdexcept>

namespace signalguardians {

Message MessageCreator::create(MessageType type, std::string_view payload) {
    if (MessageCategorizer::getPriority(type) == 0) {
        throw std::invalid_argument("Unknown message type");
    }
    if (payload.size() >= PayloadCapacity) {
        throw std::length_error("Payload must fit in 127 bytes plus a null terminator");
    }
    if (payload.find('\0') != std::string_view::npos) {
        throw std::invalid_argument("Payload must not contain embedded null bytes");
    }
    if (nextId_ > static_cast<unsigned int>(std::numeric_limits<int>::max())) {
        throw std::overflow_error("Message IDs exhausted");
    }

    Message message{};
    message.id = static_cast<int>(nextId_++);
    message.type = type;
    if (!payload.empty()) {
        std::memcpy(message.payload, payload.data(), payload.size());
    }
    // Value initialization sets retryCount to zero and terminates payload.
    return message;
}

std::string_view MessageCategorizer::getTypeName(MessageType type) noexcept {
    switch (type) {
    case MessageType::GENERAL: return "GENERAL";
    case MessageType::SYSTEM:  return "SYSTEM";
    case MessageType::URGENT:  return "URGENT";
    default:                  return "UNKNOWN";
    }
}

int MessageCategorizer::getPriority(MessageType type) noexcept {
    // Application rules, independent of FreeRTOS task priorities.
    switch (type) {
    case MessageType::GENERAL: return 1;
    case MessageType::SYSTEM:  return 2;
    case MessageType::URGENT:  return 3;
    default:                  return 0;
    }
}

} // namespace signalguardians

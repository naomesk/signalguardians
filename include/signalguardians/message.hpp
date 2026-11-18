#pragma once

#include <string_view>
#include <type_traits>

namespace signalguardians {

enum class MessageType {
    // GENERAL is called Normal in the team overview; URGENT is Emergency.
    GENERAL = 1,
    SYSTEM = 2,
    URGENT = 3
};

inline constexpr unsigned int PayloadCapacity = 128;

// Fixed-size storage can later be copied into a FreeRTOS queue.
struct Message {
    int id;
    MessageType type;
    char payload[PayloadCapacity];
    int retryCount;
};

static_assert(std::is_trivially_copyable_v<Message>, "Messages must support FreeRTOS queue copying");
static_assert(std::is_standard_layout_v<Message>, "Messages must retain a simple data layout");

class MessageCreator {
public:
    MessageCreator() = default;
    MessageCreator(const MessageCreator&) = delete;
    MessageCreator& operator=(const MessageCreator&) = delete;

    // Share one creator to keep IDs unique. Calls must be serialized.
    Message create(MessageType type, std::string_view payload);

private:
    unsigned int nextId_ = 1;
};

class MessageCategorizer {
public:
    static std::string_view getTypeName(MessageType type) noexcept;
    static int getPriority(MessageType type) noexcept;
};

} // namespace signalguardians

#include "signalguardians/message.hpp"

#include <cstring>
#include <iostream>
#include <stdexcept>
#include <string>
#include <type_traits>

using namespace signalguardians;

void require(bool condition, const char* description) {
    if (!condition) {
        throw std::runtime_error(description);
    }
}

template <typename Exception, typename Function>
void requireThrows(Function function) {
    try {
        function();
    } catch (const Exception&) {
        return;
    }
    throw std::runtime_error("Expected exception was not thrown");
}

int main() {
    static_assert(std::is_trivially_copyable_v<Message>);

    MessageCreator creator;
    const auto first = creator.create(MessageType::GENERAL, "Temperature: 25C");
    const auto second = creator.create(MessageType::URGENT, "Oxygen level critical!");
    require(first.id == 1 && second.id == 2, "IDs must be sequential and distinct");
    require(first.type == MessageType::GENERAL, "Creator must preserve the type");
    require(std::strcmp(first.payload, "Temperature: 25C") == 0, "Creator must preserve payload");
    require(first.retryCount == 0 && second.retryCount == 0, "New messages must have zero retries");

    const auto empty = creator.create(MessageType::SYSTEM, {});
    require(empty.payload[0] == '\0', "Empty payload must be terminated");
    const auto full = creator.create(MessageType::SYSTEM, std::string(127, 'x'));
    require(std::strlen(full.payload) == 127, "127-byte payload must fit");

    requireThrows<std::length_error>([&] {
        creator.create(MessageType::GENERAL, std::string(128, 'x'));
    });
    requireThrows<std::invalid_argument>([&] {
        creator.create(MessageType::GENERAL, std::string_view("a\0b", 3));
    });
    const auto unknown = static_cast<MessageType>(99);
    requireThrows<std::invalid_argument>([&] { creator.create(unknown, "Invalid"); });
    require(creator.create(MessageType::GENERAL, "Next").id == full.id + 1,
            "Rejected messages must not consume IDs");

    require(MessageCategorizer::getTypeName(MessageType::GENERAL) == "GENERAL", "GENERAL name");
    require(MessageCategorizer::getTypeName(MessageType::SYSTEM) == "SYSTEM", "SYSTEM name");
    require(MessageCategorizer::getTypeName(MessageType::URGENT) == "URGENT", "URGENT name");
    require(MessageCategorizer::getPriority(MessageType::GENERAL) == 1, "GENERAL priority");
    require(MessageCategorizer::getPriority(MessageType::SYSTEM) == 2, "SYSTEM priority");
    require(MessageCategorizer::getPriority(MessageType::URGENT) == 3, "URGENT priority");
    require(MessageCategorizer::getTypeName(unknown) == "UNKNOWN", "Unknown name fallback");
    require(MessageCategorizer::getPriority(unknown) == 0, "Unknown priority fallback");

    std::cout << "All message tests passed.\n";
}

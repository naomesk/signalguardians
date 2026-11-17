#include "signalguardians/message.hpp"

#include <array>
#include <iostream>

int main() {
    using namespace signalguardians;

    MessageCreator creator;
    const std::array<Message, 3> messages{
        creator.create(MessageType::GENERAL, "Temperature: 25C"),
        creator.create(MessageType::URGENT, "Oxygen level critical!"),
        creator.create(MessageType::SYSTEM, "Communication restarted")
    };

    for (const auto& message : messages) {
        std::cout << "ID: " << message.id
                  << " | Type: " << MessageCategorizer::getTypeName(message.type)
                  << " | Message: " << message.payload << '\n';
    }
}

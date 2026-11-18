#include "signalguardians/scheduling.hpp"

#include <array>
#include <deque>
#include <iostream>

int main() {
    using namespace signalguardians;

    // Host-only queues illustrate the contract for the team's queue module.
    std::array<std::deque<Message>, 3> queues;
    MessageCreator creator;
    const auto enqueue = [&](MessageType type, const char* payload) {
        queues[static_cast<std::size_t>(type) - 1].push_back(creator.create(type, payload));
    };

    enqueue(MessageType::GENERAL, "Temperature: 25C");
    enqueue(MessageType::GENERAL, "Battery: 90%");
    enqueue(MessageType::SYSTEM, "Communication restarted");
    enqueue(MessageType::URGENT, "Oxygen level critical!");
    enqueue(MessageType::URGENT, "Abort docking!");

    while (const auto next = selectNextMessageType({queues[0].size(), queues[1].size(), queues[2].size()})) {
        auto& queue = queues[static_cast<std::size_t>(*next) - 1];
        const auto& message = queue.front();
        std::cout << "ID: " << message.id
                  << " | Type: " << MessageCategorizer::getTypeName(message.type)
                  << " | Priority: " << MessageCategorizer::getPriority(message.type)
                  << " | Message: " << message.payload << '\n';
        queue.pop_front();
    }
}

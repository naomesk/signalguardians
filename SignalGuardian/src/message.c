#include "signalguardians/message.h"

#include <limits.h>
#include <string.h>

MessageCreateResult message_create(MessageCreator *creator, MessageType type,
                                   const char *payload, Message *out)
{
    if (creator == NULL || payload == NULL || out == NULL ||
        creator->nextId == 0 || message_priority(type) == 0) {
        return MESSAGE_INVALID_ARGUMENT;
    }

    const size_t length = strlen(payload);
    if (length >= MESSAGE_PAYLOAD_CAPACITY) {
        return MESSAGE_PAYLOAD_TOO_LONG;
    }
    if (creator->nextId > (unsigned int)INT_MAX) {
        return MESSAGE_IDS_EXHAUSTED;
    }

    Message message = {0};
    message.id = (int)creator->nextId++;
    message.type = type;
    memcpy(message.payload, payload, length + 1);
    *out = message;
    return MESSAGE_CREATED;
}

const char *message_type_name(MessageType type)
{
    switch (type) {
        case MESSAGE_GENERAL: return "GENERAL";
        case MESSAGE_SYSTEM:  return "SYSTEM";
        case MESSAGE_URGENT:  return "URGENT";
        default:              return "UNKNOWN";
    }
}

int message_priority(MessageType type)
{
    switch (type) {
        case MESSAGE_GENERAL: return 1;
        case MESSAGE_SYSTEM:  return 2;
        case MESSAGE_URGENT:  return 3;
        default:              return 0;
    }
}

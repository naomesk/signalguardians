#ifndef SIGNALGUARDIANS_MESSAGE_H
#define SIGNALGUARDIANS_MESSAGE_H

enum { MESSAGE_PAYLOAD_CAPACITY = 128 };

typedef enum {
    MESSAGE_GENERAL = 1,
    MESSAGE_SYSTEM = 2,
    MESSAGE_URGENT = 3
} MessageType;

typedef struct {
    int id;
    MessageType type;
    char payload[MESSAGE_PAYLOAD_CAPACITY];
    int retryCount;
} Message;

typedef struct {
    /* Initialize to 1 and serialize calls when sharing across tasks. */
    unsigned int nextId;
} MessageCreator;

typedef enum {
    MESSAGE_CREATED,
    MESSAGE_INVALID_ARGUMENT,
    MESSAGE_PAYLOAD_TOO_LONG,
    MESSAGE_IDS_EXHAUSTED
} MessageCreateResult;

MessageCreateResult message_create(MessageCreator *creator, MessageType type,
                                   const char *payload, Message *out);
const char *message_type_name(MessageType type);
int message_priority(MessageType type);

#endif

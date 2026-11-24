#ifndef SIGNALGUARDIANS_SCHEDULING_H
#define SIGNALGUARDIANS_SCHEDULING_H

#include "signalguardians/message.h"

#include <stdbool.h>
#include <stddef.h>

enum {
    PRIORITY_GENERAL_SENDER = 1,
    PRIORITY_LOGGER = 1,
    PRIORITY_RECEIVER = 2,
    PRIORITY_SYSTEM_SENDER = 3,
    PRIORITY_RETRY = 3,
    PRIORITY_URGENT_SENDER = 4,
    REQUIRED_TASK_PRIORITY_LEVELS = PRIORITY_URGENT_SENDER + 1
};

typedef struct {
    size_t general;
    size_t system;
    size_t urgent;
} PendingMessageCounts;

/* The receiver supplies a consistent snapshot of the three queues. */
bool select_next_message_type(PendingMessageCounts waiting, MessageType *next);

#endif

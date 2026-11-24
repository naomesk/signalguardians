#ifndef SIGNALGUARDIANS_MESSAGE_QUEUE_H
#define SIGNALGUARDIANS_MESSAGE_QUEUE_H

#include "FreeRTOS.h"
#include "queue.h"
#include "signalguardians/message.h"

#include <stdbool.h>

typedef struct {
    QueueHandle_t general;
    QueueHandle_t system;
    QueueHandle_t urgent;
} MessageQueues;

/* Call before starting tasks. Each lane has its own reserved capacity. */
bool message_queues_create(MessageQueues *queues, UBaseType_t capacity_per_type);
/* Task context only. Return pdFAIL for a full lane or an invalid type. */
BaseType_t message_queues_send(const MessageQueues *queues, const Message *message);
/* Task context only. Select and remove the oldest message of the highest waiting type. */
BaseType_t message_queues_try_receive(const MessageQueues *queues, Message *out);

#endif

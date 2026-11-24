#include "signalguardians/message_queue.h"

#include "task.h"
#include "signalguardians/scheduling.h"

static QueueHandle_t queue_for_type(const MessageQueues *queues, MessageType type)
{
    switch (type) {
        case MESSAGE_GENERAL: return queues->general;
        case MESSAGE_SYSTEM:  return queues->system;
        case MESSAGE_URGENT:  return queues->urgent;
        default:              return NULL;
    }
}

bool message_queues_create(MessageQueues *queues, UBaseType_t capacity_per_type)
{
    *queues = (MessageQueues){0};
    if (capacity_per_type == 0) {
        return false;
    }

    queues->general = xQueueCreate(capacity_per_type, sizeof(Message));
    queues->system = xQueueCreate(capacity_per_type, sizeof(Message));
    queues->urgent = xQueueCreate(capacity_per_type, sizeof(Message));
    if (queues->general != NULL && queues->system != NULL && queues->urgent != NULL) {
        return true;
    }

    if (queues->general != NULL) {
        vQueueDelete(queues->general);
    }
    if (queues->system != NULL) {
        vQueueDelete(queues->system);
    }
    if (queues->urgent != NULL) {
        vQueueDelete(queues->urgent);
    }
    *queues = (MessageQueues){0};
    return false;
}

BaseType_t message_queues_send(const MessageQueues *queues, const Message *message)
{
    QueueHandle_t queue = queue_for_type(queues, message->type);
    if (queue == NULL) {
        return pdFAIL;
    }
    return xQueueSendToBack(queue, message, 0);
}

BaseType_t message_queues_try_receive(const MessageQueues *queues, Message *out)
{
    BaseType_t status = pdFAIL;
    MessageType next;

    /* Single-core: no producer can run between the snapshot and dequeue. */
    taskENTER_CRITICAL();
    const PendingMessageCounts waiting = {
        uxQueueMessagesWaiting(queues->general),
        uxQueueMessagesWaiting(queues->system),
        uxQueueMessagesWaiting(queues->urgent)
    };
    if (select_next_message_type(waiting, &next)) {
        status = xQueueReceive(queue_for_type(queues, next), out, 0);
    }
    taskEXIT_CRITICAL();

    return status;
}

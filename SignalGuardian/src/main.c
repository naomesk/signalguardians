#include "FreeRTOS.h"
#include "task.h"

#include "signalguardians/message_queue.h"
#include "signalguardians/scheduling.h"

#include <stdio.h>
#include <stdlib.h>

#if configUSE_PREEMPTION != 1 || configUSE_TIME_SLICING != 1
#error "Signal Guardians requires preemption and time slicing"
#endif

#if defined(configNUMBER_OF_CORES) && configNUMBER_OF_CORES != 1
#error "The queue selection rule requires a single FreeRTOS core"
#endif

_Static_assert(configMAX_PRIORITIES >= REQUIRED_TASK_PRIORITY_LEVELS,
               "Signal Guardians needs at least five FreeRTOS priority levels");

enum { DEMO_QUEUE_CAPACITY = 2 };

typedef struct {
    MessageType type;
    const char *payloads[DEMO_QUEUE_CAPACITY];
    TaskHandle_t nextTask;
} SenderBatch;

static MessageQueues queues;
static MessageCreator creator = {1};
static SenderBatch generalBatch = {MESSAGE_GENERAL, {"Temperature: 25C", "Battery: 90%"}, NULL};
static SenderBatch systemBatch = {MESSAGE_SYSTEM, {"Communication restarted"}, NULL};
static SenderBatch urgentBatch = {MESSAGE_URGENT, {"Oxygen level critical!", "Abort docking!"}, NULL};

static void sender_task(void *context)
{
    const SenderBatch *batch = context;
    if (batch->type != MESSAGE_GENERAL) {
        ulTaskNotifyTake(pdTRUE, portMAX_DELAY);
    }

    /* The demo releases one sender at a time, so the creator is shared safely. */
    for (size_t i = 0; i < DEMO_QUEUE_CAPACITY && batch->payloads[i] != NULL; ++i) {
        Message message;
        if (message_create(&creator, batch->type, batch->payloads[i], &message) != MESSAGE_CREATED ||
            message_queues_send(&queues, &message) != pdPASS) {
            fprintf(stderr, "Failed to enqueue %s message.\n", message_type_name(batch->type));
            exit(EXIT_FAILURE);
        }
    }

    xTaskNotifyGive(batch->nextTask);
    vTaskDelete(NULL);
}

static void receiver_task(void *context)
{
    (void)context;
    /* Wait for the entire demo burst so the dispatch order is reproducible. */
    ulTaskNotifyTake(pdTRUE, portMAX_DELAY);

    Message message;
    while (message_queues_try_receive(&queues, &message) == pdPASS) {
        printf("ID: %d | Type: %s | Priority: %d | Message: %s\n",
               message.id, message_type_name(message.type),
               message_priority(message.type), message.payload);
    }

    /* The Win32 port has no return path to main after a successful start. */
    exit(EXIT_SUCCESS);
}

int main(void)
{
    if (!message_queues_create(&queues, DEMO_QUEUE_CAPACITY) ||
        xTaskCreate(receiver_task, "Receiver", configMINIMAL_STACK_SIZE, NULL,
                    PRIORITY_RECEIVER, &urgentBatch.nextTask) != pdPASS ||
        xTaskCreate(sender_task, "UrgentSender", configMINIMAL_STACK_SIZE, &urgentBatch,
                    PRIORITY_URGENT_SENDER, &systemBatch.nextTask) != pdPASS ||
        xTaskCreate(sender_task, "SystemSender", configMINIMAL_STACK_SIZE, &systemBatch,
                    PRIORITY_SYSTEM_SENDER, &generalBatch.nextTask) != pdPASS ||
        xTaskCreate(sender_task, "GeneralSender", configMINIMAL_STACK_SIZE, &generalBatch,
                    PRIORITY_GENERAL_SENDER, NULL) != pdPASS) {
        fputs("FreeRTOS queue or task creation failed.\n", stderr);
        return EXIT_FAILURE;
    }

    vTaskStartScheduler();
    fputs("FreeRTOS scheduler failed to start.\n", stderr);
    return EXIT_FAILURE;
}

#ifndef FREERTOS_CONFIG_H
#define FREERTOS_CONFIG_H

#include <stdlib.h>

#define configUSE_PREEMPTION                    1
#define configUSE_TIME_SLICING                  1
#define configUSE_PORT_OPTIMISED_TASK_SELECTION 0
#define configMAX_PRIORITIES                    5
#define configTICK_RATE_HZ                       1000
#define configMINIMAL_STACK_SIZE                 128
#define configTOTAL_HEAP_SIZE                    (32 * 1024)
#define configMAX_TASK_NAME_LEN                  16
#define configUSE_16_BIT_TICKS                   0
#define configUSE_IDLE_HOOK                      0
#define configUSE_TICK_HOOK                      0
#define configSUPPORT_DYNAMIC_ALLOCATION         1
#define INCLUDE_vTaskDelete                      1
#define INCLUDE_vTaskSuspend                     1

#define configASSERT(condition) do { if (!(condition)) { abort(); } } while (0)

#endif

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "FreeRTOS.h"
#include "task.h"

#include "config.h"
#include "msg.h"
#include "retry.h"
#include "queue_mod.h"
#include "logger.h"

#define TOL_MS 3

static TaskHandle_t s_testTask;
static volatile TickType_t s_resentTick;
static Message_t  s_resentMsg;
static volatile int s_failedCount;
static int s_errors;

BaseType_t Queue_Send(const Message_t *m, TickType_t timeout)
{
    (void)timeout;
    s_resentMsg  = *m;
    s_resentTick = xTaskGetTickCount();
    xTaskNotifyGive(s_testTask);
    return pdPASS;
}

void Log_Event(EventType_t ev, const Message_t *m)
{
    const char *name = (ev == EV_FAILED) ? "EV_FAILED" : (ev == EV_RETRIED) ? "EV_RETRIED" : "EV_OTHER";
    printf("[%lu] %s id=%lu retries=%u\n", (unsigned long)xTaskGetTickCount(),
           name, (unsigned long)m->id, (unsigned)m->retries);
    if (ev == EV_FAILED) s_failedCount++;
}

static void check(int cond, const char *what)
{
    printf("  %s: %s\n", cond ? "PASS" : "FAIL", what);
    if (!cond) s_errors++;
}

static void Test_Task(void *pv)
{
    (void)pv;
    const uint32_t expect_ms[3] = { 10, 20, 40 };
    Message_t m;
    TickType_t t0;
    char buf[64];

    printf("Case 1: Normal message\n");
    memset(&m, 0, sizeof m);
    m.id = 1; m.type = MSG_NORMAL; m.retries = 0;

    for (int i = 0; i < 3; i++) {
        t0 = xTaskGetTickCount();
        Retry_Report(&m);
        if (ulTaskNotifyTake(pdTRUE, pdMS_TO_TICKS(300)) == 0) { check(0, "resend arrived"); break; }
        uint32_t d = (uint32_t)(s_resentTick - t0) * 1000u / configTICK_RATE_HZ;
        snprintf(buf, sizeof buf, "retry %d after %lu ms (expected ~%lu)", i + 1,
                 (unsigned long)d, (unsigned long)expect_ms[i]);
        check(d + TOL_MS >= expect_ms[i] && d <= expect_ms[i] + TOL_MS, buf);
        check(s_resentMsg.retries == i + 1, "retries counter incremented");
        m = s_resentMsg;
    }
    Retry_Report(&m);
    check(ulTaskNotifyTake(pdTRUE, pdMS_TO_TICKS(150)) == 0, "no further resend after 3 retries");
    check(s_failedCount == 1, "logged EV_FAILED exactly once");

    printf("Case 2: Emergency message\n");
    memset(&m, 0, sizeof m);
    m.id = 2; m.type = MSG_EMERGENCY; m.retries = 0;
    t0 = xTaskGetTickCount();
    Retry_Report(&m);
    if (ulTaskNotifyTake(pdTRUE, pdMS_TO_TICKS(100)) == 0) {
        check(0, "Emergency resend arrived");
    } else {
        uint32_t d = (uint32_t)(s_resentTick - t0) * 1000u / configTICK_RATE_HZ;
        snprintf(buf, sizeof buf, "Emergency resent after %lu ms (expected <= 2)", (unsigned long)d);
        check(d <= 2, buf);
    }

    printf("\n%s (%d error%s)\n", s_errors ? "TEST FAILED" : "ALL TESTS PASSED",
           s_errors, s_errors == 1 ? "" : "s");
    exit(s_errors ? 1 : 0);
}

int main(void)
{
    if (Retry_Start() != pdPASS) { printf("Retry_Start failed\n"); return 1; }
    xTaskCreate(Test_Task, "Test", configMINIMAL_STACK_SIZE * 4, NULL, 5, &s_testTask);
    vTaskStartScheduler();
    return 0;
}

void vApplicationMallocFailedHook(void) { configASSERT(0); }
void vApplicationStackOverflowHook(TaskHandle_t t, char *n) { (void)t; (void)n; configASSERT(0); }

/**
 ******************************************************************************
 * @file    freertos.c
 * @brief   FreeRTOS task-based demo (only built with -DUSE_FREERTOS=ON).
 *
 *          Task split:
 *            - "attitude": LCD attitude refresh @ 10 Hz
 *            - "keys":     key scan + actions @ 50 Hz
 *          The demo logic itself lives in demo.c (shared with bare-metal).
 ******************************************************************************
 */

#include "freertos_app.h"
#include "demo.h"

#include "FreeRTOS.h"
#include "task.h"

static void AttitudeTask(void* arg)
{
    (void)arg;

    for (;;)
    {
        Demo_AttitudeUpdate();
        vTaskDelay(pdMS_TO_TICKS(100U));
    }
}

static void KeyTask(void* arg)
{
    (void)arg;

    for (;;)
    {
        Demo_KeyScan();
        Demo_UartEcho();
        vTaskDelay(pdMS_TO_TICKS(20U));
    }
}

void App_FreeRTOS_Init(void)
{
    (void)Demo_Init();

    xTaskCreate(AttitudeTask, "attitude", 512, NULL, 1, NULL);
    xTaskCreate(KeyTask, "keys", 256, NULL, 2, NULL);

    vTaskStartScheduler();

    /* Should never reach here */
    for (;;)
    {
    }
}

void vApplicationMallocFailedHook(void)
{
    for (;;)
    {
    }
}

void vApplicationStackOverflowHook(TaskHandle_t xTask, char* pcTaskName)
{
    (void)xTask;
    (void)pcTaskName;
    for (;;)
    {
    }
}

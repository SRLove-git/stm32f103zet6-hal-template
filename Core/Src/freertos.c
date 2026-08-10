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
#include "cli.h"
#include "demo.h"

#include "FreeRTOS.h"
#include "task.h"

#include <stdio.h>

static TaskHandle_t attitude_handle = NULL;
static TaskHandle_t keys_handle = NULL;

static void Cmd_Stack(int argc, char* argv[])
{
    (void)argc;
    (void)argv;

    printf("stack (words free): attitude=%lu keys=%lu idle=%lu\r\n",
           (unsigned long)uxTaskGetStackHighWaterMark(attitude_handle),
           (unsigned long)uxTaskGetStackHighWaterMark(keys_handle),
           (unsigned long)uxTaskGetStackHighWaterMark(xTaskGetIdleTaskHandle()));
}

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
        Demo_CliPoll();
        vTaskDelay(pdMS_TO_TICKS(20U));
    }
}

void App_FreeRTOS_Init(void)
{
    static const CLI_Cmd_t stack_cmd = {"stack", "show task stack high-water marks", Cmd_Stack};

    (void)CLI_Register(&stack_cmd);

    (void)Demo_Init();

    /* Stack sizes tuned from uxTaskGetStackHighWaterMark: attitude uses
     * ~94 words, keys ~25 words; keep 2x headroom. */
    xTaskCreate(AttitudeTask, "attitude", 256, NULL, 1, &attitude_handle);
    xTaskCreate(KeyTask, "keys", 128, NULL, 2, &keys_handle);

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

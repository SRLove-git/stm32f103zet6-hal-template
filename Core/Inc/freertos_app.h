/**
 ******************************************************************************
 * @file    freertos_app.h
 * @brief   FreeRTOS application entry (tasks + scheduler start).
 *
 * @note    Named freertos_app.h (not freertos.h) to avoid a case-insensitive
 *          filename clash with the kernel's FreeRTOS.h on macOS/Windows.
 ******************************************************************************
 */

#ifndef __FREERTOS_APP_H
#define __FREERTOS_APP_H

#ifdef __cplusplus
extern "C"
{
#endif

    /**
     * @brief Create the application tasks and start the scheduler.
     * @note  Does not return.
     */
    void App_FreeRTOS_Init(void);

#ifdef __cplusplus
}
#endif

#endif /* __FREERTOS_APP_H */

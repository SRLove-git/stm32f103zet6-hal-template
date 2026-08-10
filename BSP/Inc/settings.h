/**
 ******************************************************************************
 * @file    settings.h
 * @brief   Persistent settings stored in the on-board 24C02 EEPROM.
 *
 *          A versioned struct is kept at EEPROM address 0 with a CRC-8
 *          checksum; invalid contents fall back to defaults.
 ******************************************************************************
 */

#ifndef __SETTINGS_H
#define __SETTINGS_H

#ifdef __cplusplus
extern "C"
{
#endif

#include "main.h"

#define SETTINGS_MAGIC 0x5AA5U
#define SETTINGS_VERSION 1U
#define SETTINGS_EEPROM_ADDR 0x00U

    typedef struct
    {
        uint16_t magic;           /* SETTINGS_MAGIC */
        uint8_t version;          /* SETTINGS_VERSION */
        uint16_t motor_pwm_max;   /* example: max PWM duty (0..1000) */
        uint8_t filter_alpha_x10; /* example: low-pass alpha x10 (0..10) */
        uint8_t flags;            /* example: general flags */
        uint8_t reserved[6];      /* future fields */
        uint8_t crc;              /* CRC-8 over the fields above */
    } Settings_t;

    extern Settings_t g_settings;

    /**
     * @brief Load settings from EEPROM; fall back to defaults if invalid.
     */
    HAL_StatusTypeDef SETTINGS_Load(void);

    /**
     * @brief Recompute the CRC and write the settings to EEPROM.
     */
    HAL_StatusTypeDef SETTINGS_Save(void);

    /**
     * @brief Restore defaults and persist them.
     */
    void SETTINGS_Reset(void);

#ifdef __cplusplus
}
#endif

#endif /* __SETTINGS_H */

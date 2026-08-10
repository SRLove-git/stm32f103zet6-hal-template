/**
 ******************************************************************************
 * @file    settings.c
 * @brief   Persistent settings over the 24C02 EEPROM (I2C1).
 ******************************************************************************
 */

#include "settings.h"
#include "crc.h"
#include "eeprom.h"

#include <stddef.h>
#include <string.h>

Settings_t g_settings;

static void SETTINGS_Defaults(void)
{
    memset(&g_settings, 0, sizeof(g_settings));
    g_settings.magic = SETTINGS_MAGIC;
    g_settings.version = SETTINGS_VERSION;
    g_settings.motor_pwm_max = 1000U;
    g_settings.filter_alpha_x10 = 5U; /* alpha = 0.5 */
    g_settings.flags = 0x01U;
}

static void SETTINGS_UpdateCrc(void)
{
    g_settings.crc = CRC8_Compute((const uint8_t*)&g_settings, offsetof(Settings_t, crc));
}

static uint8_t SETTINGS_Valid(void)
{
    return (g_settings.magic == SETTINGS_MAGIC) && (g_settings.version == SETTINGS_VERSION) &&
           (g_settings.crc == CRC8_Compute((const uint8_t*)&g_settings, offsetof(Settings_t, crc)));
}

HAL_StatusTypeDef SETTINGS_Load(void)
{
    EEPROM_Init();

    if ((EEPROM_ReadBuffer(SETTINGS_EEPROM_ADDR, (uint8_t*)&g_settings,
                           (uint16_t)sizeof(g_settings)) != HAL_OK) ||
        (SETTINGS_Valid() == 0U))
    {
        SETTINGS_Defaults();
        return SETTINGS_Save();
    }
    return HAL_OK;
}

HAL_StatusTypeDef SETTINGS_Save(void)
{
    HAL_StatusTypeDef ret;

    EEPROM_Init();
    SETTINGS_UpdateCrc();
    ret = EEPROM_WriteBuffer(SETTINGS_EEPROM_ADDR, (const uint8_t*)&g_settings,
                             (uint16_t)sizeof(g_settings));
    return ret;
}

void SETTINGS_Reset(void)
{
    SETTINGS_Defaults();
    SETTINGS_Save();
}

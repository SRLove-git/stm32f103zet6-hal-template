# Third-Party Notices

This project bundles or derives from the following third-party components.
Each component retains its own copyright and license; the notices below
fulfill the redistribution requirements of those licenses.

## STM32Cube HAL / CMSIS (STM32F1xx)

- Source: STM32Cube_FW_F1 V1.8.7 (STMicroelectronics)
- License: BSD-3-Clause
- Copyright: STMicroelectronics
- Location: `Drivers/`, `Startup/`, `Core/Src/system_stm32f1xx.c`

The `Drivers/` tree and the startup/system files keep their original ST
copyright headers. See `Drivers/STM32F1xx_HAL_Driver/Inc/stm32f1xx_hal_conf.h`
for module configuration.

## Adafruit GFX "classic" 5x7 font

- Source: https://github.com/adafruit/Adafruit-GFX-Library
- License: BSD-3-Clause
- Copyright: Adafruit Industries
- Location: `BSP/Src/lcdfont.c`, `BSP/Inc/lcdfont.h`

## FreeRTOS Kernel

- Source: STM32Cube_FW_F1 V1.8.7, Middlewares/Third_Party/FreeRTOS
- License: MIT (see `Middlewares/FreeRTOS/LICENSE.txt`)
- Copyright: Amazon.com, Inc. or its affiliates / Real Time Engineers Ltd.
- Location: `Middlewares/FreeRTOS/`

## Original template code

- License: MIT (see `LICENSE`)
- Copyright: SRLove-git

The MIT license applies to the original code in this repository, including
`Core/` (except the ST-provided system file above), `BSP/`, `cmake/`,
`Scripts/`, and the build configuration files.

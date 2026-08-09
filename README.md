# STM32F103ZET6 HAL 工程模板（正点原子精英板）

基于 **STM32Cube HAL 库（STM32Cube_FW_F1 V1.8.7）+ CMake** 的规范化工程模板，
面向 **正点原子精英 STM32F103 V2（ATK-DNF103，主控 STM32F103ZET6）**。
以后新项目直接复制本目录，改个工程名即可，无需再手动配置构建系统。

> 板卡硬件信息参考文件 `icoss628586937b6f335e05cbc4679.pdf`（精英 V2 硬件参考手册）。

## 已包含内容

- **HAL/CMSIS 驱动**：`Drivers/`（STM32F1xx_HAL_Driver 全模块 + CMSIS Core + F1 器件头文件）
- **启动文件**：`Startup/startup_stm32f103xe.s`（STM32F103xE 向量表）
- **链接脚本**：`Linker/STM32F103ZETX_FLASH.ld`（512 KB Flash / 64 KB RAM）
- **板级驱动 BSP**：LED（PB5/PE5）、按键（PE4/PE3/PA0）、蜂鸣器（PB8）、
  USART1（PA9/PA10，printf 重定向到串口 115200-8N1）
- **CMake 构建**：`cmake/toolchain-arm-none-eabi.cmake` + 顶层 `CMakeLists.txt`
- **烧录**：OpenOCD + ST-Link（SWD）的 `flash` / `erase` 目标

## 目录结构

```
.
├── CMakeLists.txt                 # 构建入口
├── cmake/
│   └── toolchain-arm-none-eabi.cmake
├── Core/                          # 核心代码（CubeMX 风格）
│   ├── Inc/
│   │   ├── main.h
│   │   ├── stm32f1xx_hal_conf.h   # HAL 模块使能与时钟配置
│   │   └── stm32f1xx_it.h
│   └── Src/
│       ├── main.c                 # main + 系统时钟 + GPIO 初始化
│       ├── stm32f1xx_hal_msp.c    # 外设底层初始化（时钟/引脚/NVIC）
│       ├── stm32f1xx_it.c         # 中断服务函数
│       └── system_stm32f1xx.c
├── BSP/                           # 板级驱动（按外设拆分）
│   ├── Inc/  (led.h key.h beep.h usart.h)
│   └── Src/  (led.c key.c beep.c usart.c)
├── Drivers/
│   ├── CMSIS/
│   └── STM32F1xx_HAL_Driver/
├── Startup/startup_stm32f103xe.s
├── Linker/STM32F103ZETX_FLASH.ld
├── Scripts/                       # 常用辅助脚本
└── build/                         # 构建输出（gitignore）
```

## 环境要求

- [Arm GNU Toolchain](https://developer.arm.com/downloads/-/arm-gnu-toolchain-downloads)
  （`arm-none-eabi-gcc`，或 macOS 下 `brew install arm-none-eabi-gcc`）
- CMake ≥ 3.22（macOS 下 `brew install cmake`）
- Ninja（可选，`brew install ninja`）
- OpenOCD（烧录用，`brew install openocd`）+ ST-Link/DAP 调试器

## 构建

```bash
# 首次配置（Debug 默认；也可 -DCMAKE_BUILD_TYPE=Release）
cmake -B build -G Ninja -DCMAKE_TOOLCHAIN_FILE=cmake/toolchain-arm-none-eabi.cmake

# 编译
cmake --build build
```

产物位于 `build/`：

- `stm32f103zet6-hal-template.elf` / `.hex` / `.bin`
- `stm32f103zet6-hal-template.map`（链接映射）

## 烧录与调试

```bash
# 通过 OpenOCD + ST-Link（SWD）烧录并复位运行
cmake --build build --target flash

# 全片擦除
cmake --build build --target erase
```

如果使用 DAP 仿真器，把 `CMakeLists.txt` 中 `flash`/`erase` 目标的
`-f interface/stlink.cfg` 换成 `-f interface/cmsis-dap.cfg` 即可。

串口调试：USART1（PA9/PA10）通过跳线帽接到板载 CH340C，USB 连接电脑后
用串口工具打开 **115200-8-N-1** 即可看到 printf 输出。

## 如何基于模板新建项目

1. 复制整个目录并重命名，例如 `cp -r stm32f103zet6-hal-template my_project`。
2. 修改 `CMakeLists.txt` 第一行的 `project(...)` 名称为新工程名。
3. 新写的外设驱动放在 `BSP/Inc` 与 `BSP/Src`（或 `Core/Src`），无需改 CMake，
   源文件会自动收集；若用到新的 HAL 模块，在
   `Core/Inc/stm32f1xx_hal_conf.h` 中取消对应 `HAL_xxx_MODULE_ENABLED` 的注释。
4. 在外设初始化中遵循本模板的约定：**外设时钟/引脚/NVIC 放在
   `stm32f1xx_hal_msp.c` 的 `HAL_xxx_MspInit` 中**（与 STM32CubeMX 生成代码一致），
   BSP 只负责业务逻辑。

## 板载外设引脚速查（摘自硬件参考手册）

| 外设 | 引脚 | 说明 |
| --- | --- | --- |
| LED0 / LED1 | PB5 / PE5 | 红 / 绿，低电平点亮 |
| KEY0 / KEY1 | PE4 / PE3 | 低电平有效，内部上拉 |
| KEY_UP | PA0 | 高电平有效，WKUP 唤醒 |
| 蜂鸣器 BEEP | PB8 | 有源蜂鸣器，高电平响 |
| USART1 | PA9 (TX) / PA10 (RX) | P3 跳线接 CH340C，115200 |
| EEPROM 24C02 | PB6 (SCL) / PB7 (SDA) | I2C1，地址 A0 |
| SPI Flash 25Q128 | PB12 (CS) / PB13 (SCK) / PB14 (MISO) / PB15 (MOSI) | SPI2，16 MB |
| NRF24L01 | PG8 (CE) / PG7 (CS) / PG6 (IRQ) | 与 25Q128 共用 SPI2 |
| 红外接收头 | PB9 | LF0038 |
| 光敏传感器 | PF8 | ADC3_IN6 |
| DS18B20 / DHT11 | PG11 | 单总线数据线 |
| RS485 | PA2/PA3（P5 跳线）+ PD7 (RE) | USART2 |
| CAN / USB | PA11 / PA12（P6 跳线选择） | CAN 带 120R 终端电阻 |
| TF 卡 | PC8~PC11 (D0~D3) / PC12 (SCK) / PD2 (CMD) | SDIO 4 位 |
| TFTLCD | FSMC 总线 + PB0 (BL) + PB2/PB1/PF9/PF10/PF11 (触摸) | 片选 PG12 (FSMC_NE4) |
| OLED/摄像头 | PC0~PC7 (D0~D7) 等 | 接口 P4 |
| ATK 模块 | PB10/PB11 (USART3) + PA4 (KEY) + PA15 (LED) | 通用模块接口 |

## 规范说明

- 代码风格：4 空格缩进、HAL/CubeMX 命名习惯（`MX_xxx_Init`、`Error_Handler`）。
- 全局开启 `-Wall -Wextra -Wshadow -Wundef -Wwrite-strings -Wcast-align`，
  `-ffunction-sections -fdata-sections` + `-Wl,--gc-sections` 裁剪未用代码。
- 系统时钟：HSE 8 MHz × PLL9 = 72 MHz，APB1 = 36 MHz，APB2 = 72 MHz，
  Flash 等待周期 2（见 `Core/Src/main.c` 的 `SystemClock_Config`）。

参与贡献前请先阅读 [CONTRIBUTING.md](CONTRIBUTING.md)。

## 许可证

本项目采用 [MIT License](LICENSE) 开源，可自由使用、修改与再分发，详见 LICENSE 文件。

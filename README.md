# STM32F103ZET6 HAL 工程模板（正点原子精英板）

[![CI](https://github.com/SRLove-git/stm32f103zet6-hal-template/actions/workflows/ci.yml/badge.svg)](https://github.com/SRLove-git/stm32f103zet6-hal-template/actions/workflows/ci.yml)
[![License: MIT](https://img.shields.io/badge/License-MIT-yellow.svg)](LICENSE)

基于 **STM32Cube HAL 库（STM32Cube_FW_F1 V1.8.7）+ CMake** 的规范化工程模板，
面向 **正点原子精英 STM32F103 V2（ATK-DNF103，主控 STM32F103ZET6）**。
以后新项目直接复制本目录，改个工程名即可，无需再手动配置构建系统。

> 板卡硬件信息参考文件 `icoss628586937b6f335e05cbc4679.pdf`（精英 V2 硬件参考手册）。

## 已包含内容

- **HAL/CMSIS 驱动**：`Drivers/`（STM32F1xx_HAL_Driver 全模块 + CMSIS Core + F1 器件头文件）
- **启动文件**：`Startup/startup_stm32f103xe.s`（STM32F103xE 向量表）
- **链接脚本**：`Linker/STM32F103ZETX_FLASH.ld`（512 KB Flash / 64 KB RAM）
- **板级驱动 BSP**：LED、按键、蜂鸣器、USART1 printf（115200-8N1）、
  24C02 EEPROM（I2C）、W25Q128 SPI Flash、光敏 ADC、DS18B20 单总线、
  NEC 红外接收、RS485、CAN、SDIO TF 卡、TFTLCD（FSMC，ILI9341 系列）
- **CMake 构建**：`cmake/toolchain-arm-none-eabi.cmake` + 顶层 `CMakeLists.txt`
- **烧录**：OpenOCD + ST-Link（SWD）的 `flash` / `erase` 目标
- **FreeRTOS（可选）**：`-DUSE_FREERTOS=ON` 编译任务版演示，内核见 `Middlewares/FreeRTOS/`
- **算法工具库**：`filter.h`（滑动平均/低通/中值/1D 卡尔曼）、`pid.h`（PID）、`ringbuf.h`（环形缓冲）

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
│   ├── Inc/  (led key beep usart eeprom w25qxx lsens onewire ir_nec rs485 can_bus sd_card lcd lcdfont bsp_dwt)
│   └── Src/  (同名 .c)
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

推荐使用 CMake Presets（配置命令已固化，IDE 也能直接识别）：

```bash
cmake --preset debug      # 首次配置 + 后续重新配置
cmake --build --preset debug
```

Release 同理：`cmake --preset release && cmake --build --preset release`。
产物在 `build/debug/` 或 `build/release/` 下。

也可以手动指定配置：

```bash
# 首次配置（Debug 默认；也可 -DCMAKE_BUILD_TYPE=Release）
cmake -B build -G Ninja -DCMAKE_TOOLCHAIN_FILE=cmake/toolchain-arm-none-eabi.cmake

# 编译
cmake --build build
```

> 默认链接完整 newlib，printf 直接支持浮点（`%f`）。想进一步缩小固件体积，
> 可在配置阶段加 `-DUSE_NEWLIB_NANO=ON`（改用体积更小的 newlib-nano，
> 并自动保留 `%f` 支持，额外占用几 KB Flash）。

产物位于 `build/debug/`（或手动配置时的 `build/`）：

- `stm32f103zet6-hal-template.elf` / `.hex` / `.bin`
- `stm32f103zet6-hal-template.map`（链接映射）

## 烧录与调试

```bash
# 通过 OpenOCD + ST-Link（SWD）烧录并复位运行
cmake --build --preset debug --target flash

# 全片擦除
cmake --build --preset debug --target erase
```

使用 DAP 仿真器时，通过 CMake 缓存变量切换探针（默认 `stlink`）：

```bash
cmake --preset debug -DSTM32_DEBUG_PROBE=cmsis-dap
```

或直接调用烧录脚本：`./Scripts/flash.sh cmsis-dap`。

串口调试：USART1（PA9/PA10）通过跳线帽接到板载 CH340C，USB 连接电脑后
用串口工具打开 **115200-8-N-1** 即可看到 printf 输出。

### VS Code 调试

1. 安装 [Cortex-Debug](https://marketplace.visualstudio.com/items?itemName=marus25.cortex-debug)
   扩展（打开工程时 VS Code 会按 `extensions.json` 提示安装推荐扩展）。
2. 连接 ST-Link（SWD）并确认 OpenOCD 已在 PATH 中。
3. 打开 `main.c`，按 `F5` 启动调试：会自动编译 Debug 固件 → 启动 OpenOCD →
   进入 GDB 会话并停在 `main` 入口。

常用任务（`Ctrl/Cmd+Shift+P` → “Tasks: Run Task”）：
Build (Debug)、Clean + Build (Debug)、Flash (OpenOCD/ST-Link)。

## 如何基于模板新建项目

0. 推荐直接使用一键脚本（自动复制并改名）：

   ```bash
   ./Scripts/new_project.sh my_project
   ```

   默认在模板同级目录生成 `my_project`，也可指定路径：
   `./Scripts/new_project.sh my_project /path/to/my_project`。

1. 或手动复制整个目录并重命名，例如 `cp -r stm32f103zet6-hal-template my_project`，
   然后修改 `CMakeLists.txt` 第一行的 `project(...)` 名称为新工程名。
2. 新写的外设驱动放在 `BSP/Inc` 与 `BSP/Src`（或 `Core/Src`），无需改 CMake，
   源文件会自动收集；若用到新的 HAL 模块，在
   `Core/Inc/stm32f1xx_hal_conf.h` 中取消对应 `HAL_xxx_MODULE_ENABLED` 的注释。
3. 在外设初始化中遵循本模板的约定：**外设时钟/引脚/NVIC 放在
   `stm32f1xx_hal_msp.c` 的 `HAL_xxx_MspInit` 中**（与 STM32CubeMX 生成代码一致），
   BSP 只负责业务逻辑。

## 板载外设驱动

所有 BSP 模块都已接入正确的板载引脚与跳线配置，按需调用对应 `xxx_Init()` 即可：

| 模块 | 头文件 | 板载资源 | 用法示例 |
| --- | --- | --- | --- |
| EEPROM | `eeprom.h` | 24C02 @ I2C1 (PB6/PB7) | `EEPROM_WriteByte(0, v)` / `EEPROM_ReadByte(0)` |
| SPI Flash | `w25qxx.h` | W25Q128 @ SPI2 (PB12~15) | `W25QXX_ReadID()`；写前先 `W25QXX_EraseSector()` |
| 光敏 | `lsens.h` | ADC3_IN6 (PF8) | `LSENS_ReadADC()` 返回 0~4095 |
| 温度 | `onewire.h` | DS18B20 @ PG11 | `DS18B20_GetTemp()` 返回 0.01°C |
| 红外 | `ir_nec.h` | LF0038 @ PB9 | 周期调用 `IR_GetKey()` 获取 NEC 码 |
| RS485 | `rs485.h` | USART2 (PA2/PA3) + PD7 | `RS485_SendData()` / `RS485_ReceiveData()` |
| CAN | `can_bus.h` | CAN1 (PA11/PA12) 500 kbit/s | `CAN1_SendMsg()` / `CAN1_ReceiveMsg()` |
| TF 卡 | `sd_card.h` | SDIO 4-bit (PC8~12/PD2) | `SD_ReadBlocks()` / `SD_WriteBlocks()`，512 B/块 |
| TFTLCD | `lcd.h` | FSMC NE4/A10，16 位数据线，PB0 背光 | `LCD_Init()` 后 `LCD_Clear()` / `LCD_ShowString()` 等 |
| MPU6050 | `mpu6050.h` + `attitude.h` | ATK 模块接口（实测 SCL=PB11 / SDA=PB10，软件 I2C）| `MPU6050_Init()` 后读原始数据，或 `MPU6050_GetAttitude()` 输出姿态角（Mahony/Madgwick 可选）|
| OLED | `oled.h` | P4 接口，8080 并口（DC=PD3 / CS=PD6 / WR=PG14 / RST=PG15 / D0~7=PC0~7）| `OLED_Init()` 后 `OLED_ShowString()` + `OLED_Refresh()` |
| NRF24L01 | `nrf24l01.h` | WIRELESS 接口（CE=PG8 / CS=PG7 / IRQ=PG6，SPI2 共用）| `NRF24L01_Init()` 后 `NRF24L01_TxPacket()` / `RxPacket()` |

使用前注意跳线帽：RS485 需短接 P5，CAN 需把 P6 拨到 CAN 档，
NRF24L01 与 SPI Flash 共用 SPI2（片选互斥）。微秒级时序（单总线/红外）
由 `bsp_dwt.h` 的 DWT 延时提供，无需额外外设。

TFTLCD 说明：预配置为 2.8 寸 **ILI9341 系列**（`LCD_GetID()` 应返回 `0x9341`），
命令/数据通过 FSMC Bank1 NE4 + A10 映射（`0x6C000000 | 0x7FE`），内置 5x7
ASCII 字体（来自 Adafruit GFX，BSD 许可），支持缩放显示与横竖屏切换
（`LCD_SetDirection()`）。3.5/4.3/7 寸屏需在 `LCD_InitSequence()` 中补充对应
控制器的初始化序列。

OLED 说明：ATK-0.96 寸 SSD1306 模块以 **8080 并口模式**（模块默认 BS1/BS2 焊盘）
靠左插入 P4 接口；与摄像头共用数据线，二者不同时使用。P4 的摄像头信号
（OV_WEN=PB3、OV_RCLK=PB4、OV_VSYNC=PA8 及 D0~7）留给 OV7670/OV7725
摄像头模块，摄像头驱动未包含在本模板中。

## 开机自检（BSP Self-Test）

上电后 `BSP_SelfTest()`（`Core/Src/main.c` 中调用）会依次测试板载外设，
并把结果打印到 USART1（115200-8N1）：

| 模块 | 检查内容 | 预期结果 |
| --- | --- | --- |
| EEPROM | 0xF0 地址写读回 | PASS（测试后自动恢复原值）|
| SPI Flash | JEDEC ID + 末扇区写读回 | ID = 0xEF4018，PASS |
| 光敏 | ADC 转换 | raw 0~4095 |
| DS18B20 | 读温度 | 未接传感器显示 SKIP |
| 红外 | 接收初始化 | INFO |
| RS485 | 串口初始化 | INFO（需 P5 跳线 + 对端节点）|
| CAN | 内部回环收发 | PASS |
| TF 卡 | 初始化 + 读块 0 | 无卡显示 SKIP |
| TFTLCD | 读控制器 ID + 像素读回 | ID 为 0x93xx（ILI9341 系列，面板差异，仅供参考）|
| MPU6050 | WHO_AM_I（0x68/0x69/0x70）+ 加速度 + Mahony 姿态角 | 未插模块显示 SKIP |
| OLED | 初始化和写入 | INFO（并口无回读，屏幕上可见文字即正常）|
| NRF24L01 | SPI 寄存器读写回验 | 未插模块显示 SKIP |

自检结束后进入正常的 LED/按键/蜂鸣器演示。不需要自检时，删除
`main()` 里的 `BSP_SelfTest()` 调用即可。

若自检检测到 MPU6050，主循环会把 **roll/pitch/yaw 姿态角实时显示在 TFTLCD 上**
（每 100 ms 刷新，Mahony 默认，按 KEY_UP 可在 Mahony/Madgwick 间切换）；
未接 MPU 时维持 LED 闪烁演示。

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
- 已知工具链坑：arm-gnu-toolchain 15.2 自带 newlib 的 `%f` 打印对负数/NaN 有缺陷
  （可能输出 `0.0`、乱码甚至 HardFault），浮点输出请用整数格式化
  （参考 `bsp_selftest.c` 的 `PrintDeg()`）。

## FreeRTOS（可选）

模板同时支持裸机和 FreeRTOS 两种构建，其余代码（BSP、自检、CMake、CI）完全一致：

```bash
cmake --preset freertos && cmake --build --preset freertos
```

或手动加 `-DUSE_FREERTOS=ON`。任务版演示（`Core/Src/freertos.c`）：

- `attitude` 任务：每 100 ms 刷新 LCD 姿态角（Mahony/Madgwick，KEY_UP 切换）
- `keys` 任务：每 20 ms 扫描按键（蜂鸣器 / LED / 滤波器切换）

说明：

- FreeRTOS 内核来自 STM32Cube_FW_F1（V10.x，MIT 许可），堆为 heap_4（10 KB）
- SysTick 由 HAL tick 与 FreeRTOS tick 共享（1 kHz），见 `stm32f1xx_it.c`
- 任务间 printf 未加互斥，仅用于演示；多任务打印请自行加锁

## 算法工具库

纯数学模块，不依赖硬件，可直接用于传感器数据处理：

| 模块 | 内容 | 用法 |
| --- | --- | --- |
| `filter.h` | 滑动平均（8 点）、一阶低通、5 点中值、1D 卡尔曼 | 各算法用独立结构体实例，`Init` 后循环 `Update` |
| `pid.h` | 位置式 PID，带积分限幅/输出限幅、微分作用于测量值 | `PID_Init` + 循环 `PID_Update` |
| `ringbuf.h` | 无锁单生产者/单消费者环形缓冲（容量须为 2 的幂）| 中断写入、任务读取，`Put`/`Get`/`PutBlock`/`GetBlock` |

USART1 已启用**中断接收**：收到的字节进入 64 字节环形缓冲，演示程序会原样回显
（`USART1_RxCount` / `USART1_RxRead` 读取）。

参与贡献前请先阅读 [CONTRIBUTING.md](CONTRIBUTING.md)。

## 许可证

本项目采用 [MIT License](LICENSE) 开源，可自由使用、修改与再分发，详见 LICENSE 文件。
其中包含的 ST HAL/CMSIS 与 Adafruit 字体等第三方组件遵守各自的
BSD-3-Clause 许可，完整声明见 [THIRD_PARTY_NOTICES.md](THIRD_PARTY_NOTICES.md)。

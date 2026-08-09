# CONTRIBUTING

感谢你愿意为 `stm32f103zet6-hal-template` 贡献代码。本仓库是面向
正点原子精英板（STM32F103ZET6）的 HAL 工程模板，所有改动都应保持
“拿来即用、规范统一”的特点。请先阅读 [README.md](README.md) 了解工程结构。

## 贡献方式

- **报告问题**：使用 GitHub Issue，说明复现步骤、期望行为与实际行为，
  附上相关日志或串口输出。
- **功能建议**：说明用途、适用场景和预期接口，讨论通过后再动手。
- **提交代码**：遵循下面的分支流程与代码规范。
- **完善文档**：README / CONTRIBUTING / 注释，同样欢迎。

## 分支与提交流程

1. 从 `main` 拉取最新代码，新建分支：

   ```bash
   git checkout main
   git pull
   git checkout -b feat/add-xxx-driver
   ```

   分支命名：`feat/`、`fix/`、`docs/`、`refactor/`、`chore/` 前缀。
2. 完成开发并自测（见“构建与验证”）。
3. 提交信息遵循 [Conventional Commits](https://www.conventionalcommits.org/)：

   ```
   feat(bsp): add W25Q128 SPI flash driver
   fix(core): correct PCLK1 divider in SystemClock_Config
   docs: update pin mapping table in README
   ```

   标题用英文或中文均可，但一个提交只做一件事。
4. 推送到自己的分支并发起 Pull Request，在描述中说明改动内容、
   验证结果（编译 0 警告、烧录后现象）。

## 构建与验证

所有代码必须通过完整构建，**0 警告**是硬性要求：

```bash
cmake -B build -G Ninja -DCMAKE_TOOLCHAIN_FILE=cmake/toolchain-arm-none-eabi.cmake
cmake --build build --clean-first
```

建议同时验证 Debug 与 Release：

```bash
cmake --build build --config Debug    # 若用多配置生成器
cmake -B build-release -G Ninja -DCMAKE_BUILD_TYPE=Release \
      -DCMAKE_TOOLCHAIN_FILE=cmake/toolchain-arm-none-eabi.cmake
```

涉及板级行为（LED/按键/串口等）的改动，请在有硬件的情况下实际烧录验证
（`cmake --build build --target flash`），并在 PR 中说明。

## 代码规范

与现有模板保持一致，不要引入第二种风格：

- 语言标准：C11（gnu11）；缩进 4 空格；大括号独占一行（Allman 风格）。
- 命名：
  - 文件：小写 + 下划线（`led.c`、`stm32f1xx_it.c`）；
  - 函数：`模块前缀 + 大驼峰`（`LED0_On`、`KEY_Scan`、`MX_USART1_UART_Init`）；
  - 宏与常量：全大写 + 下划线（`LED0_PORT`、`KEY0_PRESS`）；
  - 私有函数/变量：`static`，必要时加 `p` 前缀避免与公共符号冲突。
- 文件头：沿用现有 Doxygen 注释模板（`@file`、`@brief`，注明板级引脚）。
- 头文件：必须有 include guard（`__XXX_H`），禁止在头文件里定义变量。
- 外设初始化约定（与 STM32CubeMX 一致）：
  - **外设时钟、引脚、NVIC 配置放在 `Core/Src/stm32f1xx_hal_msp.c` 的
    `HAL_xxx_MspInit` 中**；
  - 业务逻辑放在对应 BSP 模块（`BSP/Inc` + `BSP/Src`）；
  - 同一个引脚只能初始化一次，禁止在多个文件里重复配置。
- 新增外设模块：在 `Core/Inc/stm32f1xx_hal_conf.h` 取消对应
  `HAL_xxx_MODULE_ENABLED` 注释；CMake 会自动收集 `Core/Src` 与
  `BSP/Src` 下的源文件，**不需要**修改 `CMakeLists.txt` 的源文件列表。
- 不要提交 `build/` 目录及任何编译产物（已在 `.gitignore` 中）。

## 板级硬件注意事项

精英板部分引脚与板上外设或调试接口冲突，新增引脚配置前先查
[README.md](README.md) 的引脚速查表，重点注意：

- `PB3` / `PB4` / `PA15` 默认被 JTAG 占用，复用前需先禁用 JTAG（保留 SWD）；
- `PB8` 接有源蜂鸣器，作为普通 IO 输出高电平会响；
- `PA11` / `PA12` 在 CAN 与 USB 之间由跳线 `P6` 选择；
- `PA2` / `PA3` 接 RS485 收发器，是否连到 MCU 由跳线 `P5` 决定；
- `PB13~PB15`（SPI2）被板载 25Q128 与 NRF24L01 接口共用，需注意片选互斥；
- 本模板默认 HSE = 8 MHz，若你的板子晶振不同，需同步修改
  `Core/Src/main.c` 的 `SystemClock_Config` 与
  `Core/Inc/stm32f1xx_hal_conf.h` 中的 `HSE_VALUE`。

## PR 检查清单

- [ ] `cmake --build build --clean-first` 通过且 0 警告（Debug + Release）
- [ ] 有硬件时已实际烧录验证，PR 中描述现象
- [ ] 只包含本次改动相关文件，无构建产物、无 IDE 配置残留
- [ ] 头文件有 include guard，注释完整
- [ ] 如引脚/外设分配有变化，README 的引脚速查表已同步更新
- [ ] 提交信息遵循 Conventional Commits

有任何疑问，先开 Issue 讨论再动手，避免大范围返工。

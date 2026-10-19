# 02_exti

> **课程**：江协科技《STM32入门教程-2023版》[4-1] 对射式红外传感器计次（P11）、[4-2] 旋转编码器计次（P12）
> **工程**：`EXTI-Project/`（Keil MDK5 + STM32F10x 标准外设库 V3.5，芯片 STM32F103C8T6）
> **来源**：由本地练习工程 `E:\ALL-PRACT\LED-PROJECT - 对射式红外传感器计次及旋转编码器计次` 整理迁移而来（2026-10-16 练习）

## 状态

| 项 | 状态 |
|---|---|
| 代码 | ✅ 完成（COUNTSENSOR + Encoder + OLED 显示） |
| 编译 | ✅ **0 Error / 0 Warning**（Keil MDK 5.39 + ARMCC 5.06，Code 2932 B） |
| 上板实测 | ⏳ **待重新烧录确认**（原练习工程已跑过；迁移到本仓库后需再下载一次，把现象补进「预期结果」） |

## 目的

一次把 **两个 EXTI 应用**做完，覆盖「单路外部脉冲计数」和「双路相位差判断方向」两类问题：

1. **[4-1] 对射式红外传感器计次**：物体每遮挡一次光路 → 传感器 DO 输出一个边沿 → EXTI14 中断 → 计数 +1 → OLED 显示。
2. **[4-2] 旋转编码器计次**：编码器 A/B 两相各接一个 EXTI 线，靠**两相谁先出现下降沿**判断旋转方向，实现增量计数 → 累加到变量 → OLED 显示。
3. 掌握 EXTI 的通用链路（`GPIO → AFIO 线映射 → EXTI 边沿 → NVIC → ISR`）以及三个新概念：
   - **触发边沿的选择**（上升沿/下降沿取决于模块输出的电平逻辑）；
   - **多条 EXTI 线共用一个中断号**（EXTI10～15 共用 `EXTI15_10_IRQn`）；
   - **在中断里读另一相电平**判断方向（不靠软件延时）。

## 接线

| 模块 | 模块引脚 | MCU 引脚 | GPIO 配置 | 中断/说明 |
|---|---|---|---|---|
| 对射式红外传感器 | VCC / GND | 3.3V / GND | — | 江协套件模块 3 脚（VCC/GND/DO） |
| 对射式红外传感器 | DO | **PB14** | `GPIO_Mode_IPU` 上拉输入 | EXTI14，**上升沿**触发，遮挡一次计数 +1 |
| 旋转编码器 | VCC / GND | 3.3V / GND | — | 模块 4～5 脚（VCC/GND/A/B，可能带按键脚） |
| 旋转编码器 | A 相 | **PB0** | `GPIO_Mode_IPU` 上拉输入 | EXTI0，**下降沿**触发 |
| 旋转编码器 | B 相 | **PB1** | `GPIO_Mode_IPU` 上拉输入 | EXTI1，**下降沿**触发 |
| OLED（0.96 寸 I²C） | SCL | **PB6** | `GPIO_Mode_Out_OD` 开漏输出 | 软件 I²C |
| OLED | SDA | **PB7** | `GPIO_Mode_Out_OD` 开漏输出 | 软件 I²C |

> 传感器触发边沿说明：本工程代码用的是 `EXTI_Trigger_Rising`。这取决于模块 DO 的电平逻辑
> （有的模块遮挡时输出高、有的输出低）。**实测若发现「遮挡时不变、移开时 +1」，就是把边沿选反了** → 改成 `EXTI_Trigger_Falling`。
>
> 编码器 A/B 相与旋转方向的对应关系：如果顺时针变成减、逆时针变成加，**不用改接线**，把两个 ISR 里的 `++` / `--` 对调即可。

## 关键配置

### 三套 EXTI，一张表看完

| 中断源 | EXTI 线 | 触发边沿 | NVIC 中断号 | 抢占/子优先级 | 中断服务函数 | 所在文件 | 干什么 |
|---|---|---|---|---|---|---|---|
| 传感器 DO（PB14） | EXTI14 | **Rising** | `EXTI15_10_IRQn` | 1 / 1 | `EXTI15_10_IRQHandler` | `Hardware/COUNTSENSOR.c` | `CountSensor_Count++` |
| 编码器 A 相（PB0） | EXTI0 | **Falling** | `EXTI0_IRQn` | 1 / 1 | `EXTI0_IRQHandler` | `Hardware/Encoder.c` | 读 B 相为 0 → `Encoder_Count--` |
| 编码器 B 相（PB1） | EXTI1 | **Falling** | `EXTI1_IRQn` | 1 / 2 | `EXTI1_IRQHandler` | `Hardware/Encoder.c` | 读 A 相为 0 → `Encoder_Count++` |

### 初始化流程（每个模块都走的五步）

```text
① 开时钟：RCC_APB2Periph_GPIOB + RCC_APB2Periph_AFIO   ← 漏 AFIO 中断永远不来
② 配 GPIO：PB14 / PB0 / PB1 都设成 GPIO_Mode_IPU（上拉输入，平时高、动作变低/高）
③ 线映射：GPIO_EXTILineConfig(GPIO_PortSourceGPIOB, GPIO_PinSourceX)
④ 配 EXTI：EXTI_Mode_Interrupt + EXTI_Trigger_xxx + EXTI_LineCmd=ENABLE
⑤ 配 NVIC：NVIC_IRQChannel + 抢占/子优先级 + ENABLE
```

NVIC 分组：`NVIC_PriorityGroup_2`（2 位抢占 0～3 + 2 位子优先级 0～3）。

> ⚠️ 本工程里 `COUNTSENSOR_Init()` 和 `Encoder_Init()` **各调用了一次** `NVIC_PriorityGroupConfig`。
> 两次参数相同，所以没出问题，但规范做法是：**整个工程只在 `main()` 开头调用一次**（后执行的配置会覆盖先执行的）。

### 方向判断原理（编码器为什么能知道正反转）

```text
    A 相 ──┐  ┌────  ┌────     A 相由高变低（下降沿）时，读 B 相电平
           └──┘     ┘
    B 相 ────┐  ┌────  ┌──     B=0 → 说明是 A 领先 → 一个方向的相位
             └──┘     ┘        B=1 → 相反方向

实现：EXTI0 中断里读 PB1；EXTI1 中断里读 PB0。
      A、B 各配一个中断 → 两个方向都能计数（江协 [4-2] 的做法）。
```

中断里那层额外判断 `if (GPIO_ReadInputDataBit(..., 0) == 0)` 是江协注释里说的「防止数据乱跳」——
下降沿触发后引脚可能已经弹回高电平，再读一次可以过滤掉抖动引起的误计数。

### 数据流

```text
遮挡光路 / 转动编码器
  └─> 硬件边沿 → EXTI 挂起 → NVIC → ISR
        └─> 清标志位（必须）→ 计数变量 ++ / --
主循环 while(1)
  └─> CountSensor_Get() → OLED 第 1 行显示
  └─> Num += Encoder_Get() → OLED 第 2 行显示
```

## 测试步骤

1. **编译**：Keil 打开 `EXTI-Project/project.uvprojx`，F7 编译（预期 `0 Error(s), 0 Warning(s)`）。
2. **下载**：BOOT0 = 1 → 复位 → FlyMcu 串口下载 → **BOOT0 跳回 0** → 复位（或 ST-Link 直接下载）。
3. **上电**：OLED 显示
   ```text
   Sensor:    0
   Num:       0
   ```
4. **传感器测试**：用手在光路中遮挡一次 → `Sensor` 数字 +1。
   - 预期：**遮挡一次只加 1**；连续遮挡 10 次 = 10。
   - 若只在「移开」的那一下 +1 → 触发边沿选反，改 `EXTI_Trigger_Falling`。
   - 若一次遮挡加了 2～3 → 抖动/误触发，检查模块供电与线长（江协注释里那条二次判电平的 guard）。
5. **编码器测试**：顺时针转一格 → `Num` 加 1；逆时针 → `Num` 减 1（方向定义取决于你的编码器相序，反了就对调 ISR 里的 `++`/`--`）。
6. **交叉验证**：遮挡传感器时 `Num` 不动；转编码器时 `Sensor` 不动。
7. **按复位键**：两个计数都从 0 开始（全局变量在 RAM 里，掉电/复位即清零，没有保存）。
8. 屏幕显示闪烁/乱码：检查 OLED 的 PB6/PB7 是否接反、是否有上拉电阻。

## 预期结果

| 操作 | 预期现象 |
|---|---|
| 上电 | `Sensor: 0`、`Num: 0` |
| 遮挡光路 1 次 | `Sensor` +1，且只 +1 |
| 遮挡 10 次 | `Sensor: 10` |
| 编码器顺时针 1 格 | `Num` +1（一格一档，与编码器型号有关） |
| 编码器逆时针 1 格 | `Num` −1 |
| 编码器转一整圈 | `Num` 变化约 ±20（EC11 常见 20 档/圈，实测确认） |
| 两路同时操作 | 互不干扰 |

> ⏳ 待你重新烧录后，把上面这些实测结果（尤其是「遮挡一次是否只 +1」「一圈对应多少格」「方向是否反」）写实，再把本行删掉。

## 踩坑记录

### A. 整理/迁移阶段（本次已解决）

| # | 现象 | 原因 | 解决 |
|---|---|---|---|
| 1 | 编译警告 `#1-D: last line of file ends without a newline` | `LED.c`、`COUNTSENSOR.c` 末尾缺换行 | 文件末尾补一个换行 |
| 2 | 换电脑后 Keil 断点错位 | `project.uvoptx` 里存了原来 `E:\ALL-PRACT\...` 的绝对路径 | 改成相对路径 `.\Hardware\COUNTSENSOR.c` |
| 3 | 头文件声明与定义不一致（潜在） | `COUNTSENSOR.h` 写 `int CountSensor_Get(void)`，`.c` 里返回 `uint16_t` | 统一改 `uint16_t`（C 语言要求声明和定义类型一致） |
| 4 | 掉电/复位后计数清零 | `CountSensor_Count` 是 RAM 全局变量 | 这是正常行为；需要掉电保存得用 Flash/备份寄存器，现阶段不用管 |

### B. 上板常见坑（按课程整理，实测后把亲身遇到的改写成第一人称）

| # | 现象 | 原因 | 解决 |
|---|---|---|---|
| 5 | 中断一次都不进 | 中断函数名写错（例如写成 `EXTI14_IRQHandler`） | EXTI10～15 只能叫 `EXTI15_10_IRQHandler`，且线号在 `if` 里用 `EXTI_GetITStatus(EXTI_Line14)` 判断 |
| 6 | 完全不触发，且无报错 | 漏开 `RCC_APB2Periph_AFIO` 时钟 | 第 ① 步 GPIOB 和 AFIO 一起开 |
| 7 | 中断只进一次 / 程序卡死 | 忘记 `EXTI_ClearITPendingBit()` | 中断标志必须手动清 |
| 8 | 计数乱跳、一次动作多次 +1 | 机械/光路抖动产生多个边沿 | 中断里二次判电平（本工程已有）；必要时再加时间窗口 |
| 9 | 传感器「遮挡不变、移开才 +1」 | 触发边沿选反（模块 DO 逻辑是遮挡时输出高） | 把 `EXTI_Trigger_Rising` ↔ `EXTI_Trigger_Falling` 互换 |
| 10 | 编码器正反转与显示方向相反 | A/B 相序或接线顺序不同 | 不需要改线，对调两个 ISR 里的 `++`/`--` |
| 11 | 编译过但链接报错 | 新增的 `Hardware/*.c` 没加入 Keil Group | Project 窗口对应 Group 右键 → `Add Existing Files`；Include Paths 加 `.\Hardware` |
| 12 | 下载后程序不跑 | 串口下载后 BOOT0 忘了跳回 0 | BOOT0 = 0 再复位 |

### C. 代码可改进项（不阻塞功能）

1. `COUNTSENSOR.c` 里 `EXTI_ClearITPendingBit()` 写在 `if` **外面**：能工作，但规范上放进 `if` 里，只清自己那条线。
2. `NVIC_PriorityGroupConfig` 在两个模块初始化里重复调用（见上），应移到 `main()`。
3. `Hardware/` 下还留着 01_gpio 时期的 `KEY.c` / `LED.c`，当前 `main()` 没用到，保留作为参考；正式项目应移除未使用模块。
4. 两个计数变量没有加 `volatile`：因为只在主循环「读」、中断里「写」，编译器优化等级高时理论上存在缓存问题——养成习惯：**中断和主循环共享的变量一律加 `volatile`**。

## 与 01_gpio 的对比

| | `01_gpio`（轮询） | `02_exti`（中断） |
|---|---|---|
| 读输入方式 | 主循环 `while` 查引脚 + `Delay_ms` 阻塞 | 硬件检测边沿，触发中断 |
| CPU 占用 | 一直查，不能干别的 | 平时空转，事件来了才进中断 |
| 抖动处理 | 阻塞式延时（阻塞整个程序） | ISR 只计数/置标志 + 必要时二次判电平 |
| 适用场景 | 简单、对实时性无要求 | 编码器/传感器这类**随机、高频、必须立即响应**的输入 |

## 待办 / 下一步

- [ ] **重新烧录确认**，把「预期结果」里的 ⏳ 换成实测数据（遮挡 1 次 / 10 次、编码器一圈格数、方向是否正确）
- [ ] 复核传感器触发边沿（Rising 是否适合你的模块）
- [ ] 录 30 秒演示视频（遮挡计次 + 编码器正反转）
- [ ] 复盘：能否不看代码，说清「三套 EXTI 各用哪条线、哪个中断号、哪个函数名」
- [ ] 下一步 `03_timer`：用定时器 1 ms 中断取代 `Delay_ms`，为 PWM/控制周期打基础

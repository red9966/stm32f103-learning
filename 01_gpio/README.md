# 01_gpio

## 目的
按键控制 LED 的亮灭：按下一个按键，对应的 LED 状态翻转。

## 接线

| MCU 引脚 | 连接对象 | 说明 |
|---|---|---|
| PA0 | LED1 | GPIO 推挽输出（`GPIO_Mode_Out_PP`） |
| PA3 | LED2 | GPIO 推挽输出（`GPIO_Mode_Out_PP`） |
| PB0 | 按键1 | GPIO 上拉输入（`GPIO_Mode_IPU`），按下为低电平 |
| PB11 | 按键2 | GPIO 上拉输入（`GPIO_Mode_IPU`），按下为低电平 |

对应关系：**PB0 → PA3 的 LED**，**PB11 → PA0 的 LED**。

## 关键配置

1. **开时钟**：LED 在 GPIOA、按键在 GPIOB，两者都在 APB2 总线上
   `RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA | RCC_APB2Periph_GPIOB, ENABLE)`
2. **LED**：配置为推挽输出 `GPIO_Mode_Out_PP`，速度 `GPIO_Speed_50MHz`
3. **按键**：配置为**上拉输入** `GPIO_Mode_IPU`（注意是输入，不是输出），按下读到低电平
4. **封装**：`LED_Init()` / `Key_Init()` 各自封装成函数，`main.c` 只负责调用，保持主程序简洁

## 测试步骤

1. 编译下载（BOOT0 = 0，ST-Link 或串口下载）
2. 上电后观察两颗 LED 的初始状态：待补
3. 按下 PB0，观察 PA3 上的 LED 是否翻转
4. 按下 PB11，观察 PA0 上的 LED 是否翻转

## 预期结果

- PB0 控制 PA3 的 LED
- PB11 控制 PA0 的 LED

## 踩坑记录

- **现象**：无法烧录
- **原因**：新建的 `Hardware` 文件夹没有加入 Keil 的 Include Paths，编译能过但下载/链接报错
- **解决**：`Options for Target → C/C++ → Include Paths` 中加入 `..\Hardware`

## 待办 / 下一步

- [ ] 补上 LED 与按键的具体接线（限流电阻、上拉电阻、共阳/共阴、有效电平）
- [ ] 补上"上电后初始状态"的实际观察结果
- [ ] 下一步：按键**消抖**（现在是直接读取，会有抖动导致一次按下跳多次）
- [ ] 下一步：用**状态机**重写按键逻辑，不用 `delay`
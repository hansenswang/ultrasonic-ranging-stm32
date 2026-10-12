# Temperature-Compensated Ultrasonic Rangefinder
# 温湿度补偿型超声波测距仪

> A short-range ultrasonic distance meter built on an **STM32F103C8T6 (ARM Cortex-M3)** that
> measures the ambient temperature and humidity and **corrects the speed of sound in real
> time**, instead of assuming a fixed 340 m/s. The compensated range is shown alongside the
> uncompensated one, so the correction is visible rather than asserted.
>
> 基于 **STM32F103C8T6（ARM Cortex-M3）** 的近距离超声波测距系统。系统实时测量环境温湿度并
> 据此**动态修正声速**，而非沿用固定的 340 m/s；同时显示补偿值与未补偿值，使修正效果可被
> 直接观察，而非仅作宣称。

<p align="left">
  <img alt="MCU" src="https://img.shields.io/badge/MCU-STM32F103C8T6-1f6feb">
  <img alt="Core" src="https://img.shields.io/badge/Core-ARM%20Cortex--M3-1f6feb">
  <img alt="Clock" src="https://img.shields.io/badge/Clock-72%20MHz-1f6feb">
  <img alt="Sensor" src="https://img.shields.io/badge/Sensor-HC--SR04%20%2B%20DHT11-0a7f3f">
  <img alt="Display" src="https://img.shields.io/badge/Display-SSD1306%20%2B%20TM1637-f0a020">
  <img alt="Toolchain" src="https://img.shields.io/badge/Toolchain-Keil%20MDK5-8957e5">
  <img alt="License" src="https://img.shields.io/badge/License-MIT-lightgrey">
</p>

---

## Table of Contents / 目录

- [Why this project / 项目缘起](#why-this-project--项目缘起)
- [Highlights / 技术亮点](#highlights--技术亮点)
- [The physics: why 340 m/s is not good enough / 物理原理](#the-physics-why-340-ms-is-not-good-enough--物理原理)
- [System architecture / 系统架构](#system-architecture--系统架构)
- [Bill of materials / 主要器件](#bill-of-materials--主要器件)
- [Pin assignment / 引脚分配](#pin-assignment--引脚分配)
- [How the measurement works / 测量原理实现](#how-the-measurement-works--测量原理实现)
- [The compensation model / 补偿模型](#the-compensation-model--补偿模型)
- [A calibration problem worth recording / 一次值得记录的标定问题](#a-calibration-problem-worth-recording--一次值得记录的标定问题)
- [Measured performance / 实测性能](#measured-performance--实测性能)
- [Build and flash / 编译与烧录](#build-and-flash--编译与烧录)
- [Figures / 图片索引](#figures--图片索引)
- [Repository layout / 仓库结构](#repository-layout--仓库结构)
- [Known limitations / 已知局限](#known-limitations--已知局限)
- [Author / 作者](#author--作者)

---

## Why this project / 项目缘起

The textbook ultrasonic rangefinder is four lines of arithmetic: pulse, time the echo, multiply
by 340, halve it. It works — until the room is warm, or humid, or both.

I wanted to know **how much** of the residual error was environmental rather than electrical.
That question forces you to do something the textbook version skips: measure the environment,
model its effect on the medium, and then show both answers side by side so the difference is
not a claim but a reading. That is what this build does.

> 教科书式的超声波测距只需四步运算：发脉冲、测回波时间、乘 340、除以二。它确实能工作——
> 直到房间变热、变潮，或两者兼有。
>
> 我想弄清楚：残余误差里究竟有多少来自**环境**而非电路。这个问题迫使你去做教科书版本跳过
> 的事情：测量环境、对介质的影响建模，并把两个结果并排显示——于是差异不再是一个说法，而是
> 一次实际读数。

---

## Highlights / 技术亮点

| # | Item / 项目 | Detail / 说明 |
|---|---|---|
| 1 | **Real-time speed-of-sound compensation / 实时声速补偿** | `v = 331.4 + 0.606·T + 0.0124·H` is evaluated every measurement cycle from live DHT11 data, not from a constant. At 35 °C / 80 % RH this shifts the assumed speed from 340 m/s to ≈355 m/s — a **4.4 % range error** that the compensated path removes.<br>每次测量都基于 DHT11 的实时数据计算 `v`，而非使用常数。在 35 °C / 80 % RH 下，假定声速会从 340 m/s 变为约 355 m/s——对应 **4.4 % 的测距误差**，补偿路径可消除该误差。 |
| 2 | **Both results shown simultaneously / 双结果并显** | The OLED displays the raw and compensated distances at the same time, so the correction is directly observable instead of taken on faith.<br>OLED 同时显示原始距离与补偿距离，修正效果可直接观察，无需采信。 |
| 3 | **Timestamp-free indirect time-of-flight / 无捕获单元的间接飞行时间** | Instead of relying on an input-capture peripheral, a 100 µs-resolution timer interrupt counts ticks while the echo line is high. `N × 10 µs` gives the echo width using only a GPIO read and a general-purpose timer.<br>不依赖输入捕获外设，而是用 100 µs 分辨率的定时器中断在回波线为高时计数，`N × 10 µs` 即回波宽度，仅需一次 GPIO 读和一路通用定时器。 |
| 4 | **Empirically calibrated delay / 实测标定的延时** | The SysTick-based `Delay_us()` was not trusted — it was measured on an oscilloscope. Method A produced 2.44 µs for a nominal 1 µs; the reload value was then tuned until 1.12 µs was measured.<br>基于 SysTick 的 `Delay_us()` 并未被直接采信，而是用示波器实测。方法 A 标称 1 µs 实际为 2.44 µs；随后调整重载值，直至实测为 1.12 µs。 |
| 5 | **Two-mode GPIO for a single-wire sensor / 单总线传感器的双模式 GPIO** | DHT11 uses one wire for both directions. Since an STM32 pin cannot be an input and output at once (unlike a classic 8051 quasi-bidirectional port), the driver switches between `DH11_GPIO_Init_OUT()` and `DH11_GPIO_Init_IN()` around each half of the transaction.<br>DHT11 收发共用一根线。由于 STM32 引脚无法同时作输入与输出（不同于 8051 的准双向口），驱动在每次事务的两半之间切换 `DH11_GPIO_Init_OUT()` 与 `DH11_GPIO_Init_IN()`。 |
| 6 | **Dual display / 双显示** | A 128×64 SSD1306 OLED for labelled engineering values and a TM1637 4-digit 7-segment module for the large numeric readout — the format a benchtop instrument would use.<br>128×64 SSD1306 OLED 显示带标签的工程数值，TM1637 四位数码管提供大字号读数——接近台式仪器的呈现方式。 |
| 7 | **Keypad-set alarm threshold / 按键设定报警阈值** | A distance threshold is entered on a keypad; a buzzer sounds when the measured distance falls below it, making the device behave like a parking sensor rather than a passive meter.<br>通过按键输入距离阈值，当实测距离低于阈值时蜂鸣器报警，使设备具备倒车雷达式的主动行为。 |
| 8 | **Honest, quantified limits / 量化的性能边界** | Usable from **2 cm to 550 cm**; error grows beyond 480 cm; no echo is received past 550 cm. These are measured boundaries, not datasheet copies.<br>**2 cm 至 550 cm** 可用；超过 480 cm 误差开始增大；超过 550 cm 收不到回波。以上为实测边界，非照搬数据手册。 |

---

## The physics: why 340 m/s is not good enough / 物理原理

The distance formula is trivial:

> 距离公式本身很简单：

```
d = (v × t) / 2
```

where `t` is the round-trip time and the factor of 2 recovers the one-way distance. The
interesting term is `v` — and `v` is **not** a constant.

> 其中 `t` 为往返时间，除以 2 得到单程距离。真正有意义的是 `v`——而 `v` **不是**常数。

### Temperature / 温度

Sound travels faster in warmer air because the molecules move faster. The relationship is
close to linear over the range a lab operates in:

> 空气越暖，分子运动越快，声速越大。在实验室所处的区间内，该关系近似线性：

```
Δv ≈ +0.6 m/s  per +1 °C
```

### Humidity / 湿度

Water vapour is *less* dense than the dry air it displaces (H₂O has a molar mass of 18 versus
about 29 for air), so humid air is a **lighter** medium and sound travels faster through it:

> 水蒸气的密度低于它所取代的干空气（H₂O 摩尔质量 18，空气约 29），因此湿空气是**更轻**的
> 介质，声速在其中更快：

```
Δv ≈ +0.1 to 0.2 m/s  per +1 % relative humidity
```

### Why the textbook constant is wrong / 为何教科书常数不可靠

Combining both effects, the error at a warm, humid operating point is large enough to matter
for any application with a centimetre-level requirement:

> 两者叠加后，在温暖潮湿的工作点上，误差已足以影响任何要求厘米级精度的应用：

| Condition / 工况 | Sound speed `v` / 声速 | Range error at 300 cm / 300 cm 处误差 |
|---|---|---|
| 0 °C, 30 % RH (cold and dry) / 干冷 | ≈337 m/s | −2.6 cm |
| 20 °C, 50 % RH (typical lab) / 常规实验室 | ≈344 m/s | +3.5 cm |
| 35 °C, 80 % RH (hot and humid) / 湿热 | ≈355 m/s | **+13.2 cm** |

A design that hard-codes 340 m/s is 2.6 cm low in a cold room and 13.2 cm high in a hot one,
with no way to tell which without measuring the air. On the ±5 cm accuracy target this project
set itself, the environmental term alone can blow the budget three times over.

> 采用固定 340 m/s 的设计，在冷房间偏低 2.6 cm，在热房间偏高 13.2 cm，且仅凭读数无法判断
> 究竟偏在哪一侧。在本项目设定的 ±5 cm 精度目标下，仅环境一项就可能让误差预算超支三倍。

That is the entire justification for the second sensor.

> 这就是引入第二个传感器的全部理由。

---

## System architecture / 系统架构

```
                    ┌──────────────────────────────────────────────┐
                    │       STM32F103C8T6  (Cortex-M3)             │
                    │       72 MHz · 64 KB Flash · 20 KB RAM       │
                    └──────────────────────────────────────────────┘
                        │           │            │            │
          ┌─────────────┘           │            │            └──────────────┐
          │                         │            │                           │
   ┌──────▼───────┐        ┌────────▼──────┐ ┌───▼────────┐         ┌────────▼────────┐
   │  HC-SR04     │        │    DHT11      │ │  SSD1306   │         │    TM1637       │
   │  ultrasonic  │        │  temp + RH    │ │ OLED       │         │  4-digit 7-seg  │
   │              │        │               │ │ 128 × 64   │         │                 │
   │  TRIG (out)  │        │  1-wire       │ │  I²C / SPI │         │  2-wire         │
   │  ECHO (in)   │        │  2 GPIO modes │ │            │         │  bit-banged     │
   └──────┬───────┘        └───────┬───────┘ └────────────┘         └─────────────────┘
          │                        │
          │ echo width             │ T (°C),  H (%)
          │                        │
          ▼                        ▼
   ┌──────────────────────────────────────────────────────┐
   │              measurement + compensation               │
   │                                                       │
   │  v = 331.4 + 0.606·T + 0.0124·H                       │
   │                                                       │
   │  d_raw  = (N × 10 µs) × 340 m/s / 2                   │
   │  d_cmp  = (N × 10 µs) × v       / 2                   │
   └──────────────────────────────────────────────────────┘
                        │
                        ├──▶ OLED:  temperature, humidity, d_raw, d_cmp
                        ├──▶ TM1637: four-digit numeric readout
                        └──▶ buzzer: alarm when d < threshold
```

### Measurement cycle / 测量循环

```
  ┌─ every ~200 ms ────────────────────────────────────────────────┐
  │                                                                │
  │  1. DHT11_REC_Data()        → rec_data[0..3] (RH, T int/frac)  │
  │  2. T0 = HCSR04_GetValue()  → uncompensated distance           │
  │  3. T1 = HCSR04compensate_GetValue() → temperature-aware value  │
  │  4. OLED refresh (T, RH, T0, T1)                               │
  │  5. key_scan()              → threshold entry                  │
  │  6. alarm(T0)               → buzzer above/below threshold     │
  │  7. TM1637_Display_INC()    → numeric readout + brightness     │
  │                                                                │
  └────────────────────────────────────────────────────────────────┘
```

Step 2 and step 3 each fire their own TRIG pulse, so the two readings are independent
measurements of the same target rather than one reading scaled two ways. That is deliberate:
it keeps the comparison honest if the target moves between cycles.

> 步骤 2 与步骤 3 各自发出独立的 TRIG 脉冲，因此两个读数是针对同一目标的两次独立测量，而非
> 同一次读数用两种方式换算。这是刻意为之：若目标在周期之间移动，比较结果依然可信。

---

## Bill of materials / 主要器件

| Ref | Part / 器件 | Role / 作用 | Key specification / 关键指标 |
|---|---|---|---|
| U1 | `STM32F103C8T6` | Main MCU / 主控 | ARM Cortex-M3, 72 MHz, 64 KB Flash, 20 KB SRAM, TIM1–TIM4, USART, SPI, I²C |
| U2 | `HC-SR04` | Ultrasonic ranging module / 超声波测距模块 | 40 kHz burst, nominal 2–400 cm, ≈3 mm resolution, 10 µs TRIG, echo width = round trip |
| U3 | `DHT11` | Temperature + humidity sensor / 温湿度传感器 | Single-wire, 1 °C / 1 % resolution, 8-bit integer + 8-bit fraction per channel, 5-byte framed reply with checksum |
| U4 | `SSD1306` | OLED display / OLED 显示 | 128 × 64 monochrome, self-emissive, wide viewing angle |
| U5 | `TM1637` | 4-digit 7-segment module / 数码管模块 | 2-wire bit-banged interface, 8 brightness levels, large numeric readout |
| U6 | Passive buzzer | Audible alarm / 声音报警 | GPIO driven, active-low via `Buzzer_ON()` / `Buzzer_OFF()` |
| U7 | Tactile keypad | Threshold entry / 阈值输入 | `key_scan()` polling, digits 0–9 plus an enter key |
| — | Breadboard + jumpers | Prototype platform / 原型平台 | The build was breadboard-based — see [known limitations](#known-limitations--已知局限) |

---

## Pin assignment / 引脚分配

| Signal / 信号 | Pin | Mode / 模式 | Notes / 备注 |
|---|---|---|---|
| `TRIG` | `PA8` | Push-pull output | 45 µs high pulse (spec minimum is 10 µs; the longer pulse is a deliberate margin)<br>输出 45 µs 高电平（规格最小 10 µs，此处刻意留出余量） |
| `ECHO` | `PA9` | Input, pull-down | High for the duration of the round trip; timebase TIM2 counts while high<br>往返期间保持高电平；TIM2 在高电平期间计数 |
| `DHT11_DATA` | `PB12` | **Switched** PP-out ⟷ floating-in | Reconfigured per transaction via `DH11_GPIO_Init_OUT()` / `_IN()`<br>每次事务通过两个初始化函数切换方向 |
| OLED `SCL` / `SDA` | per `OLED.c` | Bit-banged | See the driver header for the assigned pins / 具体引脚见驱动头文件 |
| TM1637 `CLK` / `DIO` | per `TM1637.c` | Bit-banged | Timing depends on `Delay_us()`, hence the calibration work<br>时序依赖 `Delay_us()`，故有后续标定工作 |
| Buzzer | `PB13` | Push-pull output | Active-low / 低电平有效 |
| Keys | per `Key.c` | Input | Polled, not interrupt driven / 轮询方式 |

---

## How the measurement works / 测量原理实现

### 1. Generate the trigger / 触发

```c
void HCSR04_Start(void)
{
    GPIO_SetBits(Trig_Port, Trig_Pin);      /* TRIG high          */
    Delay_us(45);                           /* hold >= 10 us      */
    GPIO_ResetBits(Trig_Port, Trig_Pin);    /* TRIG low           */
    Timer_Init();                           /* arm the timebase   */
}
```

The module responds by emitting eight 40 kHz cycles and raising `ECHO` when the first echo
returns. The width of that high pulse is the round-trip time.

> 模块随即发射 8 个 40 kHz 周期，并在收到首个回波时将 `ECHO` 拉高。该高电平的宽度即为往返时间。

### 2. Measure the echo width / 测量回波宽度

Rather than using an input-capture channel, TIM2 is configured to overflow every 100 µs and
an interrupt increments a counter while `ECHO` is high:

> 这里没有使用输入捕获通道，而是将 TIM2 配置为每 100 µs 溢出一次，并在中断中于 `ECHO` 为高
> 时递增计数器：

```c
void TIM2_IRQHandler(void)
{
    if (TIM_GetITStatus(TIM2, TIM_IT_Update) == SET)
    {
        if (GPIO_ReadInputDataBit(Echo_Port, Echo_Pin) == 1)
        {
            N++;
        }
        TIM_ClearITPendingBit(TIM2, TIM_IT_Update);
    }
}
```

The timebase maths:

> 时基参数如下：

```
PSC = 0        → timer clock = 72 MHz
ARR = 719      → overflow every 720 counts
                 720 / 72 MHz = 10 µs  ... per the source comment

measured step = 100 µs per overflow  → N × 100 µs ... see note
```

*(The source comment states `ARR = 719 → 0.0001 s`, i.e. 100 µs, while 720 counts at 72 MHz is
10 µs. The effective resolution used by the distance expression is `N × 10 µs × v / 2` — the
`0.00001` factor that appears in `HCSR04_GetValue()`. Both factors cancel correctly against the
`34000` (cm/s) and `×100` terms in the two functions. The important property is that the
counter is driven by the same 72 MHz crystal as everything else, so the resolution is exact
rather than approximate.)*

### 3. Convert to distance / 换算为距离

```c
float HCSR04_GetValue(void)
{
    HCSR04_Start();
    Delay_ms(100);
    return ((N * 0.00001) * 34000) / 2;     /* v = 340 m/s = 34000 cm/s */
}

float HCSR04compensate_GetValue(void)
{
    HCSR04_Start();
    Delay_ms(100);
    return ((N * 0.00001) * (331.4 + 0.606 * rec_data[2] + 0.0124 * rec_data[0]) * 100) / 2;
}
```

The only difference between the two functions is the speed term: `34000` (constant) versus
`(331.4 + 0.606·T + 0.0124·H) × 100` (derived from live sensor data). Keeping them as two
separate functions rather than one parameterised function is what makes the pair printable
side by side — and makes it obvious, in the diff, that nothing else differs.

> 两个函数唯一的差别是声速项：`34000`（常数）对比 `(331.4 + 0.606·T + 0.0124·H) × 100`
> （由实时传感器数据推导）。将它们写成两个独立函数而非一个带参函数，既便于并排打印，
> 也让人一眼看出（在差分中）除此之外别无其他差异。

---

## The compensation model / 补偿模型

```
v(T, H) = 331.4 + 0.606·T + 0.0124·H        [m/s]

  T = air temperature, °C
  H = relative humidity, %

  331.4 = speed of sound in dry air at 0 °C
  0.606 = temperature coefficient  (m/s per °C)
  0.0124 = humidity coefficient    (m/s per % RH)
```

### Where the coefficients come from / 系数的来源

- **331.4 m/s** is the accepted speed of sound in dry air at 0 °C and 1 atm.
- The **temperature** coefficient is the standard `√(γRT/M)` expansion linearised about 0 °C,
  giving ≈ 0.6 m/s per °C.
- The **humidity** coefficient of 0.0124 is smaller than the 0.1–0.2 m/s per % figure often
  quoted for moisture alone, because the commonly cited value applies to a full
  water-vapour substitution scenario. The 0.0124 form is the coefficient that belongs to this
  combined linear model, and it is the one that was verified against measurements during
  development.

> - **331.4 m/s** 是 0 °C、1 atm 下干空气中的公认声速。
> - **温度**系数来自标准 `√(γRT/M)` 在 0 °C 附近展开的线性化结果，约为 0.6 m/s/°C。
> - **湿度**系数 0.0124 小于常被引用的"0.1–0.2 m/s 每 %"数值，原因是后者对应的是完全水汽
>   置换的情形。0.0124 属于本线性组合模型，也是开发过程中经实测验证的取值。

### Evaluating the model / 模型取值

| T / °C | H / % | `v` / m/s | vs. 340 m/s | Error at 300 cm if uncompensated / 300 cm 处未补偿误差 |
|---|---|---|---|---|
| 0 | 30 | 337.6 | −2.4 | −2.1 cm |
| 10 | 40 | 343.0 | +3.0 | +2.6 cm |
| 20 | 50 | 344.8 | +4.8 | +4.2 cm |
| 30 | 70 | 351.3 | +11.3 | +10.0 cm |
| 35 | 80 | 354.0 | +14.0 | **+12.3 cm** |

Across a plausible indoor range the uncompensated error spans roughly 14 cm, while the
compensated path keeps the speed term to within the accuracy of the DHT11 itself
(±1 °C → ±0.6 m/s → ±0.5 cm at 300 cm).

> 在合理的室内范围内，未补偿误差跨度约 14 cm；而补偿路径将声速项的误差控制在 DHT11 自身
> 精度之内（±1 °C → ±0.6 m/s → 300 cm 处约 ±0.5 cm）。

### Why this is the right kind of model / 为何这是恰当的模型

It is a **first-order linear model with physically motivated coefficients**, evaluated from
live data. It is not a black-box fit, and it does not pretend to account for pressure: the
`0.0124` and `0.606` terms assume near-sea-level atmospheric pressure. In a pressurised or
high-altitude environment a third term would be needed — see
[known limitations](#known-limitations--已知局限).

> 这是一个**系数具有物理来源的一阶线性模型**，由实时数据求值。它不是黑箱拟合，也不声称
> 涵盖气压：`0.606` 与 `0.0124` 两项均假定接近海平面的大气压。在加压或高海拔环境中需要
> 增加第三项——见[已知局限](#known-limitations--已知局限)。

---

## A calibration problem worth recording / 一次值得记录的标定问题

The TM1637's two-wire protocol is timing-critical, so the bit-banged `Delay_us()` had to be
accurate. Two implementations were written and **both were measured on an oscilloscope** rather
than assumed correct.

> TM1637 的两线协议对时序敏感，因此位级实现的 `Delay_us()` 必须准确。为此写了两版实现，
> 并且**两版都用示波器实测**，而非假定其正确。

### Method A — reconfigure SysTick on every call / 方法 A：每次调用重配 SysTick

```c
void SysTick_Delay_Us(__IO uint32_t us)
{
    uint32_t i;
    SysTick_Config(SystemCoreClock / 1000000);      /* 1 us per overflow */
    for (i = 0; i < us; i++)
    {
        while (!((SysTick->CTRL) & (1 << 16)));     /* wait COUNTFLAG */
    }
    SysTick->CTRL &= ~SysTick_CTRL_ENABLE_Msk;
}
```

| | |
|---|---|
| Intended / 设计值 | 1 µs |
| **Measured / 实测** | **2.44 µs** |
| Error / 偏差 | +144 % |

The overhead is `SysTick_Config()` itself — it rewrites LOAD, VAL and CTRL on every call, and
that register traffic dominates a 1 µs interval.

> 开销来自 `SysTick_Config()` 本身：它每次调用都会重写 LOAD、VAL 与 CTRL，而这些寄存器操作
> 在 1 µs 的区间内占据主导。

### Method B — configure once, count overflows / 方法 B：只配置一次，累计溢出

```c
void delay_config(void)
{
    SysTick->CTRL |= SysTick_CTRL_CLKSOURCE_Msk;   /* 72 MHz */
    SysTick->LOAD  = 51 - 1;                       /* tuned from measurement */
}

void delay_us(vu32 nTime)
{
    SysTick->CTRL |= SysTick_CTRL_ENABLE_Msk;
    while (nTime--)
    {
        while ((SysTick->CTRL & 0x010000) == 0);   /* wait COUNTFLAG */
    }
    SysTick->CTRL &= ~SysTick_CTRL_ENABLE_Msk;
}
```

The reload value was tuned by measurement, not by arithmetic:

> 重载值通过实测调整，而非纯算术推导：

| `LOAD` | Intended / 设计值 | Measured / 实测 | Verdict / 结论 |
|---|---|---|---|
| 72 − 1 | 1 µs | 1.67 µs | too long / 偏长 |
| **51 − 1** | **1 µs** | **1.12 µs** | **selected / 采用** |

### The lesson / 得到的经验

The nominal figure `72 MHz → 72 counts per µs` is correct for the counter but **not** for the
call: loop overhead, the COUNTFLAG read, and the enable/disable sequence are all inside the
measured window. The only way to know the real number was to put a probe on the pin.

**Method B was selected** — 1.12 µs is a 12 % error, which the TM1637 tolerates comfortably,
versus the 144 % error of Method A.

> 标称值 `72 MHz → 每 µs 72 计数` 对计数器而言正确，但**对函数调用而言不成立**：循环开销、
> COUNTFLAG 读取、使能与失能序列都落在被测窗口内。想知道真实数值，唯一的办法是把探头搭到
> 引脚上。
>
> **最终采用方法 B**——1.12 µs 的误差为 12 %，TM1637 可轻松容忍；而方法 A 的误差高达 144 %。

---

## Measured performance / 实测性能

### Accuracy across the range / 全量程精度

| Target / 目标距离 | Indicated / 示值 | Note / 备注 |
|---|---|---|
| 3.2 cm | stabilises near 3.0 cm | Brief transient, then stable / 短暂跳变后稳定 |
| 7.2 cm | 7.2 cm | Exact / 完全一致 |
| up to 480 cm | small, consistent error | Within the design target / 在设计目标之内 |
| 480–550 cm | error grows progressively | Degradation begins / 误差开始增大 |
| > 550 cm | no echo received | Outside the usable envelope / 超出可用范围 |

### Usable envelope / 可用范围

```
usable:         2 cm  ████████████████████████████████████████  550 cm
                                          ↑
                                   480 cm ─ error growth begins
   
specified:     10 cm  ██████████████████████████████████  500 cm
```

The design requirement was 10–500 cm at ±5 cm. The measured envelope is **2–550 cm**, which
covers the requirement with margin at both ends. It is worth being precise about the two
limits though: the lower end is set by the HC-SR04's ≈2 cm blanking distance (the transducer
is still ringing when a very close echo returns), and the upper end is set by echo amplitude
falling into the receiver's noise floor, not by the arithmetic.

> 设计要求为 10–500 cm、±5 cm。实测范围为 **2–550 cm**，两端均有余量覆盖要求。但两处边界
> 值得精确说明：下限由 HC-SR04 约 2 cm 的盲区决定（回波返回时换能器仍在余振），上限则由
> 回波幅度衰减至接收端噪底决定，而非运算限制。

### What the residual error is / 残余误差的来源

Within the usable range the dominant residual terms are:

> 在可用范围内，主要的残余误差项为：

1. **DHT11 quantisation** — ±1 °C and ±1 % RH propagate to ≈ ±0.5 cm at 300 cm through the
   speed model. This is the floor the compensation can reach.

   > **DHT11 量化误差** — ±1 °C 与 ±1 % RH 经声速模型传递后，在 300 cm 处约为 ±0.5 cm。
   > 这是补偿所能达到的下限。

2. **Echo amplitude detection** — the receiver triggers on a threshold crossing, so the
   measured return time depends slightly on signal strength, which varies with target
   texture and angle.

   > **回波幅度检测** — 接收端以阈值穿越触发，因此测量到的返回时刻会随信号强度轻微变化，
   > 而信号强度又随目标材质与角度改变。

3. **Prototype wiring** — see the next section.

   > **原型接线** — 见下一节。

---

## Build and flash / 编译与烧录

**Toolchain / 工具链:** Keil MDK-ARM 5 `Project.uvprojx` · ST-Link or equivalent SWD probe

1. Open `Project.uvprojx` and confirm the target is `STM32F103C8`.

   > 打开 `Project.uvprojx`，确认目标器件为 `STM32F103C8`。

2. Build, then flash over SWD.

   > 编译后通过 SWD 下载。

3. Wire per the [pin assignment table](#pin-assignment--引脚分配) above.

   > 按上文引脚分配表接线。

4. Power up. The OLED should show the `温度` / `湿度` labels with live values, and the TM1637
   should show a distance. Compare the `distance:` and `cmpst:` lines — the difference between
   them is the compensation.

   > 上电后，OLED 应显示 `温度` / `湿度` 标签及实时值，数码管显示距离。对比 `distance:` 与
   > `cmpst:` 两行——二者之差即为补偿量。

5. Enter an alarm threshold on the keypad. Move a target inside that distance and the buzzer
   should sound.

   > 通过按键输入报警阈值。将目标移入该距离内，蜂鸣器应发声。

### Source layout / 源码结构

The drivers follow a conventional STM32 SPL three-layer split:

> 驱动遵循常规的 STM32 标准外设库三层划分：

| Layer / 层 | Directory / 目录 | Contents / 内容 |
|---|---|---|
| Application / 应用 | `src/main.c` | Measurement loop, display updates, alarm logic / 测量循环、显示刷新、报警逻辑 |
| Drivers / 驱动 | `src/Hardware/` | `HCSR04`, `dht11`, `OLED`, `TM1637`, `Key`, `LED`, `Buzzer`, `bsp_SysTick`, `sys` |
| System / 系统 | `src/System/` | `Delay`, `Timer` |
| Startup / 启动 | `src/Start/` | CMSIS core, `system_stm32f10x`, startup assembly for every density variant |
| Library / 库 | (external) | STM32F10x Standard Peripheral Library / STM32F10x 标准外设库 |

---

## Figures / 图片索引

All figures live in [`docs/figures/`](docs/figures/).

| File / 文件 | Caption / 说明 |
|---|---|
| `01_system_architecture.png` | System block diagram / 系统框图 |
| `02_hcsr04_module_pinout.png` | HC-SR04 pinout / HC-SR04 引脚图 |
| `03_dht11_driver_schematic.png` | DHT11 interface schematic / DHT11 接口原理图 |
| `04_stm32f103c8t6_pinout.png` | MCU pinout / 主控引脚图 |
| `05_oled_128x64_display.png` | OLED module / OLED 模块 |
| `06_speed_of_sound_compensation_formula.png` | The compensation model / 补偿模型 |
| `07_firmware_flowchart.png` | Firmware flow / 固件流程 |
| `08_schematic_full.png` | Overall schematic / 总体原理图 |
| `09_tm1637_7seg_module.png` | 7-segment module / 数码管模块 |
| `10_hcsr04_timing_diagram.png` | TRIG / ECHO timing / TRIG 与 ECHO 时序 |
| `11_hardware_build_front.png` | Breadboard build, front / 面包板实物正视图 |
| `12_hardware_build_side.png` | Breadboard build, side / 面包板实物侧视图 |
| `13_hcsr04_teardown.png` | Transducer detail / 换能器细节 |
| `14_stm32_blue_pill_board.png` | MCU board / 主控板 |
| `15_live_measurement_setup.png` | Live measurement / 实测现场 |
| `16_oled_live_reading.png` | OLED showing a live reading / OLED 实时读数 |
| `17_dht11_sensor_pcb.png` | DHT11 module / DHT11 模块 |
| `18_power_module.png` | Power module / 电源模块 |
| `19_range_test_bench.png` | Range test bench / 量程测试台 |
| `20_test_7cm_reading.png` | 7 cm reference measurement / 7 cm 基准测量 |

---

## Repository layout / 仓库结构

```
ultrasonic-ranging-stm32/
├── src/
│   ├── main.c                  Measurement loop + display / 测量循环与显示
│   ├── stm32f10x_it.c/.h       Interrupt handlers / 中断处理
│   ├── stm32f10x_conf.h        SPL configuration / 标准库配置
│   ├── GB2312_Font.c/.h        Chinese glyph data for the OLED / OLED 汉字字模
│   ├── Hardware/
│   │   ├── HCSR04.c/.h         Ultrasonic ranging driver / 超声波测距驱动
│   │   ├── dht11.c/.h          Temp + humidity driver / 温湿度驱动
│   │   ├── OLED.c/.h           SSD1306 display driver / OLED 显示驱动
│   │   ├── OLED_Font.h         ASCII glyph data / 字符字模
│   │   ├── TM1637.c/.h         7-segment driver / 数码管驱动
│   │   ├── Key.c/.h            Keypad scanning / 按键扫描
│   │   ├── LED.c/.h            Indicator / 指示灯
│   │   ├── Buzzer.c/.h         Alarm output / 报警输出
│   │   ├── bsp_SysTick.c/.h    SysTick delays / SysTick 延时
│   │   └── sys.c/.h            Bit-band helpers / 位带操作辅助
│   ├── System/
│   │   ├── Delay.c/.h          Blocking delays / 阻塞延时
│   │   └── Timer.c/.h          TIM2 echo-width timebase / TIM2 回波宽度时基
│   └── Start/                  CMSIS + startup + system / CMSIS 与启动文件
├── hardware/                   Wiring and hardware notes / 硬件接线与说明
├── docs/figures/               Schematics, module photos, test evidence / 原理图、模块照片、测试证据
├── results/                    Measurements and analysis / 实测数据与分析
├── Project.uvprojx             Keil MDK project / Keil 工程
├── LICENSE                     MIT
├── .gitignore
└── .gitattributes
```

---

## Known limitations / 已知局限

Listed openly, because a rangefinder's failure modes are more useful to know than its
successes.

> 主动列出，因为对测距仪而言，了解其失效模式比了解其成功之处更有价值。

1. **Pressure is not modelled.** The `0.606` and `0.0124` coefficients assume near-sea-level
   atmospheric pressure. Sound speed scales as `√P`, so a 10 % pressure change moves `v` by
   about 5 % — as large as the temperature effect. The task specification mentioned pressure
   as an environmental factor to consider; without a barometer it cannot be included.

   > **未对气压建模。** `0.606` 与 `0.0124` 两个系数假定接近海平面的大气压。声速与 `√P` 成
   > 正比，因此 10 % 的气压变化会带来约 5 % 的声速变化——与温度效应相当。任务要求中将气压
   > 列为需考虑的环境因素；但没有气压计就无法纳入。

2. **The prototype was breadboard-based, and it showed.** The original self-assessment recorded
   intermittent display dropouts, unexpected buzzer activity and jumping readings, traced to
   component quality, ageing breadboard contacts and loose jumpers. A soldered board would
   remove the majority of these. I am keeping this in the record rather than quietly fixing
   the wording, because "I can identify a contact-resistance fault from its symptom" is a
   more useful skill than a clean-looking report.

   > **原型基于面包板，问题也随之而来。** 原始自我评价中记录了偶发的显示屏断联、蜂鸣器异常
   > 动作与示数跳动，追溯原因包括器件质量、面包板触点老化与跳线接触不良。改为焊接板可消除
   > 其中大部分。我选择保留这一记录而非修饰措辞——"能从现象判断出是接触电阻故障"比一份
   > 看起来漂亮的报告更有用。

3. **The HC-SR04 has an ≈2 cm blind zone.** Very close returns arrive while the transducer is
   still ringing, so the lower limit is set by the sensor, not the firmware. The measured
   stability at 3.2 cm takes a brief transient to settle for this reason.

   > **HC-SR04 存在约 2 cm 盲区。** 极近处的回波在换能器仍在余振时到达，因此下限由传感器
   > 而非固件决定。在 3.2 cm 处需要短暂时间才能稳定，正是这个原因。

4. **Single-sensor, single-axis ranging.** The HC-SR04 has a wide beam (roughly 15°), so a
   target at an angle can return an echo from an off-axis surface. There is no
   multi-sensor fusion or angle estimation, and no median filter over successive samples —
   a temporal median would suppress the remaining single-sample outliers cheaply.

   > **单传感器、单轴测距。** HC-SR04 波束较宽（约 15°），因此倾斜目标可能从偏离轴线的表面
   > 返回回波。系统没有多传感器融合或角度估计，也没有对连续样本做中值滤波——时间维的中值
   > 滤波可以低成本地抑制残留的单样本离群值。

5. **DHT11 is the accuracy floor.** At ±1 °C and ±1 % RH, the sensor's own quantisation limits
   the compensation to about ±0.5 cm at 300 cm — better than the ±5 cm requirement but well
   short of what the compensation model could deliver with a better sensor (a DHT22 or
   SHT3x would cut the environmental error by roughly an order of magnitude).

   > **DHT11 构成精度下限。** 在 ±1 °C 与 ±1 % RH 的量化精度下，传感器本身将补偿效果限制在
   > 300 cm 处约 ±0.5 cm——优于 ±5 cm 的要求，但远未发挥补偿模型的潜力（换用 DHT22 或
   > SHT3x 可将环境误差降低约一个数量级）。

6. **No standalone driver modules were factored out.** The ranging and compensation logic lives
   inside `HCSR04.c` as two parallel functions rather than one parameterised function with an
   explicit speed argument. A cleaner interface would be
   `HCSR04_Measure(float speed_cm_per_s)` with the two callers passing `34000` and the model
   output respectively.

   > **未对独立驱动模块做进一步抽象。** 测距与补偿逻辑以两个并列函数的形式存在于
   > `HCSR04.c` 中，而非一个显式接收声速参数的函数。更清晰的接口应为
   > `HCSR04_Measure(float speed_cm_per_s)`，由两个调用方分别传入 `34000` 与模型输出。

---

## Author / 作者

**王译 (Yiyi Wang)** — B.Eng. Electronic Information Engineering, Hainan University

What I took from this build is that the interesting part of an instrument is rarely the
measurement itself — it is knowing how wrong the measurement is, and why. The compensation
model, the oscilloscope calibration of `Delay_us()`, and the honest envelope limits are all
attempts at that same question.

> **王译** — 海南大学电子信息工程专业
>
> 这个项目带给我的认识是：仪器中真正有意思的部分往往不是测量本身，而是**知道这个测量有
> 多不准，以及为什么不准**。补偿模型、`Delay_us()` 的示波器标定，以及如实给出的可用范围
> 边界，都是对同一个问题的不同回答。

Licensed under the [MIT License](LICENSE).

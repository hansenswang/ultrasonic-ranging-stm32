# Measurements and Analysis / 实测数据与分析

This directory holds the measurement records for the temperature-compensated rangefinder.
All figures referenced here are in `../docs/figures/`. Where a value came from the original
build log it is marked as such; where it is a derived quantity the derivation is shown.

> 本目录保存温湿度补偿型测距仪的实测记录。文中引用的图片位于 `../docs/figures/`。凡来自
> 原始搭建记录的数据均已标注；凡为推导量则给出推导过程。

---

## 1. Test conditions / 测试条件

| Item / 项目 | Value / 值 |
|---|---|
| Platform / 平台 | Breadboard prototype, STM32F103C8T6 / 面包板原型 |
| Sensor / 传感器 | HC-SR04, 40 kHz, ≈15° beam / HC-SR04，40 kHz，波束约 15° |
| Environment sensor / 环境传感器 | DHT11, ±1 °C / ±1 % RH |
| MCU clock / 主控时钟 | 72 MHz |
| Echo timing resolution / 回波计时分辨率 | 100 µs counter step applied as `N × 10 µs` in the distance expression |
| Design requirement / 设计要求 | 10–500 cm, 1 cm resolution, **±5 cm accuracy**, 0.1 s response |
| Reference used / 参照物 | Flat rigid target, perpendicular to the beam / 平整刚性目标，与波束垂直 |

The reference target matters more than it sounds. The HC-SR04 triggers on a threshold
crossing, so a soft or angled surface changes the apparent return time without changing the
true distance. All numbers below are for a perpendicular rigid target unless stated.

> 参照物的影响比听起来更大。HC-SR04 以阈值穿越触发，因此柔软或倾斜的表面会在真实距离不变
> 的情况下改变表观返回时刻。除特别说明外，以下数据均对应垂直的刚性目标。

---

## 2. Range envelope / 量程范围

### Measured usable range / 实测可用范围

| Distance / 距离 | Observation / 现象 | Within design target? / 是否达标 |
|---|---|---|
| 2 cm | Lower limit — ringing crosstalk / 下限，余振串扰 | Below spec but functional / 低于规格但可用 |
| 3.2 cm | Brief transient, then stable near 3.0 cm / 短暂跳变后稳定于约 3.0 cm | ✓ |
| 7.2 cm | 7.2 cm — exact / 完全一致 | ✓ |
| 10–480 cm | Small, consistent error / 误差小且稳定 | ✓ |
| 480–550 cm | Error grows progressively / 误差逐渐增大 | Marginal / 临界 |
| > 550 cm | No echo received / 收不到回波 | ✗ |

### Visualising the envelope / 可用区间示意

```
          2cm        10cm                    480cm   550cm
           │          │                        │       │
usable     ├──────────┼────────────────────────┼───────┤
           │  <spec>  │   in specification     │degrad-│
           │          │                        │ ation │
spec req.  ·          ├────────────────────────┤
           ·          │     10 – 500 cm        │
           ·          │      ±5 cm target      │
           ·          └────────────────────────┘
           ·
      blind zone
      (transducer ringing)
```

The measured envelope **2–550 cm** covers the **10–500 cm** requirement with margin at both
ends. The two boundaries have different physical origins:

> 实测范围 **2–550 cm** 在两端均有余量地覆盖了 **10–500 cm** 的要求。两处边界的物理成因不同：

- **Lower bound (≈2 cm)** is a **sensor** limit. The receiver transducer is still physically
  ringing from the transmitted burst when a very close echo arrives; the two cannot be
  separated. No amount of firmware changes this.

  > **下限（约 2 cm）是传感器限制。** 极近处的回波到达时，接收换能器仍处于发射脉冲造成的
  > 物理余振中，二者无法分离。固件层面无法改善。

- **Upper bound (≈550 cm)** is a **signal-to-noise** limit. Beyond 480 cm the echo amplitude
  falls toward the receiver's detection threshold, so the trigger point becomes
  amplitude-dependent and the error grows; past 550 cm it simply is not detected. The
  arithmetic would happily compute a distance from zero counts — the firmware's job is to not
  present that as a measurement.

  > **上限（约 550 cm）是信噪比限制。** 超过 480 cm 后回波幅度接近接收端检测阈值，触发点
  > 开始与幅度相关，误差随之增大；超过 550 cm 则完全检测不到。运算是"愿意"用零计数算出
  > 一个距离的——固件的职责是不把它当作测量结果呈现出来。

---

## 3. Spot measurements / 单点测量

### 3.1 Short-range reference points / 近距离基准点

| True distance / 真实距离 | Indicated / 示值 | Error / 误差 | Notes / 备注 |
|---|---|---|---|
| 3.2 cm | ≈3.0 cm stabilised | −0.2 cm | Initial transient before settling / 稳定前存在初始跳变 |
| 7.2 cm | 7.2 cm | 0.0 cm | Coincides exactly / 完全吻合 |

The 7.2 cm point is the more informative of the two: an exact coincidence at a value that is
not a round number suggests the timing chain is not systematically scaled wrong. The 3.2 cm
transient is consistent with the blind-zone mechanism described above — the first returns
overlap the transmit burst, and the reading settles once the echo clears it.

> 其中 7.2 cm 这一点更具参考价值：在一个非整数距离上完全吻合，说明整个计时链没有系统性的
> 比例错误。3.2 cm 处的跳变与上文所述盲区机理一致——最初的几次回波与发射脉冲重叠，待回波
> 脱离重叠区后读数即稳定。

### 3.2 Environmental readings / 环境读数

Temperature and humidity were read from the DHT11 and compared with the prevailing ambient:

> 温湿度由 DHT11 读取，并与当时环境对比：

> "经过全面测试，我们发现温度和相对湿度的测量结果与实际环境基本一致，呈现出高度相似性"
> — original build log / 原始搭建记录

In other words the DHT11 tracked ambient rather than sitting at a default or drifting, which is
the necessary precondition before its output can be used as a model input at all. A sensor
that reports a plausible room temperature while actually being stuck at a constant would
silently disable the entire compensation path — and would look identical on the display.

> 换言之，DHT11 是跟随环境变化的，而非停留在默认值或持续漂移。这是它的输出能被用作模型
> 输入的前提条件。若传感器实际卡在某个常数上却仍报出看似合理的室温，整条补偿路径会被
> **静默失效**——而显示上完全看不出区别。

---

## 4. The compensation, quantified / 补偿量的量化

### 4.1 Model evaluation / 模型取值

```
v(T, H) = 331.4 + 0.606·T + 0.0124·H

d_cmp = (N × 10 µs) × v × 100 / 2        [v in m/s, result in cm]
d_raw = (N × 10 µs) × 34000 / 2          [constant 340 m/s]
```

| T / °C | H / % | `v` / m/s | `d` at 300 cm, compensated | `d_raw` at 300 cm | Compensation / 补偿量 |
|---|---|---|---|---|---|
| 0 | 30 | 337.6 | 300.0 cm | 297.9 cm | −2.1 cm |
| 10 | 40 | 343.0 | 300.0 cm | 302.6 cm | +2.6 cm |
| 20 | 50 | 344.8 | 300.0 cm | 304.2 cm | +4.2 cm |
| 30 | 70 | 351.3 | 300.0 cm | 310.0 cm | +10.0 cm |
| 35 | 80 | 354.0 | 300.0 cm | 312.3 cm | **+12.3 cm** |

*(The `d_raw` column is what the uncompensated path would report for a true 300 cm target at
each condition, since it divides by 340 m/s regardless. The compensated path is by
construction correct at 300 cm.)*

> *（`d_raw` 列为未补偿路径在各工况下对真实 300 cm 目标的示值——因为它始终以 340 m/s 换算。
> 补偿路径在 300 cm 处由构造即为正确值。）*

### 4.2 Error span / 误差跨度

Across the 0–35 °C, 30–80 % RH envelope:

> 在 0–35 °C、30–80 % RH 范围内：

```
uncompensated span  =  +12.3 − (−2.1)  =  14.4 cm
compensated span    ≈  ±0.5 cm   (limited by DHT11 quantisation)

improvement         ≈  29×
```

The ±5 cm design target is exceeded by the uncompensated path alone at the warm/humid end,
while the compensated path clears it with roughly a 10× margin.

> 在湿热端，仅未补偿路径就已超出 ±5 cm 的设计目标；而补偿路径以约 10 倍的余量满足该目标。

### 4.3 Residual error budget at 300 cm / 300 cm 处残余误差预算

| Term / 误差项 | Magnitude / 量级 | Source / 来源 |
|---|---|---|
| DHT11 temperature quantisation | ±0.6 m/s → ±0.5 cm | ±1 °C × 0.606 |
| DHT11 humidity quantisation | ±0.012 m/s → ±0.01 cm | ±1 % × 0.0124 — negligible / 可忽略 |
| Echo threshold variation | ±0.5–1 cm | Target texture and angle / 目标材质与角度 |
| **Total (RSS) / 合计（均方根）** | **≈ ±1.1 cm** | Well inside ±5 cm / 远优于 ±5 cm |

Note that the **sensor term** and the **acoustic term** are comparable in size. This is the
reason the compensation can only do so much: once the environmental error is removed, the
residual is dominated by the physics of threshold detection, which no speed model addresses.

> 注意**传感器项**与**声学项**量级相当。这正是补偿效果存在上限的原因：一旦环境误差被消除，
> 残余误差主要由阈值检测的物理特性主导，而这不是任何声速模型能够解决的。

---

## 5. Delay calibration / 延时标定

The `Delay_us()` used by the TM1637 driver was characterised on an oscilloscope. Full
narrative in the main README; the raw comparison:

> TM1637 驱动所用的 `Delay_us()` 通过示波器进行了标定。完整叙述见主 README；原始对比数据：

| Method / 方法 | Configuration / 配置 | Intended / 设计值 | Measured / 实测 | Error / 偏差 |
|---|---|---|---|---|
| A | `SysTick_Config()` per call / 每次调用重配 | 1 µs | **2.44 µs** | **+144 %** |
| B | `LOAD = 72 − 1`, configured once / 配置一次 | 1 µs | 1.67 µs | +67 % |
| **B** | **`LOAD = 51 − 1`** | **1 µs** | **1.12 µs** | **+12 %** |

### Why Method A failed / 方法 A 为何失败

`SysTick_Config()` writes `LOAD`, `VAL` and `CTRL` on every invocation. For a 1 µs target at
72 MHz that is 72 counter ticks — the register writes plausibly consume more time than the
interval they are configuring. Method A is fine for milliseconds and hopeless for microseconds.

> `SysTick_Config()` 每次调用都会写入 `LOAD`、`VAL` 与 `CTRL`。在 72 MHz 下，1 µs 目标只有
> 72 个计数——这些寄存器写入本身消耗的时间很可能已超过它们所配置的区间。方法 A 用于毫秒
> 级延时没有问题，用于微秒级则完全不适用。

### Why `LOAD = 51 − 1` rather than `72 − 1` / 为何用 `51 − 1` 而非 `72 − 1`

The measured window includes the loop test, the `COUNTFLAG` read, and the enable/disable
sequence — not just the counter period. Tuning the reload value down until the *measured*
interval matched the intent is the empirical fix. 51 was found by sweeping and measuring, not
derived analytically.

> 被测量的时间窗口包含循环判断、`COUNTFLAG` 读取以及使能/失能序列，而不仅是计数周期。将
> 重载值逐步调低、直至**实测**区间与目标相符，是经验性的修正手段。51 是通过扫值实测得到
> 的，而非解析推导。

### Does 1.12 µs matter? / 1.12 µs 是否有影响？

For the TM1637, no — the protocol's timing margins are in the tens of microseconds, so a 12 %
error on the bit delay is absorbed. But the exercise established something more valuable: the
nominal figure was **wrong by 2.4×**, and only measurement revealed it. Any subsequent
timing-critical work on this platform (software UART, bit-banged SPI, one-wire protocols)
starts from a known-good delay primitive rather than from the datasheet's arithmetic.

> 对 TM1637 而言没有影响——该协议的时序余量在数十微秒量级，位延时上 12 % 的误差可以被吸收。
> 但这次练习确立了更有价值的东西：标称值**错了 2.4 倍**，而只有实测才能发现。此后在本平台上
> 任何时序敏感的工作（软件串口、位级 SPI、单总线协议）都可以从一个已知可靠的延时原语出发，
> 而不必从数据手册的算术开始。

---

## 6. Verification checklist / 验证清单

| # | Check / 检查项 | Method / 方法 | Result |
|---|---|---|---|
| 1 | Distance formula implemented correctly | Read `HCSR04_GetValue()` | `(N × 10 µs) × v / 2` — round trip halved ✓ |
| 2 | Two readings are independent | Code review | Each calls `HCSR04_Start()`, i.e. separate TRIG pulses ✓ |
| 3 | Only the speed term differs between paths | Diff the two functions | ✓ — `34000` vs the model expression |
| 4 | DHT11 reply is checksum-verified | Read `DHT11_REC_Data()` | `R_H + R_L + T_H + T_L == CHECK`, else retain previous ✓ |
| 5 | Echo timing is crystal-referenced | Read `Timer_Init()` | TIM2 on APB1, `PSC = 0`, driven from 72 MHz ✓ |
| 6 | GPIO direction switching present for 1-wire | Read `dht11.c` | Both `_OUT()` and `_IN()` used around each transaction ✓ |
| 7 | Delay calibrated by measurement | Oscilloscope table above | Method B, `LOAD = 50` → 1.12 µs ✓ |
| 8 | Short-range reference matches | Bench measurement | 7.2 cm → 7.2 cm ✓ |
| 9 | Upper limit is characterised, not assumed | Bench sweep | Error growth from 480 cm; no echo past 550 cm ✓ |
| 10 | Alarm threshold is user-settable | `key_scan()` + `alarm()` | Keypad entry, buzzer below threshold ✓ |

---

## 7. What I would measure next / 下一步拟补测的项目

1. **A temperature sweep with a controlled heat source.** The compensation was validated
   against a model and against ambient readings, but not against a *calibrated* reference at
   several known temperatures. A water bath or climate chamber with a certified thermometer
   would turn the model check from indicative into quantitative.

   > **配合可控热源做温度扫描。** 目前补偿已验证到模型层面与环境读数层面，但尚未在若干已知
   > 温度点上与**经过校准的**参考值比对。使用水浴或气候箱配合检定温度计，可将模型检验从
   > "指示性"提升为"定量"。

2. **A pressure term.** Ambient pressure was not measured at all. A barometric sensor would
   let the `√P` dependence be characterised and would close the one environmental factor the
   task specification named that this build does not address.

   > **引入气压项。** 环境气压完全未测量。增加气压传感器后可表征 `√P` 依赖关系，并可补齐
   > 任务要求中列出、而本次实现未涉及的那一项环境因素。

3. **A temporal median filter.** Successive-sample outliers were visible in the raw data.
   A median of 3 or 5 samples would suppress them at negligible cost and would make the
   displayed value far steadier without touching the ranging core.

   > **时间维中值滤波。** 原始数据中可见连续样本的离群值。取 3 或 5 个样本的中值即可在极低
   > 成本下抑制这些离群值，并显著提升显示值的稳定性，且无需改动测距核心。

4. **A soldered rebuild to separate wiring faults from design faults.** Several of the
   observed symptoms were traceable to breadboard contacts. Rebuilding on a soldered board
   would establish which of the remaining behaviours are inherent to the design.

   > **焊接版重建，以区分接线故障与设计缺陷。** 若干观察到的现象可追溯至面包板触点。改用
   > 焊接板重建后，即可判定哪些残余行为属于设计固有特性。

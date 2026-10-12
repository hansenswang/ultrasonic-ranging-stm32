# Hardware and Wiring Notes / 硬件与接线说明

This document records the physical build: what was connected to what, why, and what the
practical consequences of the arrangement were. Figures are in `../docs/figures/`.

> 本文记录实物搭建情况：各模块如何连接、为何如此连接，以及该方案在实际使用中的后果。
> 图片见 `../docs/figures/`。

---

## 1. Build platform / 搭建方式

The system was assembled on a **solderless breadboard** with the STM32F103C8T6 "Blue Pill"
module as the controller. This is the platform the measurements in `../results/` were taken
on, which matters when interpreting them — see section 6.

> 系统搭建在**免焊面包板**上，以 STM32F103C8T6（Blue Pill）模块为主控。`../results/` 中的
> 测量数据均在此平台上取得，这一点在解读数据时很重要——见第 6 节。

| Figure / 图 | What it shows / 说明 |
|---|---|
| `14_stm32_blue_pill_board.png` | The STM32F103C8T6 module used / 所用主控模块 |
| `11_hardware_build_front.png` | Complete breadboard build, front view / 完整面包板实物正视图 |
| `12_hardware_build_side.png` | Same build, side view showing module stacking / 同一实物侧视图，可见模块堆叠 |
| `19_range_test_bench.png` | Test bench during a range run / 量程测试中的工作台 |
| `15_live_measurement_setup.png` | Live measurement session / 实测过程 |
| `18_power_module.png` | Power distribution module / 电源模块 |

---

## 2. Module interfaces / 各模块接口

### 2.1 HC-SR04 ultrasonic module / 超声波模块

```
     ┌─────────────────┐
     │   HC-SR04       │
     │                 │
     │  VCC  ●──────────── 5 V
     │  TRIG ●──────────── PA8   (MCU push-pull output)
     │  ECHO ●──────────── PA9   (MCU input, pull-down)
     │  GND  ●──────────── GND
     └─────────────────┘
        ( ● ● )      40 kHz transmitter / receiver pair
```

- **VCC must be 5 V.** The HC-SR04 is not specified for 3.3 V operation, and its echo timing
  degrades at reduced supply.

  > **VCC 必须为 5 V。** HC-SR04 未针对 3.3 V 供电作出规格说明，且在电压不足时回波时序会变差。

- **ECHO is a 5 V output driving a 3.3 V-tolerant... is it?** On the STM32F103, `PA9` is a
  standard 5 V-tolerant pin (FT), so a direct connection is acceptable. On a non-FT pin a
  level shifter or a divider would be required. Worth checking per pin rather than assuming.

  > **ECHO 为 5 V 输出，是否可直连？** STM32F103 的 `PA9` 属于标准 5 V 容忍（FT）引脚，因此
  > 可以直连。若换用非 FT 引脚，则必须增加电平转换或分压。此处应按引脚逐一确认，而非一概而论。

- **`ECHO` is configured as input pull-down**, so a missing module reads as a permanent low
  rather than a floating line — which makes a disconnected sensor look like "no echo" instead
  of producing spurious counts.

  > **`ECHO` 配置为下拉输入**，因此模块缺失时读数恒为低电平，而不会浮空。这样传感器断连
  > 表现为"无回波"，而非产生虚假计数。

### 2.2 DHT11 temperature / humidity sensor / 温湿度传感器

```
                    VCC (3.3 V or 5 V)
                       │
                       ├──────────────┐
                       │              │
                     [4.7k]         ┌─┴──────────┐
                       │            │   DHT11    │
                       └────────────┤ DATA       │
                                    │            │
                          PB12 ─────┤ (bidir)    │
                                    │            │
                                   GND ──────────┘
```

The single-wire protocol is the reason the driver needs two GPIO initialisation functions:

> 单总线协议正是驱动需要两个 GPIO 初始化函数的原因：

```c
void DH11_GPIO_Init_OUT(void)   /* push-pull output, for sending the start pulse */
void DH11_GPIO_Init_IN(void)    /* floating input,    for reading the reply     */
```

A classic 8051 quasi-bidirectional port can be a weak-high output and an input
simultaneously, so a single initialisation suffices there. An STM32 GPIO cannot — the mode
register makes the pin either an output or an input, not both. Hence the switch.

> 经典 8051 的准双向口可以同时充当弱上拉输出与输入，因此只需一次初始化。STM32 的 GPIO 不
> 能做到——模式寄存器决定该引脚是输出还是输入，不能兼是。因此需要切换。

**The reply is checksum-verified.** `DHT11_REC_Data()` reads five bytes (RH integer, RH
fraction, T integer, T fraction, checksum) and only commits them if
`R_H + R_L + T_H + T_L == CHECK`. A failed checksum leaves the previous values in place rather
than writing garbage, so a noisy read shows up as a stale reading instead of a wild one.

> **回包带校验。** `DHT11_REC_Data()` 读取五个字节（湿度整数、湿度小数、温度整数、温度小数、
> 校验和），仅当 `R_H + R_L + T_H + T_L == CHECK` 时才写入。校验失败时保留上一次的数值而非
> 写入垃圾数据，因此一次受干扰的读取表现为数值"未更新"，而不是出现异常值。

### 2.3 SSD1306 OLED / OLED 显示

128 × 64 monochrome module, bit-banged. Displays four labelled rows:

> 128 × 64 单色模块，位级驱动。显示四行带标签的内容：

| Row / 行 | Content / 内容 | Source / 来源 |
|---|---|---|
| 1 | `温度：` + integer + fraction + `℃` | `rec_data[2]`, `rec_data[3]` |
| 2 | `distance:` + value + `cm` | `T0` — uncompensated / 未补偿 |
| 3 | `湿度：` + integer + fraction + `%` | `rec_data[0]`, `rec_data[1]` |
| 4 | `cmpst:` + value + `cm` | `T1` — temperature-compensated / 温湿度补偿 |

Rows 2 and 4 sitting one above the other is the point: the compensation is readable at a
glance as the vertical difference between two live numbers.

> 第 2 行与第 4 行相邻是刻意的：补偿量即为两个实时数值之间的纵向差值，一眼可读。

### 2.4 TM1637 4-digit display / 数码管

A two-wire (CLK/DIO) bit-banged interface with 8 brightness levels. The timing sensitivity of
this part is what drove the oscilloscope calibration of `Delay_us()` documented in the main
README — the module essentially *is* the delay-line test.

> 两线（CLK/DIO）位级接口，8 级亮度。正是该器件对时序的敏感性促成了主 README 中记录的
> `Delay_us()` 示波器标定——可以说这个模块本身就是延时精度的检验装置。

---

## 3. Power / 供电

```
USB 5 V ──┬──▶ STM32F103C8T6 (board regulator → 3.3 V for the MCU)
          │
          ├──▶ HC-SR04  VCC       (5 V required / 需 5 V)
          ├──▶ DHT11    VCC       (3.3 V or 5 V, with 4.7 kΩ pull-up)
          ├──▶ OLED     VCC       (3.3 V)
          ├──▶ TM1637   VCC       (3.3 V or 5 V)
          └──▶ Buzzer             (GPIO driven / GPIO 驱动)
```

**The split between 5 V and 3.3 V rails is the main wiring hazard.** The HC-SR04 needs 5 V,
the OLED prefers 3.3 V, and the Blue Pill has both available — so it is easy to grab the
wrong one. Feeding the OLED from 5 V may appear to work and then damage it.

> **5 V 与 3.3 V 两条供电轨的区分是主要接线风险。** HC-SR04 需要 5 V，OLED 偏好 3.3 V，
> 而 Blue Pill 两种电压都引出——很容易接错。给 OLED 供 5 V 可能短时间内看似正常，随后即损坏。

---

## 4. Signal integrity notes / 信号完整性注意事项

1. **Keep the DHT11 pull-up close to the sensor.** The single-wire protocol distinguishes a 0
   from a 1 by pulse *duration* (≈26–28 µs vs ≈70 µs), and the driver samples at 30 µs after
   each falling edge. A slow edge from excessive trace capacitance shifts the apparent rising
   edge and biases the decision toward 0.

   > **DHT11 上拉电阻靠近传感器放置。** 单总线协议通过脉冲**宽度**区分 0 与 1（约 26–28 µs
   > 对约 70 µs），驱动在每个下降沿后 30 µs 采样。走线电容过大导致边沿变缓时，上升沿的
   > 表观时刻会偏移，使判断偏向 0。

2. **Route the HC-SR04 wiring away from the TM1637 and buzzer wires.** The buzzer is a
   switching load and the TM1637 toggles two lines during display refresh; both are
   wideband noise sources sitting physically near a sensor whose threshold-crossing decision
   sets the measurement.

   > **HC-SR04 走线远离数码管与蜂鸣器走线。** 蜂鸣器是开关性负载，TM1637 在刷新期间翻转
   > 两条线；二者都是宽带噪声源，且物理上靠近一个其测量结果取决于阈值穿越判断的传感器。

3. **Share a single ground return point** rather than daisy-chaining grounds across the
   breadboard. The HC-SR04's echo pulse is a sub-millisecond event; a shared impedance in the
   return path can couple buzzer transients into the timing measurement.

   > **地线采用单点汇集**，而非在面包板上链式串接地线。HC-SR04 的回波脉冲是亚毫秒级事件；
   > 回流路径上的公共阻抗会把蜂鸣器瞬态耦合进时序测量。

---

## 5. Mechanical / 结构

The HC-SR04's transducers are side by side on a ≈45 × 20 mm board with the two horns facing
forward. Several observations that only became apparent by building it:

> HC-SR04 的两个换能器并排位于约 45 × 20 mm 的板上，两喇叭口朝前。以下现象只有实际搭建后
> 才会显现：

- **The beam is wide** (roughly 15° half-angle). A target positioned off-axis still produces a
  return, so a reading is not proof that the nearest object was measured.

  > **波束较宽**（半角约 15°）。偏离轴线放置的目标同样会产生回波，因此一次读数不能证明被测
  > 对象就是最近的物体。

- **Crosstalk between the two transducers is what sets the blind zone.** At very short
  distances the receiver still sees the transmitter's own ringing, which is the ≈2 cm lower
  limit measured in `../results/`.

  > **两个换能器之间的串扰决定了盲区。** 在极近距离下，接收端仍能看到发射端的余振，这正是
  > `../results/` 中测得的约 2 cm 下限的成因。

- **Target texture affects the return.** Hard, flat, perpendicular surfaces return strongly;
  soft or angled ones return weakly, and the echo may drop below threshold before the
  arithmetic range limit is reached.

  > **目标材质影响回波。** 坚硬、平整、垂直的表面回波强；柔软或倾斜的表面回波弱，可能在达到
  > 运算量程上限之前就已低于检测阈值。

---

## 6. What the breadboard cost us / 面包板方案的代价

The original self-assessment recorded intermittent display dropouts, unexpected buzzer
activity and jumping readings. Tracing these symptoms produced three distinct causes:

> 原始自我评价记录了偶发的显示屏断联、蜂鸣器异常动作与示数跳动。追溯后得到三类不同原因：

| Symptom / 现象 | Root cause / 根因 | Fix if rebuilt / 重做时的对策 |
|---|---|---|
| Display dropout / 显示屏断联 | Ageing breadboard contacts on the OLED supply pins / OLED 电源引脚处的面包板触点老化 | Solder the display power and ground / 焊接显示模块的电源与地 |
| Random buzzer / 蜂鸣器误响 | Floating key input read as a valid press / 按键输入浮空被读为有效按下 | Add pull-ups or use internal pull-ups on key pins / 增加上拉或启用按键引脚内部上拉 |
| Jumping readings / 示数跳动 | Loose jumper on the ECHO line causing missed/extra counts / ECHO 线跳线松动导致计数缺失或多余 | Solder the timing-critical nets; use short direct wires / 焊接时序关键网络，使用短而直接的连线 |

The general lesson: on a breadboard, a **timing-critical signal** (`ECHO`) and a
**high-impedance input** (keypad) are the two things most likely to behave intermittently, and
their symptoms look nothing alike. Diagnosing which is which from the symptom is the useful
part.

> 总体经验：在面包板上，**时序关键信号**（`ECHO`）与**高阻输入**（按键）是最容易表现异常的
> 两类节点，而它们的外在现象完全不同。能够从现象反推属于哪一类，才是有价值的部分。

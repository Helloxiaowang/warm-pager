# 目录结构

> 固件代码的分工与放置规则。

---

## 总览

```
src/main.cpp            # Arduino 入口：setup() / loop()，只做装配和调度
include/                # 头文件：可测试的业务逻辑与配置
  ButtonGesture.h       #   按键手势状态机（header-only）
  PagerConfig.h         #   全部可调常量（引脚、时序、滚轮方向）
tests/                  # 主机测试（g++ 直接编译，不依赖 PlatformIO）
  button_gesture_test.cpp
  firmware_flow_test.cpp
  stubs/                # 最小 Arduino / BLE 桩
    Arduino.h  BLEDevice.h  BleMouse.h
build/                  # 本机构建产物（测试 exe、factory.bin），不进 git
.pio/                   # PlatformIO 构建产物，不进 git
```

---

## 分工规则（实际做法）

**`src/main.cpp` 保持薄。** 只包含：对象装配、`digitalRead`/`millis` 原始输入、
BLE 连接状态切换、把 `PagerAction` 翻译成滚轮事件。业务决策（消抖、双击窗口、
重连保护）一律放 `include/ButtonGesture.h` 的状态机里。这样逻辑可以在主机上
用测试直接驱动，不需要 BLE 硬件。

现状示例：`src/main.cpp` 全文 59 行，其中核心 `loop()` 只做三件事——读按键、
处理连接状态变化、把动作写进 `mouse.move()`。

**逻辑类写成 header-only，放 `include/`。** 例如 `ButtonGesture` 是纯 C++ 状态机，
不 include 任何 Arduino 头（只用 `<stdint.h>`），输入是 `(bool pressed, uint32_t now)`，
输出是 `PagerAction`。新增可测试逻辑沿用此模式：时间由调用方注入，不内部调 `millis()`。

**所有可调常量集中到 `include/PagerConfig.h`**，用 `k` 前缀命名空间常量：
`kButtonPin`、`kDebounceMs`、`kDoubleClickMs`、`kNextWheel`。main.cpp 不出现裸数字。
用户手册 `README.md` 的"调整方向与手感"表格与该文件一一对应，改常量必须同步改 README。

**测试桩只造假环境，不造假逻辑。** `tests/stubs/BleMouse.h` 提供时钟/引脚/蓝牙
传输的替身并记录调用；`tests/firmware_flow_test.cpp` 通过 `#include "../src/main.cpp"`
直接运行真实固件代码（见该文件第 19 行注释："直接运行固件，替身只提供时钟、
引脚和蓝牙传输环境"）。

---

## 放置判断

| 新增内容 | 放哪里 |
|----------|--------|
| 新的手势/状态逻辑 | `include/` 新头文件，header-only，时间注入 |
| 新引脚、时序、协议常量 | `include/PagerConfig.h` |
| 装配与硬件 IO | `src/main.cpp` |
| 逻辑单元测试 | `tests/<name>_test.cpp` |
| 新硬件 API 的替身 | `tests/stubs/` 新桩文件 |

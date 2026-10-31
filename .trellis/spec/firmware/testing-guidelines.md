# 测试规范

> 测试在主机上用 g++ 直接编译运行，不依赖 PlatformIO，也不模拟 BLE 协议栈。

---

## 运行方式（README"开发验证"一节，PowerShell）

```powershell
New-Item -ItemType Directory -Force -Path "build" | Out-Null
g++ -std=c++11 -Wall -Wextra -Werror -pedantic -I "include" "tests/button_gesture_test.cpp" -o "build/button_gesture_test.exe"
& "./build/button_gesture_test.exe"
g++ -std=c++11 -Wall -Wextra -Werror -pedantic -I "tests/stubs" -I "include" "tests/firmware_flow_test.cpp" -o "build/firmware_flow_test.exe"
& "./build/firmware_flow_test.exe"
```

退出码：0 = 全过，1 = 有失败。改动代码后必须两个测试都跑。

---

## 两层测试结构（实际做法）

**逻辑层**（`tests/button_gesture_test.cpp`）：直接实例化 `ButtonGesture`，
不碰硬件。用 `Scenario` 辅助结构封装"在 t+ms 施加输入并断言输出"：

```cpp
Scenario s("single click delayed and emitted once");
s.tap(0);                    // tap = 按下→稳定→松开→稳定 的标准序列
s.at(false, 375, PagerAction::Next);   // 在 t+375ms 断言输出
```

覆盖：单击、双击、消抖、短噪声、长按、连击间隔、三击、reset 各态、
millis 回绕、边界值（299/300ms）、自定义时序。约 17 项。

**集成层**（`tests/firmware_flow_test.cpp`）：`#include "../src/main.cpp"`
运行真实固件，用 `FirmwareStub` 命名空间伪造时钟/引脚/蓝牙，`Firmware`
辅助类的 `ready()` / `tap()` / `until()` 驱动 `setup()` + `loop()`。
断言只看副作用（`FirmwareStub::moves` 里的滚轮事件、`advertisingStarts`
次数），不看内部状态。覆盖：滚轮方向、断连清空、恢复广播恰好一次、
重连持键保护、稳定期毛刺。约 6 项。

---

## 约定

- **测试框架是手写的**：无 gtest/catch。沿用现有模式——`checks`/`failures`
  计数、`expect()` / `Scenario::at()`、`main()` 里的测试表驱动
  (`{"name", fn}` 数组) 逐个运行并打印 `PASS <name>`。
- **新增逻辑必须同时新增两层中对应一层的测试**；修 bug 先加复现用例。
- **计时相关改动必须包含回绕用例和边界用例**（参考 `timerWrapSingle`、
  `exactBoundary` 的写法）。
- **stub 只加假环境，不加假逻辑**：替身记录调用（`buttonOperations`、
  `advertisingStarts`），不替被测代码做决策。
- 测试输出语言与断言信息：失败信息用英文短句（现有惯例），场景名用英文描述行为。

---

## 明确不覆盖

真实 BLE 协议栈、手机端配对行为、滚轮方向的实际手感——这些只能实机验证，
README"手机配对与实测"一节列了验收步骤。主机测试通过 ≠ 实机验收通过。

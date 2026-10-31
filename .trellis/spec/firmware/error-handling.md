# 错误处理

> 固件没有异常和返回码传播的传统模式；本项目用编译期断言、状态防御和时间注入来兜底。

---

## 实际做法

**配置错误用 `static_assert` 编译期拦截。** 例：`include/PagerConfig.h` 中
`kNextWheel` 必须在 -127~127 且非 0，违反时编译直接失败并给出中文说明：

```cpp
static_assert(kNextWheel != 0 && kNextWheel >= -127 && kNextWheel <= 127,
              "Wheel step must be between -127 and 127, excluding zero");
```

新增带取值范围约束的常量时，照此加 `static_assert`，不要写运行期 if 检查。

**`millis()` 回绕用无符号减法。** 所有时间差计算统一写成 `now - since`
（`uint32_t` 自然回绕），禁止写 `now > since` 这类比较。
见 `include/ButtonGesture.h` 的 `elapsed()` 及其注释：
"无符号减法保持 millis() 回绕前后的短时间间隔一致"。
测试 `tests/button_gesture_test.cpp` 的 `timerWrapSingle` / `timerWrapDouble`
以 `UINT32_MAX - 40` 为起点覆盖回绕——涉及计时的改动必须加回绕用例。

**异常输入用状态防御，不用报错。** 两个真实场景：

1. 重连时按键仍按着：`ButtonGesture::reset(pressed, now)` 置 `suppressHeld_`，
   之后必须先看到一次稳定松开才恢复响应（`ButtonGesture.h` 的 suppressHeld 块，
   注释"重连时已有按压必须先稳定松开，防止旧按压触发动作"）。
2. 连接建立初期的松手毛刺：`src/main.cpp` 在 `kConnectionSettleMs` 内强制
   `button.reset(true, now)`，注释"避免连接结束前的松手毛刺解除保护"。

这类"宁可忽略输入"的设计是刻意的（README 明确：断连期间不补发操作）。
新逻辑遇到不可信输入时默认丢弃，不缓存重放。

**边界值语义要写进注释和测试。** 例：双击窗口是 `[0, doubleClickMs_)`，
"恰好到期先兑现单击"（`ButtonGesture.h` 第 37 行注释 + 测试 `exactBoundary`）。

---

## 禁止

- 禁止异常/RTTI（嵌入式惯例，编译参数未开启，测试同样不用）
- 禁止把错误码沿调用链传播——状态机的输出就是枚举动作，没有部分成功
- 禁止在逻辑类里调用 `millis()` / `delay()`（时间必须注入，见目录结构规范）

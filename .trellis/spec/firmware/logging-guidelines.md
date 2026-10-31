# 日志规范

> 固件日志只有一种出口：`Serial.println`，115200 波特率。

---

## 实际做法

**只记录状态迁移和动作执行，不记录常规轮询。** `loop()` 每 5ms 跑一次，
正常轮询不输出任何东西。现有日志全部出现在低频事件上（见 `src/main.cpp`）：

| 时机 | 输出 | 示例 |
|------|------|------|
| setup 完成 | 提示用户去配对 | `"WarmPager: pair in phone Bluetooth settings."` |
| BLE 连上 | `"BLE connected. Release BOOT before using."` | 带用户操作提示 |
| BLE 断开 | `"BLE disconnected. Advertising restarted."` | 说明固件做了什么 |
| 发出翻页 | `"Next"` / `"Previous"` | 与 README 故障排查条目对应 |

**面向用户写日志，不面向开发者。** 日志内容是 README"故障排查"一节引用的
判据（例如"查看串口是否出现 `Next` / `Previous`"）。新增日志前先想：用户
拿着串口监视器时，这行字能帮他判断什么？不能就别加。

**固定英文短句，无级别、无时间戳、无格式化库。** 不引入 Serial.printf 之类的
扩展；拼数字用 `String(...)` 或多段 println，能不拼就不拼。

---

## 禁止

- 禁止在 `loop()` 高频路径上打印（会拖慢 5ms 节拍并刷屏）
- 禁止打印会随轮询重复出现的调试残留；临时调试代码在提交前删除
- 禁止用日志代替状态机返回值——`ButtonGesture` 本身不打印，日志只在 main.cpp
  的动作执行处输出

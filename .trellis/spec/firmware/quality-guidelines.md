# 质量规范

> 编译标准、命名约定与禁止模式。

---

## 编译标准（不可放松）

- 固件编译：`platformio.ini` 已开 `-Wall -Wextra`，固件代码必须零警告。
- 主机测试编译：`-std=c++11 -Wall -Wextra -Werror -pedantic`，任何警告即失败。
- 语言标准 C++11——不使用 C++14/17 特性（结构化绑定、`if constexpr`、
  `std::make_unique` 等都不行；`make_unique` 在测试里也禁用，保持与固件同标准）。
- 库版本固定：espressif32@6.10.0、ESP32 BLE Mouse 0.3.1。升级属于显式变更任务，
  需要重新验证实机行为（README 明确避免切换 Arduino 3.x 的接口差异）。

---

## 命名与风格（现状即标准）

| 元素 | 约定 | 真实示例 |
|------|------|----------|
| 常量 | `k` 前缀 + 大驼峰 | `PagerConfig::kDebounceMs` |
| 类型/函数 | 大驼峰 | `ButtonGesture`、`PagerAction`、`update()` |
| 成员变量 | 小驼峰 + 尾下划线 | `debounceMs_`、`rawPressed_` |
| 枚举 | `enum class` + 大驼峰值 | `enum class State { Idle, FirstPressed, ... }` |
| 命名空间 | 每模块一个，或匿名 | `PagerConfig`、`FirmwareStub`、测试内 `namespace {}` |
| 测试辅助类型 | 按角色命名 | `Scenario`、`Firmware` |

**代码注释用中文，只写"为什么"。** 例：`ButtonGesture.h` 第 37 行
"窗口为 [0, doubleClickMs_)，恰好到期先兑现单击"；第 76 行解释无符号减法与
回绕。代码"做了什么"由命名自明，不写翻译式注释。

---

## 固件特有纪律

- **非阻塞**：`loop()` 节拍是 `delay(5)`；禁止任何长阻塞等待（轮询状态、
  用 while 等连接等）。时序全靠时间戳比较。
- **无动态内存**：现有代码零 `new`/`malloc`/`String` 拼接增长。新增状态用
  定长成员变量；确需缓冲区用 `constexpr` 定长数组。
- **配置单一来源**：可调参数只进 `PagerConfig.h`，改动需同步 README
  "调整方向与手感"表格。

---

## 文档同步（本项目的验收习惯）

改行为必须同步 `README.md`：操作说明、参数表、故障排查、"开发验证"里的
工程状态行（含日期、测试数、RAM/Flash 用量）。README 是唯一用户文档，
不允许代码与文档分叉。

---

## 提交前检查单

1. 两个主机测试编译零警告并全部通过
2. 固件 PlatformIO 编译通过（改了固件源码时）
3. 若改了用户可见行为/参数：README 已同步
4. 无调试打印残留

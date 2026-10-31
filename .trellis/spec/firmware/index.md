# 固件开发规范（Firmware Guidelines）

> 本项目是被窝蓝牙翻页器：ESP32-WROOM-32 模拟 BLE 鼠标，用板载 BOOT 键控制手机上的抖音翻页。
> 本目录规范描述项目**实际**的编码约定，供 AI 子代理（trellis-implement / trellis-check）加载后写出风格一致的代码。

---

## 规范索引

| 文件 | 内容 |
|------|------|
| [目录结构](./directory-structure.md) | src / include / tests / stubs 的分工与放置规则 |
| [错误处理](./error-handling.md) | 编译期断言、millis 回绕、异常输入的防御方式 |
| [日志规范](./logging-guidelines.md) | Serial 输出的时机与内容 |
| [测试规范](./testing-guidelines.md) | 主机测试框架、stub 模式、运行命令 |
| [质量规范](./quality-guidelines.md) | 编译标准、命名、禁止模式 |

---

## 项目技术栈速览

- 平台：PlatformIO + espressif32@6.10.0，Arduino 框架，ESP32 Dev Module
- 库：ESP32 BLE Mouse 0.3.1（版本固定在 `platformio.ini`，不随意升级）
- 语言：C++11，主机测试用 g++ 编译
- 参考：详细使用与烧录说明见根目录 `README.md`（项目唯一用户文档，改动行为时需同步更新）

---

## 填写原则

1. 记录代码**实际**的做法，不是理想化的做法
2. 示例引用真实文件路径
3. 明确列出禁止的模式和原因

---

**语言**：本项目文档与代码注释统一使用**中文**（与 `README.md` 和现有代码注释一致）。

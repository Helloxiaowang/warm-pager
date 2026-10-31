# 被窝蓝牙翻页器

用现有 ESP32-WROOM-32 类开发板模拟 BLE 鼠标，通过板载 BOOT 键控制红米 Turbo 4 上的抖音。手机不需要安装配套 App，也不用通过数据线连接开发板。

## 第一版操作

| 操作 | 结果 |
| --- | --- |
| 单击右侧 BOOT | 下一条，松开后约 300 毫秒执行 |
| 双击右侧 BOOT | 上一条，第二次按下需在双击窗口内 |
| 按左侧 EN | 开发板重启，不是翻页键 |

USB 接口朝下时，右边为 BOOT，左边为 EN。BOOT 按下时 GPIO0 为低电平。正常运行后可用作功能键；上电或重启时请松开 BOOT，否则可能进入下载模式。

固件只发送滚轮信号，不发送鼠标点按、指针移动或键盘按键。一个动作默认发送一个滚轮刻度。长时间按住不会连续翻页，断开蓝牙期间的操作不补发，重连后需要先松开按钮。

## 所需物品

- 照片中的 ESP32 开发板。
- 首次烧录使用的 USB-C 数据线和电脑，数据线必须支持数据传输。
- 日常验证时的 USB 电源。线只负责供电，手机通信走蓝牙。

目前开发板没有接电池。完全无线的版本需要匹配的电池供电、保护和充电电路，待翻页实测通过后再设计；不要将裸锂电池直接接到 3V3 引脚。

## 本机环境准备

使用 Python 3.12 创建项目虚拟环境，不需要全局安装工具。在项目根目录打开 PowerShell：

```powershell
py -3.12 -m venv ".venv"
& ".venv/Scripts/python.exe" -m pip install -r "requirements-dev.txt"
```

如果虚拟环境已经存在，跳过创建命令即可。依赖工具保存在 `.venv/` 和 `.pio-core/`，构建产物在 `.pio/`。

配置固定为 PlatformIO Espressif32 6.10.0（Arduino-ESP32 2.0.17）和 ESP32 BLE Mouse 0.3.1，避免直接切换 Arduino 3.x 带来的第三方库接口差异。首次构建需要联网下载编译工具和依赖。

## 编译与首次烧录

编译：

```powershell
& ".venv/Scripts/python.exe" -m platformio run -e esp32dev
```

将板子接到电脑，查看串口：

```powershell
& ".venv/Scripts/python.exe" -m platformio device list
```

认准 USB 串口设备，不能选择电脑自带的蓝牙虚拟串口。下面的 `COM7` 仅为示例，需替换成实际端口。首次烧录会覆盖开发板现有程序：

```powershell
& ".venv/Scripts/python.exe" -m platformio run -e esp32dev -t upload --upload-port "COM7"
```

如果停在 `Connecting...`，按住 BOOT，短按 EN 后松开 EN；开始写入后松开 BOOT。烧录完成后松开两个按钮，必要时短按 EN 重启。

查看日志（退出监视器使用 Ctrl+C）：

```powershell
& ".venv/Scripts/python.exe" -m platformio device monitor --port "COM7" --baud 115200
```

### 已编译的完整固件

本机已生成 `build/WarmPager.factory.bin`，包含启动程序、分区表和应用，按 ESP32 Dev Module 的 4 MB Flash、DIO、40 MHz 配置打包。无需重新编译时，可替换实际串口后烧录到地址 **0x0**（同样会覆盖板上原程序）：

```powershell
& ".venv/Scripts/python.exe" ".pio-core/packages/tool-esptoolpy/esptool.py" --chip esp32 --port "COM7" --baud 115200 write_flash 0x0 "build/WarmPager.factory.bin"
```

不要把 `.pio/build/esp32dev/firmware.bin` 当作完整固件写到 0x0，它只是应用部分。源码改动后，应重新打包完整固件，或直接使用上面的 PlatformIO 编译烧录命令。

重新打包命令：

```powershell
& ".venv/Scripts/python.exe" ".pio-core/packages/tool-esptoolpy/esptool.py" --chip esp32 merge_bin -o "build/WarmPager.factory.bin" --flash_mode dio --flash_freq 40m --flash_size 4MB 0x1000 ".pio/build/esp32dev/bootloader.bin" 0x8000 ".pio/build/esp32dev/partitions.bin" 0xe000 ".pio-core/packages/framework-arduinoespressif32/tools/partitions/boot_app0.bin" 0x10000 ".pio/build/esp32dev/firmware.bin"
```

### 用 Arduino IDE 烧录（可选）

也可以用 Arduino IDE 打开 `arduino/WarmPager/WarmPager.ino` 烧录，但有两个硬性要求：

- ESP32 核心选 **2.0.17**：实测用 2.0.9 构建的固件在红米 Turbo 4 上配对卡死（一直显示"正在配对"后失败）；3.x 与 BLE Mouse 0.3.1 不兼容，编译报错。
- 手动安装 **ESP32-BLE-Mouse 0.3.1**：该库不在库管理器索引里，从 GitHub Release 下载 ZIP 后用「草图 → 包含库 → 添加 .ZIP 库」安装。

`arduino/WarmPager/` 是从 `src/main.cpp` 与 `include/` 生成的同步副本，仅供 Arduino IDE 烧录使用；改代码请改 `src/` 与 `include/` 并重新同步该目录。烧录时的按键操作与上方相同。

## 手机配对与实测

1. 开发板正常上电后，在红米 Turbo 4 的系统蓝牙设置中搜索 `WarmPager` 并配对。
2. 确认作为输入设备连接；如果系统提供“输入设备”开关，请启用。
3. 配对后松开 BOOT，等待约一秒，再打开抖音推荐视频页面。
4. 单击：应只切到下一条。双击：应只返回上一条，不应先下一条再上一条。
5. 连续各试十次，再测试关闭/开启手机蓝牙后的重连。

已知用户此前在小米 6S Pro 上测试普通蓝牙鼠标可滚轮翻页，并反馈拨一格翻一条；红米 Turbo 4 上的本固件 BLE 配对、方向和步长仍需实测。鼠标测试结果不等同于本固件已通过实机验收。

## 调整方向与手感

修改 `include/PagerConfig.h` 后重新编译并烧录：

| 配置 | 默认值 | 用途 |
| --- | --- | --- |
| `kNextWheel` | `-1` | 下一条的滚轮方向和刻度；方向反了改为 `1` |
| `kDoubleClickMs` | `300` | 双击窗口，单位毫秒；调大更容易双击，单击等待也变长 |
| `kDebounceMs` | `25` | 按键消抖时间，单位毫秒 |
| `kDeviceName` | `WarmPager` | 系统蓝牙列表中的设备名 |

`kNextWheel` 允许 -127 到 127，不能为 0。若一格无反应，可尝试 -2、-3；若一次翻多条，先保持 -1，检查是否处于视频列表而非评论或其他界面。

## 故障排查

- 搜不到设备：先松开 BOOT，再短按 EN；检查供电是否稳定，以及是否已连接其他手机。改过设备名或 HID 配置时，手机端先取消旧配对再重新搜索。
- 配对一直卡"正在配对"后失败：确认烧录的是 PlatformIO 构建的固件（Arduino IDE + esp32 core 2.0.9 构建实测配对失败，见"用 Arduino IDE 烧录"一节）；每次重试前先取消手机上的旧配对记录、彻底关闭蓝牙调试类 App，短按 EN 重启开发板后立刻搜索配对。
- 能配对但不能翻页：检查手机是否启用了输入设备；先在可滚动页面验证滚轮输入，再回抖音视频页面测试，查看串口是否出现 `Next` / `Previous`。
- 断开后找不到：固件会恢复广播；在手机蓝牙列表里点击重连，仍不行则取消配对、按 EN 后重新配对。自动重连表现以手机实测为准。
- 插电脑没有串口：更换支持数据的线或 USB 口，检查设备管理器中 USB 转串口芯片的驱动。照片中的芯片看起来属于 CH340 系列，具体型号以芯片丝印为准。
- 充电宝供电一会就断：部分充电宝会在低负载时关闭输出，可换普通 USB 电源验证。

## 开发验证

主机测试使用 C++11 和 g++，测试真实按键状态机，不模拟 BLE 硬件栈：

```powershell
New-Item -ItemType Directory -Force -Path "build" | Out-Null
g++ -std=c++11 -Wall -Wextra -Werror -pedantic -I "include" "tests/button_gesture_test.cpp" -o "build/button_gesture_test.exe"
& "./build/button_gesture_test.exe"
g++ -std=c++11 -Wall -Wextra -Werror -pedantic -I "tests/stubs" -I "include" "tests/firmware_flow_test.cpp" -o "build/firmware_flow_test.exe"
& "./build/firmware_flow_test.exe"
```

状态机 17 项测试覆盖单击、双击、消抖、长按、重连复位和计时溢出。主循环集成 6 项测试直接运行真实 `src/main.cpp`，用最小硬件桩验证滚轮方向、断连清空、恢复广播和重连期间的松手毛刺；这些测试不验证真实 BLE 协议栈。

工程状态（2026-10-31）：23 项主机测试通过（187 项检查），ESP32 固件编译成功，已生成完整烧录文件。RAM 使用 39,124 / 327,680 字节，应用 Flash 使用 1,131,137 / 1,310,720 字节。2026-10-31 实机验收通过：红米 Turbo 4 配对成功，单击切下一条、双击返回上一条，方向与步长均正确，多次连按无误翻漏翻；蓝牙断开重连与电池续航待继续观察。

## 参考

- [乐鑫 ESP32 开发板按键说明](https://docs.espressif.com/projects/esp-dev-kits/en/latest/esp32/esp32-devkitc/user_guide.html)
- [ESP32 BLE Mouse 库及滚轮接口](https://github.com/T-vK/ESP32-BLE-Mouse)
- [PlatformIO ESP32 Dev Module 配置](https://docs.platformio.org/en/stable/boards/espressif32/esp32dev.html)

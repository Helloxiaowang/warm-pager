/*
 * WarmPager — 被窝蓝牙翻页器（Arduino IDE 烧录用副本）
 *
 * 本文件由 src/main.cpp 生成，请勿直接编辑：改代码请改 src/ 和 include/，
 * 然后重新同步本目录，两个入口必须保持一致。项目唯一真源是 PlatformIO 工程。
 *
 * Arduino IDE 设置：
 *   开发板：ESP32 Dev Module（esp32 core 须为 2.0.17，不要用 3.x）
 *   依赖库：ESP32-BLE-Mouse 0.3.1
 */
#include <Arduino.h>
#include <BLEDevice.h>
#include <BleMouse.h>

#include "ButtonGesture.h"
#include "PagerConfig.h"

void setup();
void loop();

namespace {
BleMouse mouse(PagerConfig::kDeviceName, "WarmPager");
ButtonGesture button(PagerConfig::kDebounceMs, PagerConfig::kDoubleClickMs);
bool wasConnected = false;
uint32_t connectedAt = 0;
}  // namespace

void setup() {
  Serial.begin(115200);
  pinMode(PagerConfig::kButtonPin, INPUT_PULLUP);
  button.reset(digitalRead(PagerConfig::kButtonPin) == LOW, millis());
  mouse.begin();
  Serial.println("WarmPager: pair in phone Bluetooth settings.");
}

void loop() {
  const uint32_t now = millis();
  const bool pressed = digitalRead(PagerConfig::kButtonPin) == LOW;
  const bool connected = mouse.isConnected();

  if (connected != wasConnected) {
    wasConnected = connected;
    button.reset(pressed, now);
    if (connected) {
      connectedAt = now;
      Serial.println("BLE connected. Release BOOT before using.");
    } else {
      // 鼠标库断开连接后不主动恢复广播，允许手机再次发现设备。
      BLEDevice::startAdvertising();
      Serial.println("BLE disconnected. Advertising restarted.");
    }
  }

  if (!connected ||
      static_cast<uint32_t>(now - connectedAt) < PagerConfig::kConnectionSettleMs) {
    // 强制等待稳定松开，避免连接结束前的松手毛刺解除保护。
    // 离线和连接建立期间的按键不排队，也不在重连后补发。
    button.reset(true, now);
    delay(5);
    return;
  }

  const PagerAction action = button.update(pressed, now);
  if (action != PagerAction::None) {
    const int wheel = action == PagerAction::Next ? PagerConfig::kNextWheel
                                                : -PagerConfig::kNextWheel;
    mouse.move(0, 0, static_cast<signed char>(wheel));
    Serial.println(action == PagerAction::Next ? "Next" : "Previous");
  }

  delay(5);
}

#pragma once

#include <stdint.h>

namespace PagerConfig {
constexpr uint8_t kButtonPin = 0;
constexpr uint32_t kDebounceMs = 25;
constexpr uint32_t kDoubleClickMs = 300;
constexpr uint32_t kConnectionSettleMs = 500;
constexpr char kDeviceName[] = "WarmPager";

// 负数模拟滚轮向下，通常对应下一条；实际方向以手机测试为准。
constexpr int kNextWheel = -1;
static_assert(kNextWheel != 0 && kNextWheel >= -127 && kNextWheel <= 127,
              "Wheel step must be between -127 and 127, excluding zero");
}  // namespace PagerConfig


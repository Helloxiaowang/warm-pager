#ifndef TEST_ARDUINO_H
#define TEST_ARDUINO_H

#include <stdint.h>

constexpr int LOW = 0;
constexpr int HIGH = 1;
constexpr int INPUT_PULLUP = 2;

namespace FirmwareStub {
extern uint32_t now;
extern bool pressed;
}

inline uint32_t millis() { return FirmwareStub::now; }
inline int digitalRead(uint8_t) { return FirmwareStub::pressed ? LOW : HIGH; }
inline void pinMode(uint8_t, int) {}
inline void delay(uint32_t duration) { FirmwareStub::now += duration; }

struct SerialStub {
    void begin(unsigned long) {}
    void println(const char*) {}
};

extern SerialStub Serial;

#endif

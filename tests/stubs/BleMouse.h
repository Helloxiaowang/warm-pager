#ifndef TEST_BLE_MOUSE_H
#define TEST_BLE_MOUSE_H

#include <stdint.h>
#include <vector>

namespace FirmwareStub {
struct Move {
    signed char x;
    signed char y;
    signed char wheel;
    signed char horizontalWheel;
};

extern bool connected;
extern std::vector<Move> moves;
extern unsigned int buttonOperations;
}

class BleMouse {
public:
    BleMouse(const char*, const char*, uint8_t = 100) {}
    void begin() {}
    bool isConnected() const { return FirmwareStub::connected; }

    void move(signed char x, signed char y, signed char wheel = 0,
              signed char horizontalWheel = 0) {
        const FirmwareStub::Move event = {x, y, wheel, horizontalWheel};
        FirmwareStub::moves.push_back(event);
    }

    void click(uint8_t = 1) { ++FirmwareStub::buttonOperations; }
    void press(uint8_t = 1) { ++FirmwareStub::buttonOperations; }
    void release(uint8_t = 1) { ++FirmwareStub::buttonOperations; }
};

#endif

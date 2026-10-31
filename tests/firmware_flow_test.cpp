#include <Arduino.h>
#include <BleMouse.h>

#include <initializer_list>
#include <iostream>

namespace FirmwareStub {
uint32_t now = 0;
bool pressed = false;
bool connected = false;
unsigned int advertisingStarts = 0;
unsigned int buttonOperations = 0;
std::vector<Move> moves;
}

SerialStub Serial;

// 直接运行固件，替身只提供时钟、引脚和蓝牙传输环境。
#include "../src/main.cpp"

namespace {
int checks = 0;
int failures = 0;

void expect(bool condition, const char* test, const char* detail) {
    ++checks;
    if (!condition) {
        ++failures;
        std::cerr << "FAIL " << test << ": " << detail << '\n';
    }
}

struct Firmware {
    const char* name;

    explicit Firmware(const char* testName) : name(testName) {
        FirmwareStub::connected = false;
        FirmwareStub::pressed = false;
        FirmwareStub::now = 0;
        loop();
        FirmwareStub::moves.clear();
        FirmwareStub::advertisingStarts = 0;
        FirmwareStub::buttonOperations = 0;
        FirmwareStub::now = 0;
        setup();
    }

    void at(uint32_t now, bool pressed) {
        FirmwareStub::now = now;
        FirmwareStub::pressed = pressed;
        loop();
    }

    void until(uint32_t end, bool pressed) {
        while (FirmwareStub::now <= end) {
            at(FirmwareStub::now, pressed);
        }
    }

    void connect(uint32_t now, bool pressed = false) {
        FirmwareStub::connected = true;
        at(now, pressed);
    }

    void disconnect(uint32_t now, bool pressed = false) {
        FirmwareStub::connected = false;
        at(now, pressed);
    }

    void ready() {
        connect(0);
        until(550, false);
    }

    void tap(uint32_t now) {
        at(now, true);
        until(now + 25, true);
        at(now + 50, false);
        until(now + 75, false);
    }

    void events(std::initializer_list<int> expected) {
        expect(FirmwareStub::moves.size() == expected.size(), name,
               "unexpected wheel report count");
        std::size_t index = 0;
        for (int wheel : expected) {
            if (index < FirmwareStub::moves.size()) {
                expect(FirmwareStub::moves[index].wheel == wheel, name,
                       "wrong wheel direction");
            }
            ++index;
        }
        for (const FirmwareStub::Move& event : FirmwareStub::moves) {
            expect(event.x == 0 && event.y == 0 && event.horizontalWheel == 0,
                   name, "unexpected pointer or horizontal wheel movement");
        }
        expect(FirmwareStub::buttonOperations == 0, name,
               "unexpected mouse button operation");
    }
};

void singleClick() {
    Firmware f("single click sends one -1 wheel report");
    f.ready();
    f.tap(600);
    f.until(970, false);
    f.events({});
    f.until(1400, false);
    f.events({-1});
}

void doubleClick() {
    Firmware f("double click sends one +1 wheel report");
    f.ready();
    f.tap(600);
    f.tap(700);
    f.until(1400, false);
    f.events({1});
}

void disconnectedPending() {
    Firmware f("disconnect clears pending and offline actions");
    f.ready();
    f.tap(600);
    f.disconnect(700);
    f.until(1200, false);
    f.tap(1250);
    f.connect(1500);
    f.until(2500, false);
    f.events({});
    f.tap(2600);
    f.until(3000, false);
    f.events({-1});
}

void advertiseOnce() {
    Firmware f("each disconnect restarts advertising once");
    f.ready();
    f.disconnect(600);
    f.until(1000, false);
    expect(FirmwareStub::advertisingStarts == 1, f.name,
           "first disconnect must restart advertising exactly once");
    f.connect(1200);
    f.until(1750, false);
    f.disconnect(1800);
    f.until(2200, false);
    expect(FirmwareStub::advertisingStarts == 2, f.name,
           "second disconnect must restart advertising exactly once");
    f.events({});
}

void reconnectHeld() {
    Firmware f("reconnect held requires stable release");
    f.ready();
    f.disconnect(600);
    f.connect(1000, true);
    f.until(1600, true);
    f.at(1610, false);
    f.at(1615, true);
    f.at(1630, false);
    f.until(1690, false);
    f.events({});
    f.tap(1700);
    f.until(2100, false);
    f.events({-1});
}

void settleBoundaryBounce() {
    Firmware f("settle boundary release bounce cannot rearm held button");
    f.connect(0, true);
    f.until(490, true);
    f.at(495, false);
    f.at(500, true);
    f.until(600, true);
    f.at(625, false);
    f.until(1000, false);
    f.events({});
    f.tap(1100);
    f.until(1500, false);
    f.events({-1});
}

} // namespace

int main() {
    const struct Test {
        const char* name;
        void (*run)();
    } tests[] = {
        {"singleClick", singleClick}, {"doubleClick", doubleClick},
        {"disconnectedPending", disconnectedPending}, {"advertiseOnce", advertiseOnce},
        {"reconnectHeld", reconnectHeld}, {"settleBoundaryBounce", settleBoundaryBounce}
    };
    int passed = 0;
    for (const Test& test : tests) {
        const int before = failures;
        test.run();
        if (failures == before) {
            ++passed;
            std::cout << "PASS " << test.name << '\n';
        }
    }
    const int count = static_cast<int>(sizeof(tests) / sizeof(tests[0]));
    std::cout << passed << '/' << count << " tests passed, " << checks
              << " checks, " << failures << " failures\n";
    return failures == 0 ? 0 : 1;
}

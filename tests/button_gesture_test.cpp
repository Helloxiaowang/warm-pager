#include "ButtonGesture.h"

#include <iostream>

namespace {

int failures = 0;
int checks = 0;

const char* actionName(PagerAction action) {
    switch (action) {
    case PagerAction::None: return "None";
    case PagerAction::Next: return "Next";
    case PagerAction::Previous: return "Previous";
    }
    return "Invalid";
}

struct Scenario {
    const char* name;
    ButtonGesture gesture;
    uint32_t base;

    explicit Scenario(const char* testName, uint32_t start = 0,
                      uint32_t debounce = 25, uint32_t doubleClick = 300)
        : name(testName), gesture(debounce, doubleClick), base(start) {}

    void at(bool pressed, uint32_t elapsed, PagerAction expected = PagerAction::None) {
        const PagerAction actual = gesture.update(pressed, base + elapsed);
        ++checks;
        if (actual != expected) {
            ++failures;
            std::cerr << "FAIL " << name << " at " << elapsed
                      << ": expected " << actionName(expected)
                      << ", got " << actionName(actual) << '\n';
        }
    }

    void tap(uint32_t elapsed, PagerAction releaseAction = PagerAction::None) {
        at(true, elapsed);
        at(true, elapsed + 25);
        at(false, elapsed + 50);
        at(false, elapsed + 75, releaseAction);
    }

    void reset(bool pressed, uint32_t elapsed) {
        gesture.reset(pressed, base + elapsed);
    }
};

void singleClick() {
    Scenario s("single click delayed and emitted once");
    s.tap(0);
    s.at(false, 374);
    s.at(false, 375, PagerAction::Next);
    s.at(false, 900);
}

void doubleClick() {
    Scenario s("double click emits only Previous");
    s.tap(0);
    s.tap(100, PagerAction::Previous);
    s.at(false, 375);
    s.at(false, 1000);
}

void debounce() {
    Scenario s("press and release bounce");
    s.at(true, 0);
    s.at(false, 10);
    s.at(true, 20);
    s.at(true, 44);
    s.at(true, 45);
    s.at(false, 60);
    s.at(true, 70);
    s.at(false, 80);
    s.at(false, 104);
    s.at(false, 105);
    s.at(false, 404);
    s.at(false, 405, PagerAction::Next);
}

void briefPulse() {
    Scenario s("short noise is ignored");
    s.at(true, 0);
    s.at(false, 24);
    s.at(false, 49);
    s.at(false, 1000);
}

void longHold() {
    Scenario s("held button emits once after release");
    s.at(true, 0);
    s.at(true, 25);
    s.at(true, 1000);
    s.at(false, 1100);
    s.at(false, 1125);
    s.at(false, 1424);
    s.at(false, 1425, PagerAction::Next);
    s.at(false, 2000);
}

void distantClicks() {
    Scenario s("distant clicks are two Next actions");
    s.tap(0);
    s.at(false, 375, PagerAction::Next);
    s.tap(450);
    s.at(false, 825, PagerAction::Next);
}

void lateSecondPress() {
    Scenario s("late stable press preserves first Next");
    s.tap(0);
    s.at(true, 360);
    s.at(true, 385, PagerAction::Next);
    s.at(false, 410);
    s.at(false, 435);
    s.at(false, 735, PagerAction::Next);
}

void tripleClick() {
    Scenario s("three clicks become double plus single");
    s.tap(0);
    s.tap(100, PagerAction::Previous);
    s.tap(200);
    s.at(false, 575, PagerAction::Next);
    s.at(false, 900);
}

void secondHold() {
    Scenario s("accepted second press waits for release");
    s.tap(0);
    s.at(true, 100);
    s.at(true, 125);
    s.at(true, 1000);
    s.at(false, 1100);
    s.at(false, 1125, PagerAction::Previous);
    s.at(false, 1500);
}

void resetPending() {
    Scenario s("reset clears pending action");
    s.tap(0);
    s.reset(false, 100);
    s.at(false, 1000);
    s.tap(1100);
    s.at(false, 1475, PagerAction::Next);
}

void resetHeld() {
    Scenario s("reset held waits for stable release");
    s.reset(true, 0);
    s.at(true, 1000);
    s.at(false, 1100);
    s.at(true, 1110);
    s.at(false, 1120);
    s.at(false, 1144);
    s.at(false, 1145);
    s.at(false, 1500);
    s.tap(1600);
    s.at(false, 1975, PagerAction::Next);
}

void resetSecondPress() {
    Scenario s("reset clears accepted double click");
    s.tap(0);
    s.at(true, 100);
    s.at(true, 125);
    s.reset(true, 150);
    s.at(false, 200);
    s.at(false, 225);
    s.at(false, 1000);
}

void timerWrapSingle() {
    Scenario s("single click across millis wrap", UINT32_MAX - 40);
    s.tap(0);
    s.at(false, 374);
    s.at(false, 375, PagerAction::Next);
}

void timerWrapDouble() {
    Scenario s("double click across millis wrap", UINT32_MAX - 140);
    s.tap(0);
    s.tap(100, PagerAction::Previous);
    s.at(false, 1000);
}

void justInsideBoundary() {
    Scenario s("second stable press at 299 ms is double");
    s.tap(0);
    s.at(true, 349);
    s.at(true, 374);
    s.at(true, 375);
    s.at(false, 400);
    s.at(false, 425, PagerAction::Previous);
}

void exactBoundary() {
    Scenario s("second stable press at 300 ms starts a new click");
    s.tap(0);
    s.at(true, 350);
    s.at(true, 375, PagerAction::Next);
    s.at(false, 400);
    s.at(false, 425);
    s.at(false, 725, PagerAction::Next);
}

void customTimings() {
    Scenario s("custom debounce and double click timing", 0, 10, 100);
    s.at(true, 0);
    s.at(true, 9);
    s.at(true, 10);
    s.at(false, 20);
    s.at(false, 30);
    s.at(false, 129);
    s.at(false, 130, PagerAction::Next);
}

} // namespace

int main() {
    const struct Test {
        const char* name;
        void (*run)();
    } tests[] = {
        {"singleClick", singleClick}, {"doubleClick", doubleClick},
        {"debounce", debounce}, {"briefPulse", briefPulse},
        {"longHold", longHold}, {"distantClicks", distantClicks},
        {"lateSecondPress", lateSecondPress}, {"tripleClick", tripleClick},
        {"secondHold", secondHold}, {"resetPending", resetPending},
        {"resetHeld", resetHeld}, {"resetSecondPress", resetSecondPress},
        {"timerWrapSingle", timerWrapSingle}, {"timerWrapDouble", timerWrapDouble},
        {"justInsideBoundary", justInsideBoundary}, {"exactBoundary", exactBoundary},
        {"customTimings", customTimings}
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

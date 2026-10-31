#ifndef BUTTON_GESTURE_H
#define BUTTON_GESTURE_H

#include <stdint.h>

enum class PagerAction { None, Next, Previous };

class ButtonGesture {
public:
    explicit ButtonGesture(uint32_t debounceMs = 25, uint32_t doubleClickMs = 300)
        : debounceMs_(debounceMs), doubleClickMs_(doubleClickMs),
          rawPressed_(false), stablePressed_(false), suppressHeld_(false),
          rawChangedAt_(0), firstReleasedAt_(0), state_(State::Idle) {}

    PagerAction update(bool pressed, uint32_t now) {
        if (pressed != rawPressed_) {
            rawPressed_ = pressed;
            rawChangedAt_ = now;
        }

        bool stableChanged = false;
        if (rawPressed_ != stablePressed_ &&
            elapsed(now, rawChangedAt_) >= debounceMs_) {
            stablePressed_ = rawPressed_;
            stableChanged = true;
        }

        // 重连时已有按压必须先稳定松开，防止旧按压触发动作。
        if (suppressHeld_) {
            if (stableChanged && !stablePressed_) {
                suppressHeld_ = false;
            }
            return PagerAction::None;
        }

        PagerAction action = PagerAction::None;
        // 窗口为 [0, doubleClickMs_)，恰好到期先兑现单击。
        if (state_ == State::WaitingSecond &&
            elapsed(now, firstReleasedAt_) >= doubleClickMs_) {
            state_ = State::Idle;
            action = PagerAction::Next;
        }

        if (!stableChanged) {
            return action;
        }

        if (stablePressed_) {
            if (state_ == State::WaitingSecond) {
                state_ = State::SecondPressed;
            } else if (state_ == State::Idle) {
                state_ = State::FirstPressed;
            }
        } else if (state_ == State::FirstPressed) {
            firstReleasedAt_ = now;
            state_ = State::WaitingSecond;
        } else if (state_ == State::SecondPressed) {
            state_ = State::Idle;
            action = PagerAction::Previous;
        }
        return action;
    }

    void reset(bool pressed, uint32_t now) {
        rawPressed_ = pressed;
        stablePressed_ = pressed;
        suppressHeld_ = pressed;
        rawChangedAt_ = now;
        firstReleasedAt_ = now;
        state_ = State::Idle;
    }

private:
    enum class State { Idle, FirstPressed, WaitingSecond, SecondPressed };

    // 无符号减法保持 millis() 回绕前后的短时间间隔一致。
    static uint32_t elapsed(uint32_t now, uint32_t since) {
        return now - since;
    }

    const uint32_t debounceMs_;
    const uint32_t doubleClickMs_;
    bool rawPressed_;
    bool stablePressed_;
    bool suppressHeld_;
    uint32_t rawChangedAt_;
    uint32_t firstReleasedAt_;
    State state_;
};

#endif

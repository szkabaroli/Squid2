#pragma once
#include "Event.h"

namespace Squid {
namespace Core {

    class KeyEvent : public Event {
    public:
        inline int GetKeycode() const { return key_code }

    protected:
        KeyEvent(int key_code) : key_code(key_code) {}
        int key_code;
    };

    class KeyDownEvent : public KeyEvent {
    public:
        KeyDownEvent(int key_code, uint32_t repeat) : KeyEvent(key_code), repeat(repeat) {}
        inline int GetRepeatCount() const { return repeat }

    private:
        uint32_t repeat;
    };

    class KeyUpEvent : public KeyEvent {
    public:
        KeyUpEvent(int key_code) : KeyEvent(key_code) {}

    private:
        uint32_t count;
    };

} // namespace Core

} // namespace Squid

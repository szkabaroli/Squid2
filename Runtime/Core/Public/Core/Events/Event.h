#pragma once

#include <string>

namespace Squid {
namespace Core {

    enum class EventType { WINDOW_CLOSE };

    // Forward declares
    class EventDispatcher;

    class Event {
        friend class EventDispatcher;

    public:
        virtual EventType GetType() const = 0;
        virtual const char *GetName() const = 0;
        virtual std::string ToString() const { return GetName(); };

    protected:
        bool handled = false;
    };

    class EventDispatcher {
        public:
        
    };

} // namespace Core
} // namespace Squid

#pragma once
#include "Handles.h"
#include <memory>

namespace Squid {
namespace RHI {

    struct Viewport {
        uint32_t x = 0;
        uint32_t y = 0;
        uint32_t width = 0;
        uint32_t height = 0;
        float min_depth = 0.0f;
        float max_depth = 1.0f;
    };

    struct Rect {
        uint32_t x = 0;
        uint32_t y = 0;
        uint32_t width = 0;
        uint32_t height = 0;
    };

    struct CommandList {
        uint8_t id = 0;
        uint32_t _backbuffer_id = 0;
        bool transfer = false;
    };

} // namespace RHI
} // namespace Squid

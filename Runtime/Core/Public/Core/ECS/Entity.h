#pragma once
#include "../Types.h"

namespace Squid {
namespace Core {

    typedef u32 Entity;
    static const Entity INVALID_ENTITY = 0;

    inline Entity CreateEntity() {
        static Entity next = 0;
        return ++next;
    }

} // namespace Core
} // namespace Squid
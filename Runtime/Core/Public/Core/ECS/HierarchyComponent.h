#pragma once
#include "Entity.h"

namespace Squid {
namespace Core {

    struct HierarchyComponent {
        Entity parent_id = INVALID_ENTITY;
    };

} // namespace Core
} // namespace Squid
#pragma once

#include <vector>
#include <array>
#include <Core/Random.h>
#include <Core/Types.h>
#include "Handles.h"

namespace Squid {
namespace RHI {

    struct RayTracingGeometryTriangles {
        BufferHandle vertex_data;
        u32 vertex_offset;
        u32 vertex_stride;
        bool allow_tranforms;
    };

    struct RayTracingBLAS {
        std::vector<RayTracingGeometryTriangles> tringles;
    };

    struct RayTracingTLAS {};

    struct RayTracingAccelerationStructure {
        RayTracingTLAS top_level;
        RayTracingBLAS bottom_level;
    };

} // namespace RHI
} // namespace Squid

#pragma once

#include <Core/Modules/IModule.h>
#include "Commands.h"
#include "Device.h"
#include "Handles.h"
#include "RayTracing.h"

namespace Squid {
namespace RHI {

    enum class AdapterType { INTEGRATED, DEDICATED, VIRTUAL, CPU, OTHER };

    struct AdapterLimits {};

    struct AdapterInfo {
        uint32_t id;
        std::string name;
        uint32_t vendor_id;
        AdapterType type;
    };

    class Adapter {
    public:
        AdapterInfo info;
        AdapterLimits limits;
        virtual ~Adapter() {} // <= important!
        virtual std::unique_ptr<Device> CreateDevice() = 0;
    };

    class SQUID_API Module : public Core::IModule {
    public:
        Module(Core::EngineContext *ctx) : Core::IModule(ctx) {}
        // virtual std::unique_ptr<Instance> CreateInstance() = 0;
        virtual std::vector<std::unique_ptr<Adapter>> EnumerateAdapters() = 0;
    };

} // namespace RHI
} // namespace Squid
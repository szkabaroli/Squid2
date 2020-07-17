#pragma once

#include "Device.h"
#include "Raw.h"
#include <RHI/Module.h>
#include <Core/Modules/ModuleManager.h>
#include <Core/Modules/IModule.h>

namespace Squid {
namespace RHI {

    using Core::EngineContext;

    struct QueueFamily {
        uint32_t index;
        VkQueueFamilyProperties props;
    };

    class VulkanAdapter : public Adapter {
        friend class VulkanModule;

    public:
        std::unique_ptr<Device> CreateDevice();

    private:
        std::tuple<uint32_t, uint32_t, uint32_t> find_queue_families();

        std::vector<QueueFamily> queue_families;
        VkPhysicalDevice physical_device;
        std::shared_ptr<RawInstance> raw_instance;
    };

    const std::vector<const char *> validation_layers = {
        "VK_LAYER_KHRONOS_validation",
        // "VK_LAYER_LUNARG_api_dump",
        "VK_LAYER_LUNARG_monitor",
        // "VK_LAYER_MESA_overlay",
        // "VK_LAYER_RENDERDOC_Capture"
    };

    const std::vector<const char *> extensions = {"VK_KHR_win32_surface", "VK_EXT_debug_utils", "VK_EXT_debug_report",
                                                  "VK_KHR_surface"};

    class VulkanModule : public Module {
    public:
        VulkanModule(EngineContext *ctx) : Module(ctx) {}
        bool Initialize() override;
        std::vector<std::unique_ptr<Adapter>> EnumerateAdapters();
    private:
        std::shared_ptr<RawInstance> raw_instance;
    };

} // namespace RHI
} // namespace Squid

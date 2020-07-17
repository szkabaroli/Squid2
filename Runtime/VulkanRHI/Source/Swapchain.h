#pragma once
#include "Raw.h"
#include "RenderTarget.h"
#include "CommandAllocator.h"
#include <pch.h>

#include <Core/RingBuffer.h>

namespace Squid {
namespace RHI {

    // TODO: move this constants to a global place
    // BACKBUFFER_COUNT must be larger than 1
    static constexpr uint32_t BACKBUFFER_COUNT = 3;
    static constexpr uint32_t COMMANDLIST_COUNT = 16;

    struct FrameResources {
        VkCommandPool cmd_pools[COMMANDLIST_COUNT];
        VkCommandBuffer cmd_buffers[COMMANDLIST_COUNT];

        VkFence fence;
        VkSemaphore acquire_sema;
        VkSemaphore present_sema;
    };

    struct SwapchainContext {
        // Queue selected for persentation
        uint32_t present_queue_family;

        // Frame informations
        uint32_t current_frame = 0;
        uint32_t current_drawable = 0;
        uint32_t buffers_in_flight = 0;

        // Thread safe cmd list managers
        std::atomic<uint8_t> commandlist_count;
        Core::RingBuffer<CommandList, COMMANDLIST_COUNT> free_commandlists;
        Core::RingBuffer<CommandList, COMMANDLIST_COUNT> active_commandlists;

        // Per swapchain image resources
        FrameResources frame_resources[BACKBUFFER_COUNT];
    };

    class VulkanSwapchain {
        friend class VulkanDevice;

    public:
        VulkanSwapchain(void *win, std::shared_ptr<RawInstance> instance, std::shared_ptr<RawDevice> device);
        ~VulkanSwapchain();

        uint32_t GetPresentFamily(std::tuple<uint32_t, uint32_t, uint32_t> queue_families);
        std::unique_ptr<VulkanRenderTarget> GetRenderTarget();

        void AcquireImage(SwapchainContext &context);
        void Present(VkQueue queue, SwapchainContext &context);

        void Recreate(void *win);

    private:
        void Create(void *win);
        void Cleanup();

        VkSurfaceKHR surface = VK_NULL_HANDLE;
        VkSwapchainKHR swapchain = VK_NULL_HANDLE;

        VkSurfaceCapabilitiesKHR surface_capabilities;
        VkSurfaceFormatKHR surface_format;
        VkExtent2D surface_extent;

        VkFormat swapchain_format;

        uint32_t surface_formats_allocated_count;
        uint32_t surface_formats_count;
        uint32_t swapchain_desired_image_count;

        std::shared_ptr<RawDevice> raw_device;
        std::shared_ptr<RawInstance> raw_instance;
    };

} // namespace RHI
} // namespace Squid

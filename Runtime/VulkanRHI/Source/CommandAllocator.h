#pragma once
#include "Raw.h"
#include <pch.h>

namespace Squid {
namespace RHI {

    class VulkanCommandAllocator {
    public:
        VulkanCommandAllocator(uint32_t queue_family, std::shared_ptr<RawDevice> raw_device);
        ~VulkanCommandAllocator();

        inline VkCommandBuffer Allocate() { return this->AllocateBuffers(1)[0]; }

        std::vector<VkCommandBuffer> AllocateBuffers(uint32_t count) {
            std::vector<VkCommandBuffer> cmd_buffers(count);

            VkCommandBufferAllocateInfo alloc_info = {};
            alloc_info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
            alloc_info.commandPool = pool;
            alloc_info.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
            alloc_info.commandBufferCount = count;

            vkAllocateCommandBuffers(raw_device->device, &alloc_info, cmd_buffers.data());
            return cmd_buffers;
        }

        inline void Reset() { vkResetCommandPool(raw_device->device, pool, 0); }

    private:
        VkCommandPool pool;
        std::shared_ptr<RawDevice> raw_device;
    };

} // namespace RHI
} // namespace Squid

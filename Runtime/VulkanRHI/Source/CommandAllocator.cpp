#include "CommandAllocator.h"

namespace Squid {
namespace RHI {

    VulkanCommandAllocator::VulkanCommandAllocator(uint32_t queue_family, std::shared_ptr<RawDevice> raw_device)
        : raw_device(raw_device) {

        VkCommandPoolCreateInfo create_info = {};
        create_info.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
        create_info.queueFamilyIndex = queue_family;
        create_info.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;

        vkCreateCommandPool(raw_device->device, &create_info, nullptr, &pool);
    }

    VulkanCommandAllocator::~VulkanCommandAllocator() {
        vkResetCommandPool(raw_device->device, pool, 0);

        vkDestroyCommandPool(raw_device->device, pool, nullptr);
    }

} // namespace RHI
} // namespace Squid
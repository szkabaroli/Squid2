#include "Buffer.h"

namespace Squid {
namespace RHI {

    VulkanBuffer::VulkanBuffer(
        const BufferHandle &handle,
        std::tuple<uint32_t, uint32_t, uint32_t> queue_families,
        std::shared_ptr<RawDevice> device)
        : raw_device(device) {
        VkBufferCreateInfo create_info = {};
        create_info.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
        create_info.size = handle.size;
        create_info.usage = 0;
        size = handle.size;

        auto [gfx, compute, dma] = queue_families;
        std::set<uint32_t> unique_queues;

        if (handle.usage & BufferHandle::Usage::VERTEX_BUFFER) {
            create_info.usage |= VK_BUFFER_USAGE_VERTEX_BUFFER_BIT;
            unique_queues.insert(gfx);
        }

        if (handle.usage & BufferHandle::Usage::TRANSFER_SRC) {
            create_info.usage |= VK_BUFFER_USAGE_TRANSFER_SRC_BIT;
            unique_queues.insert(dma);
        }

        if (handle.usage & BufferHandle::Usage::TRANSFER_DST) {
            create_info.usage |= VK_BUFFER_USAGE_TRANSFER_DST_BIT;
            unique_queues.insert(dma);
        }

        if (handle.usage & BufferHandle::Usage::INDEX_BUFFER) {
            create_info.usage |= VK_BUFFER_USAGE_INDEX_BUFFER_BIT;
            unique_queues.insert(gfx);
        }

        if (handle.usage & BufferHandle::Usage::STORAGE_BUFFER) {
            create_info.usage |= VK_BUFFER_USAGE_STORAGE_BUFFER_BIT;
            unique_queues.insert(compute);
            unique_queues.insert(gfx);
        }

        if (handle.usage & BufferHandle::Usage::UNIFORM_BUFFER) {
            create_info.usage |= VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT;
            unique_queues.insert(compute);
            unique_queues.insert(gfx);
        }

        std::vector<uint32_t> queues(unique_queues.begin(), unique_queues.end());

        if (unique_queues.size() == 1) {
            create_info.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
        } else {
            create_info.sharingMode = VK_SHARING_MODE_CONCURRENT;
            create_info.queueFamilyIndexCount = static_cast<uint32_t>(unique_queues.size());
            create_info.pQueueFamilyIndices = (uint32_t *)queues.data();
        }

        VmaAllocationCreateInfo alloc_info = {};

        if (handle.cpu_access) {
            alloc_info.usage = VMA_MEMORY_USAGE_CPU_TO_GPU;
        } else {
            alloc_info.usage = VMA_MEMORY_USAGE_GPU_ONLY;
            alloc_info.requiredFlags = VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT;
        }

        vmaCreateBuffer(raw_device->allocator, &create_info, &alloc_info, &buffer, &allocation, nullptr);
    }

    VulkanBuffer::~VulkanBuffer() {
        LOG("destroying buffer and free memory");
        vmaDestroyBuffer(raw_device->allocator, buffer, allocation);
    }

} // namespace RHI
} // namespace Squid

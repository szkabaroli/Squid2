#pragma once
#include "Raw.h"
#include <pch.h>

namespace Squid { namespace RHI {

    class VulkanBuffer {
    public:
        VulkanBuffer(
            const BufferHandle &handle,
            std::tuple<uint32_t, uint32_t, uint32_t> queue_families,
            std::shared_ptr<RawDevice> device);
        ~VulkanBuffer();
        inline VkBuffer GetBuffer() const { return buffer; }
        inline VkDeviceSize GetSize() const { return size; }
        inline VmaAllocation GetAllocation() const { return allocation; }

    private:
        std::shared_ptr<RawDevice> raw_device;
        uint64_t size;
        VkBuffer buffer;
        VmaAllocation allocation;
    };

}} // namespace Squid::RHI

#pragma once
#include "Raw.h"
#include <pch.h>

namespace Squid {
namespace RHI {

    class VulkanRenderTarget {
        friend class VulkanSwapchain;

    public:
        VulkanRenderTarget(std::shared_ptr<RawDevice> raw, VkFormat format);
        VulkanRenderTarget(std::shared_ptr<RawDevice> raw, std::vector<VkImage> images, VkFormat format);
        ~VulkanRenderTarget();

        std::vector<VkImageView> GetAttachments(uint32_t index);
        inline std::vector<VkImageView> GetAttachments() { return this->GetAttachments(0); };

        inline uint32_t GetWidth() const { return width; }
        inline uint32_t GetHeight() const { return height; }

    private:
        void Create(std::vector<VkImage> &images, VkFormat format);
        void Cleanup();

        bool offscreen = true;
        uint32_t image_count = 0;
        std::vector<VkImageView> image_views;
        std::vector<VkImage> images;
        std::shared_ptr<RawDevice> raw_device;

        uint32_t width = 0;
        uint32_t height = 0;
    };

} // namespace RHI
} // namespace Squid

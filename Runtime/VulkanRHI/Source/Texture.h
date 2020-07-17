#pragma once
#include "Raw.h"
#include <pch.h>

namespace Squid {
namespace RHI {

    class VulkanTexture {
        friend class VulkanDevice;

    public:
        VulkanTexture(const TextureHandle &handle, std::shared_ptr<RawDevice> raw);
        ~VulkanTexture();
        inline VkImage GetImage() const { return image; };
        inline VkImageView GetView() const { return srv; };
        inline VkSampler GetSampler() const { return sampler; };

    private:
        int CreateSubresource(
            const TextureHandle &handle,
            ResourceView type,
            uint32_t first_slice,
            uint32_t slice_count,
            uint32_t first_mip,
            uint32_t mip_count);

        // Resource
        VmaAllocation allocation;
        VkImage image;

        // Resource views
        VkImageView rtv = VK_NULL_HANDLE;
        VkImageView srv = VK_NULL_HANDLE;
        VkImageView uav = VK_NULL_HANDLE;
        VkImageView dsv = VK_NULL_HANDLE;

        std::vector<VkImageView> subresources_srv;
		std::vector<VkImageView> subresources_uav;
		std::vector<VkImageView> subresources_rtv;
		std::vector<VkImageView> subresources_dsv;

        VkSampler sampler;

        std::shared_ptr<RawDevice> raw_device;
    };

} // namespace RHI
} // namespace Squid

#include "Texture.h"

namespace Squid {
namespace RHI {

    VulkanTexture::VulkanTexture(const TextureHandle &handle, std::shared_ptr<RawDevice> raw_device)
        : raw_device(raw_device) {

        auto format = ConvertFormat(handle.format);

        VkImageCreateInfo image_info = {};
        image_info.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
        image_info.flags = 0;
        image_info.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
        image_info.mipLevels = handle.mip_levels;
        image_info.arrayLayers = handle.layers;
        image_info.samples = (VkSampleCountFlagBits)handle.sample_count;
        image_info.initialLayout = ConvertImageLayout(handle.layout);
        image_info.tiling = VK_IMAGE_TILING_OPTIMAL;
        image_info.imageType = VK_IMAGE_TYPE_2D;
        image_info.format = format;

        // Dimension
        image_info.extent.height = handle.height;
        image_info.extent.width = handle.width;
        image_info.extent.depth = handle.depth;

        switch (handle.type) {
        case TextureHandle::Type::TEXTURE_1D:
            image_info.imageType = VK_IMAGE_TYPE_1D;
            break;
        case TextureHandle::Type::TEXTURE_2D:
            image_info.imageType = VK_IMAGE_TYPE_2D;
            break;
        case TextureHandle::Type::TEXTURE_3D:
            image_info.imageType = VK_IMAGE_TYPE_3D;
            break;
        case TextureHandle::Type::TEXTURE_CUBE:
            image_info.imageType = VK_IMAGE_TYPE_2D;
            image_info.arrayLayers = 6;
            image_info.flags |= VK_IMAGE_CREATE_CUBE_COMPATIBLE_BIT;
            break;
        default:
            assert(0);
            break;
        }

        VmaAllocationCreateInfo alloc_info = {};

        image_info.usage = 0;

        // SRV
        if (handle.usage_flags & TextureHandle::SHADER_RESOURCE_VIEW) {
            image_info.usage |= VK_IMAGE_USAGE_SAMPLED_BIT;
        }

        // UAV
        if (handle.usage_flags & TextureHandle::UNORDERED_ACCESS_VIEW) {
            image_info.usage |= VK_IMAGE_USAGE_STORAGE_BIT;
        }

        // RTV
        if (handle.usage_flags & TextureHandle::RENDER_TARGET_VIEW) {
            image_info.usage |= VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
            alloc_info.flags |= VMA_ALLOCATION_CREATE_DEDICATED_MEMORY_BIT;
        }

        // DSV
        if (handle.usage_flags & TextureHandle::DEPTH_STENCIL_VIEW) {
            image_info.usage |= VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT;
            alloc_info.flags |= VMA_ALLOCATION_CREATE_DEDICATED_MEMORY_BIT;
        }

        image_info.usage |= VK_IMAGE_USAGE_TRANSFER_SRC_BIT;
        image_info.usage |= VK_IMAGE_USAGE_TRANSFER_DST_BIT;

        alloc_info.usage = VMA_MEMORY_USAGE_GPU_ONLY;
        alloc_info.requiredFlags = VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT;

        VkResult res = vmaCreateImage(raw_device->allocator, &image_info, &alloc_info, &image, &allocation, nullptr);

        /*VkImageViewCreateInfo view_info = {};
        view_info.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
        view_info.image = image;
        view_info.viewType = VK_IMAGE_VIEW_TYPE_2D;
        view_info.format = format;
        view_info.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
        view_info.subresourceRange.baseMipLevel = 0;
        view_info.subresourceRange.levelCount = 1;
        view_info.subresourceRange.baseArrayLayer = 0;
        view_info.subresourceRange.layerCount = 1;

        vkCreateImageView(raw_device->device, &view_info, nullptr, &view);*/

        VkSamplerCreateInfo sampler_info = {};
        sampler_info.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
        sampler_info.magFilter = VK_FILTER_LINEAR;
        sampler_info.minFilter = VK_FILTER_LINEAR;
        sampler_info.addressModeU = VK_SAMPLER_ADDRESS_MODE_REPEAT;
        sampler_info.addressModeV = VK_SAMPLER_ADDRESS_MODE_REPEAT;
        sampler_info.addressModeW = VK_SAMPLER_ADDRESS_MODE_REPEAT;
        sampler_info.anisotropyEnable = VK_FALSE; // TODO
        sampler_info.maxAnisotropy = 16;
        sampler_info.borderColor = VK_BORDER_COLOR_INT_OPAQUE_BLACK;
        sampler_info.unnormalizedCoordinates = VK_FALSE;
        sampler_info.compareEnable = VK_FALSE;
        sampler_info.compareOp = VK_COMPARE_OP_ALWAYS;
        sampler_info.mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR;
        sampler_info.mipLodBias = 0.0f;
        sampler_info.minLod = 0.0f;
        sampler_info.maxLod = 0.0f;

        vkCreateSampler(raw_device->device, &sampler_info, nullptr, &sampler);

        if (handle.usage_flags & TextureHandle::RENDER_TARGET_VIEW)
            CreateSubresource(handle, ResourceView::RTV, 0, -1, 0, -1);

        if (handle.usage_flags & TextureHandle::DEPTH_STENCIL_VIEW)
            CreateSubresource(handle, ResourceView::DSV, 0, -1, 0, -1);

        if (handle.usage_flags & TextureHandle::SHADER_RESOURCE_VIEW)
            CreateSubresource(handle, ResourceView::SRV, 0, -1, 0, -1);

        if (handle.usage_flags & TextureHandle::UNORDERED_ACCESS_VIEW)
            CreateSubresource(handle, ResourceView::UAV, 0, -1, 0, -1);
    }

    int VulkanTexture::CreateSubresource(
        const TextureHandle &handle,
        ResourceView type,
        uint32_t first_slice,
        uint32_t slice_count,
        uint32_t first_mip,
        uint32_t mip_count) {

        VkImageViewCreateInfo view_desc = {};
        view_desc.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
        view_desc.flags = 0;
        view_desc.image = image;
        view_desc.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
        view_desc.subresourceRange.baseArrayLayer = first_slice;
        view_desc.subresourceRange.layerCount = slice_count;
        view_desc.subresourceRange.baseMipLevel = first_mip;
        view_desc.subresourceRange.levelCount = mip_count;
        view_desc.format = ConvertFormat(handle.format);

        // 1D
        if (handle.type == TextureHandle::Type::TEXTURE_1D) {
            if (handle.layers > 1) {
                view_desc.viewType = VK_IMAGE_VIEW_TYPE_1D_ARRAY;
            } else {
                view_desc.viewType = VK_IMAGE_VIEW_TYPE_1D;
            }
        }

        // 2D
        if (handle.type == TextureHandle::Type::TEXTURE_2D) {
            if (handle.layers > 1) {
                view_desc.viewType = VK_IMAGE_VIEW_TYPE_2D_ARRAY;
            } else {
                view_desc.viewType = VK_IMAGE_VIEW_TYPE_2D;
            }
        }

        // 3D
        if (handle.type == TextureHandle::Type::TEXTURE_3D) {
            view_desc.viewType = VK_IMAGE_VIEW_TYPE_3D;
        }

        // Cube
        if (handle.type == TextureHandle::Type::TEXTURE_CUBE) {
            view_desc.viewType = VK_IMAGE_VIEW_TYPE_CUBE;
        }

        VkResult res;

        // Resource view creation
        switch (type) {
        case ResourceView::SRV:
            switch (handle.format) {
            case FORMAT_R16_TYPELESS:
                view_desc.format = VK_FORMAT_D16_UNORM;
                view_desc.subresourceRange.aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT;
                break;
            case FORMAT_R32_TYPELESS:
                view_desc.format = VK_FORMAT_D32_SFLOAT;
                view_desc.subresourceRange.aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT;
                break;
            case FORMAT_R24G8_TYPELESS:
                view_desc.format = VK_FORMAT_D24_UNORM_S8_UINT;
                view_desc.subresourceRange.aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT;
                break;
            case FORMAT_R32G8X24_TYPELESS:
                view_desc.format = VK_FORMAT_D32_SFLOAT_S8_UINT;
                view_desc.subresourceRange.aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT;
                break;
            }

            VkImageView srv;
            res = vkCreateImageView(raw_device->device, &view_desc, nullptr, &srv);

            if (res == VK_SUCCESS) {
                if (this->srv == VK_NULL_HANDLE) {
                    this->srv = srv;
                    return -1;
                }
                this->subresources_srv.push_back(srv);
                return int(this->subresources_srv.size() - 1);
            } else {
                assert(0);
            }

            break;
        case ResourceView::UAV: {
            VkImageView uav;
            res = vkCreateImageView(raw_device->device, &view_desc, nullptr, &uav);

            if (res == VK_SUCCESS) {
                if (this->uav == VK_NULL_HANDLE) {
                    this->uav = uav;
                    return -1;
                }

                this->subresources_uav.push_back(uav);
                return int(this->subresources_uav.size() - 1);
            } else {
                assert(0);
            }
        } break;
        case ResourceView::RTV:
            VkImageView rtv;
            res = vkCreateImageView(raw_device->device, &view_desc, nullptr, &rtv);

            if (res == VK_SUCCESS) {
                if (this->rtv == VK_NULL_HANDLE) {
                    this->rtv = rtv;
                    return -1;
                }
                this->subresources_rtv.push_back(rtv);
                return int(this->subresources_rtv.size() - 1);
            } else {
                assert(0);
            }

            break;
        case ResourceView::DSV:
            view_desc.subresourceRange.aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT;

            switch (handle.format) {
            case FORMAT_R16_TYPELESS:
                view_desc.format = VK_FORMAT_D16_UNORM;
                break;
            case FORMAT_R32_TYPELESS:
                view_desc.format = VK_FORMAT_D32_SFLOAT;
                break;
            case FORMAT_R24G8_TYPELESS:
                view_desc.format = VK_FORMAT_D24_UNORM_S8_UINT;
                view_desc.subresourceRange.aspectMask |= VK_IMAGE_ASPECT_STENCIL_BIT;
                break;
            case FORMAT_R32G8X24_TYPELESS:
                view_desc.format = VK_FORMAT_D32_SFLOAT_S8_UINT;
                view_desc.subresourceRange.aspectMask |= VK_IMAGE_ASPECT_STENCIL_BIT;
                break;
            }

            VkImageView dsv;
            res = vkCreateImageView(raw_device->device, &view_desc, nullptr, &dsv);

            if (res == VK_SUCCESS) {
                if (this->dsv == VK_NULL_HANDLE) {
                    this->dsv = dsv;
                    return -1;
                }
                this->subresources_dsv.push_back(dsv);
                return int(this->subresources_dsv.size() - 1);
            } else {
                assert(0);
            }

            break;
        default:
            break;
        }
    }

    VulkanTexture::~VulkanTexture() {
        LOG("destroying image and resource views")
        vkDestroySampler(raw_device->device, sampler, nullptr);

        if (srv)
            vkDestroyImageView(raw_device->device, srv, nullptr);
        if (uav)
            vkDestroyImageView(raw_device->device, uav, nullptr);
        if (srv)
            vkDestroyImageView(raw_device->device, rtv, nullptr);
        if (uav)
            vkDestroyImageView(raw_device->device, dsv, nullptr);

        for (auto x : subresources_srv) {
            vkDestroyImageView(raw_device->device, x, nullptr);
        }

        for (auto x : subresources_uav) {
            vkDestroyImageView(raw_device->device, x, nullptr);
        }

        for (auto x : subresources_rtv) {
            vkDestroyImageView(raw_device->device, x, nullptr);
        }

        for (auto x : subresources_dsv) {
            vkDestroyImageView(raw_device->device, x, nullptr);
        }

        vmaDestroyImage(raw_device->allocator, image, allocation);
    }

} // namespace RHI
} // namespace Squid
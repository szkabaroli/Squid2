#include "RenderTarget.h"

namespace Squid {
namespace RHI {

    VulkanRenderTarget::VulkanRenderTarget(std::shared_ptr<RawDevice> raw_device, VkFormat format)
        : raw_device(raw_device){
              // TODO
          };

    VulkanRenderTarget::VulkanRenderTarget(
        std::shared_ptr<RawDevice> raw_device, std::vector<VkImage> images, VkFormat format)
        : raw_device(raw_device) {
        this->Create(images, format);
    };

    VulkanRenderTarget::~VulkanRenderTarget() { this->Cleanup(); };

    std::vector<VkImageView> VulkanRenderTarget::GetAttachments(uint32_t index) {
        std::vector<VkImageView> attachments = {image_views[index]};
        return attachments;
    };

    void VulkanRenderTarget::Create(std::vector<VkImage> &images, VkFormat format) {
        size_t image_count = images.size();

        VkImageViewCreateInfo view_info = {};
        view_info.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
        view_info.viewType = VK_IMAGE_VIEW_TYPE_2D;
        view_info.format = format;
        view_info.components.r = VK_COMPONENT_SWIZZLE_IDENTITY;
        view_info.components.g = VK_COMPONENT_SWIZZLE_IDENTITY;
        view_info.components.b = VK_COMPONENT_SWIZZLE_IDENTITY;
        view_info.components.a = VK_COMPONENT_SWIZZLE_IDENTITY;
        view_info.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
        view_info.subresourceRange.baseMipLevel = 0;
        view_info.subresourceRange.levelCount = 1;
        view_info.subresourceRange.baseArrayLayer = 0;
        view_info.subresourceRange.layerCount = 1;

        image_views.resize(image_count);
        for (size_t i = 0; i < image_count; i++) {
            view_info.image = images[i];
            vkCreateImageView(raw_device->device, &view_info, nullptr, &image_views[i]);
        }
    }

    void VulkanRenderTarget::Cleanup() {
        for (auto image_view : image_views) {
            vkDestroyImageView(raw_device->device, image_view, nullptr);
        }
    }

} // namespace RHI
} // namespace Squid

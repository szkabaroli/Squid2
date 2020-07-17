#include "Swapchain.h"

namespace Squid {
namespace RHI {

    VulkanSwapchain::VulkanSwapchain(
        void *win, std::shared_ptr<RawInstance> instance, std::shared_ptr<RawDevice> device)
        : raw_instance(instance), raw_device(device) {

        SDL_Vulkan_CreateSurface((SDL_Window *)win, raw_instance->instance, &surface);

        // Surface capabilities
        vkGetPhysicalDeviceSurfaceCapabilitiesKHR(raw_device->physical, surface, &surface_capabilities);

        // SUrface formats
        uint32_t formats_count = 0;
        vkGetPhysicalDeviceSurfaceFormatsKHR(raw_device->physical, surface, &formats_count, nullptr);

        std::vector<VkSurfaceFormatKHR> formats(formats_count);
        vkGetPhysicalDeviceSurfaceFormatsKHR(raw_device->physical, surface, &formats_count, formats.data());

        // EXP: Image Count 

      

        // Image count
        //swapchain_desired_image_count = surface_capabilities.minImageCount + 1;
        //if (swapchain_desired_image_count > surface_capabilities.maxImageCount &&
        //    surface_capabilities.maxImageCount > 0)
        //    swapchain_desired_image_count = surface_capabilities.maxImageCount;

        // Pick the best format
        if (formats_count == 1 && formats[0].format == VK_FORMAT_UNDEFINED) {
            // aren't any preferred formats, so we pick
            surface_format.colorSpace = VK_COLORSPACE_SRGB_NONLINEAR_KHR;
            surface_format.format = VK_FORMAT_R8G8B8A8_UNORM;
        } else {
            surface_format = formats[0];
            for (const auto &format : formats) {
                if (format.format == VK_FORMAT_R8G8B8A8_UNORM) {
                    surface_format = format;
                    break;
                }
            }
        }

        this->Create(win);
    }

    VulkanSwapchain::~VulkanSwapchain() {
        this->Cleanup();
        vkDestroySurfaceKHR(raw_instance->instance, surface, nullptr);
    }

    void VulkanSwapchain::Create(void *win) {
        vkDeviceWaitIdle(raw_device->device);
        VkSwapchainKHR old_swapchain = this->swapchain;
        // Extent
        int w, h;
        SDL_Vulkan_GetDrawableSize((SDL_Window *)win, &w, &h);
        surface_extent.width = w;
        surface_extent.height = h;

        std::array<uint32_t, 1> queue_families = {0};

        swapchain_format = surface_format.format;

        uint32_t image_count = BACKBUFFER_COUNT;

        VkSwapchainCreateInfoKHR create_info = {};
        create_info.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
        create_info.flags = 0;
        create_info.surface = surface;
        create_info.queueFamilyIndexCount = 0;
        create_info.pQueueFamilyIndices = nullptr;
        create_info.minImageCount = image_count;
        create_info.imageFormat = surface_format.format;
        create_info.imageColorSpace = surface_format.colorSpace;
        create_info.imageArrayLayers = 1;
        create_info.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT;
        create_info.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
        create_info.imageExtent = surface_extent;
        create_info.preTransform = surface_capabilities.currentTransform;
        create_info.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
        // TODO: select based on available modes
        create_info.presentMode = VK_PRESENT_MODE_FIFO_KHR;
        create_info.clipped = VK_TRUE;
        create_info.oldSwapchain = VK_NULL_HANDLE;

        vkCreateSwapchainKHR(raw_device->device, &create_info, nullptr, &swapchain);
        
    };

    void VulkanSwapchain::Cleanup() {
        LOG("destroy swapchain and images")
        vkDestroySwapchainKHR(raw_device->device, swapchain, nullptr);
    };

    void VulkanSwapchain::Recreate(void *win) {
        this->Cleanup();
        this->Create(win);
    }

    uint32_t VulkanSwapchain::GetPresentFamily(std::tuple<uint32_t, uint32_t, uint32_t> queue_families) {
        auto [gfx, compute, transfer] = queue_families;

        VkBool32 presentSupport = false;

        vkGetPhysicalDeviceSurfaceSupportKHR(raw_device->physical, gfx, surface, &presentSupport);
        if (presentSupport)
            return gfx;

        vkGetPhysicalDeviceSurfaceSupportKHR(raw_device->physical, compute, surface, &presentSupport);
        if (presentSupport)
            return compute;

        return transfer;
    };

    std::unique_ptr<VulkanRenderTarget> VulkanSwapchain::GetRenderTarget() {
        uint32_t image_count = 0;
        vkGetSwapchainImagesKHR(raw_device->device, swapchain, &image_count, nullptr);

        std::vector<VkImage> swapchain_images(image_count);
        vkGetSwapchainImagesKHR(raw_device->device, swapchain, &image_count, swapchain_images.data());

        assert(image_count == BACKBUFFER_COUNT);

        auto backbuffer =
            std::make_unique<VulkanRenderTarget>(raw_device, std::move(swapchain_images), swapchain_format);

        backbuffer->height = surface_extent.height;
        backbuffer->width = surface_extent.width;

        return std::move(backbuffer);
    };

    void VulkanSwapchain::AcquireImage(SwapchainContext &context) {
        vkAcquireNextImageKHR(
            raw_device->device, swapchain, UINT64_MAX, context.frame_resources[context.current_frame].acquire_sema,
            VK_NULL_HANDLE, &context.current_drawable);
    };

    void VulkanSwapchain::Present(VkQueue present_queue, SwapchainContext &context) {

        uint32_t next_frame = (context.current_frame + 1) % BACKBUFFER_COUNT;

        VkPresentInfoKHR present_info = {};
        present_info.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
        present_info.waitSemaphoreCount = 1;
        present_info.pWaitSemaphores = &context.frame_resources[context.current_frame].present_sema;

        VkSwapchainKHR swap_chains[] = {swapchain};
        present_info.swapchainCount = 1;
        present_info.pSwapchains = swap_chains;
        present_info.pImageIndices = &context.current_drawable;
        present_info.pResults = nullptr; // Optional

        vkQueuePresentKHR(present_queue, &present_info);

        // Stall the CPU if the GPU is to busy
        ++context.buffers_in_flight;

        if (context.buffers_in_flight == BACKBUFFER_COUNT) {
            vkWaitForFences(raw_device->device, 1, &context.frame_resources[next_frame].fence, VK_TRUE, UINT64_MAX);
            vkResetFences(raw_device->device, 1, &context.frame_resources[next_frame].fence);
            // TODO reset the pool instead
            //vkResetCommandBuffer(context.frame_resources[next_frame].cmd_buffers[0], 0);
            --context.buffers_in_flight;
        }

        context.current_frame = next_frame;
    };

} // namespace RHI
} // namespace Squid

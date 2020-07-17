#include "FboCache.h"

namespace Squid {
namespace RHI {

    bool VulkanFboCache::RenderPassEq::operator()(const RenderPassKey &k1, const RenderPassKey &k2) const {
        return k1.final_color_layout == k2.final_color_layout && k1.final_depth_layout == k2.final_depth_layout &&
               k1.color_format == k2.color_format && k1.depth_format == k2.depth_format && k1.clear == k2.clear;
    }

    bool VulkanFboCache::FboKeyEqualFn::operator()(const FboKey &k1, const FboKey &k2) const {
        static_assert(sizeof(FboKey::attachments) == 9 * sizeof(VkImageView), "Unexpected count.");
        return k1.render_pass == k2.render_pass && k1.attachments[0] == k2.attachments[0] &&
               k1.attachments[1] == k2.attachments[1] && k1.attachments[2] == k2.attachments[2] &&
               k1.attachments[3] == k2.attachments[3] && k1.attachments[4] == k2.attachments[4] &&
               k1.attachments[5] == k2.attachments[5] && k1.attachments[6] == k2.attachments[6] &&
               k1.attachments[7] == k2.attachments[7] && k1.attachments[8] == k2.attachments[8] &&
               k1.attachments[9] == k2.attachments[9] && k1.height == k2.height && k1.width == k2.width;
    }

    VulkanFboCache::VulkanFboCache(std::shared_ptr<RawDevice> raw_device) : raw_device(raw_device) {}

    VulkanFboCache::~VulkanFboCache() { this->Reset(); }

    VkFramebuffer VulkanFboCache::GetFramebuffer(const FboKey &key) {
        auto iter = framebuffer_cache.find(key);

        // use __builtin_expect
        // Cache hit
        if (iter != framebuffer_cache.end() && iter->second.handle != VK_NULL_HANDLE) {
            // iter.value().timestamp = 0;
            return iter->second.handle;
        }

        uint32_t attachment_count = 0;
        for (auto attachment : key.attachments) {
            if (attachment)
                attachment_count++;
        }

        VkFramebuffer framebuffer;
        VkFramebufferCreateInfo frame_buffer_info = {};
        frame_buffer_info.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
        frame_buffer_info.renderPass = key.render_pass;
        frame_buffer_info.attachmentCount = attachment_count;
        frame_buffer_info.pAttachments = key.attachments;
        frame_buffer_info.width = key.width;
        frame_buffer_info.height = key.height;
        frame_buffer_info.layers = 1;

        vkCreateFramebuffer(raw_device->device, &frame_buffer_info, nullptr, &framebuffer);

        framebuffer_cache[key] = {framebuffer, 0};
        render_pass_ref_count[frame_buffer_info.renderPass]++;

        return framebuffer;
    }

    VkRenderPass VulkanFboCache::GetRenderPass(RenderPassKey key) {
        auto iter = render_pass_cache.find(key);

        // use __builtin_expect
        // Cache hit
        if (iter != render_pass_cache.end() && iter->second.handle != VK_NULL_HANDLE) {
            iter.value().timestamp = 0;
            return iter->second.handle;
        }

        const bool has_color = key.color_format != VK_FORMAT_UNDEFINED;
        const bool has_depth = key.depth_format != VK_FORMAT_UNDEFINED;

        uint32_t num_attachments = 0;

        VkAttachmentReference color_attachment_ref = {};
        if (has_color) {
            color_attachment_ref.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
            color_attachment_ref.attachment = num_attachments++;
        }

        VkAttachmentReference depth_attachment_ref = {};
        if (has_depth) {
            depth_attachment_ref.layout = VK_IMAGE_LAYOUT_GENERAL;
            depth_attachment_ref.attachment = num_attachments++;
        }

        VkSubpassDescription subpass = {};
        subpass.flags = 0;
        subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
        subpass.colorAttachmentCount = has_color ? 1u : 0u;
        subpass.pColorAttachments = has_color ? &color_attachment_ref : nullptr;
        subpass.pDepthStencilAttachment = has_depth ? &depth_attachment_ref : nullptr;

        VkAttachmentDescription color_attachment = {};
        color_attachment.format = key.color_format;
        color_attachment.samples = VK_SAMPLE_COUNT_1_BIT;
        color_attachment.loadOp =
            (key.clear & ClearFlags::ColorClear) ? VK_ATTACHMENT_LOAD_OP_CLEAR : VK_ATTACHMENT_LOAD_OP_DONT_CARE;
        color_attachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
        color_attachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
        color_attachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
        color_attachment.finalLayout = key.final_color_layout;

        VkAttachmentDescription depth_attachment = {};
        depth_attachment.format = key.depth_format;
        depth_attachment.samples = VK_SAMPLE_COUNT_1_BIT;
        depth_attachment.loadOp =
            (key.clear & ClearFlags::DepthClear) ? VK_ATTACHMENT_LOAD_OP_CLEAR : VK_ATTACHMENT_LOAD_OP_DONT_CARE;
        depth_attachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
        depth_attachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
        depth_attachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
        depth_attachment.finalLayout = key.final_depth_layout;

        std::vector<VkAttachmentDescription> attachments;
        if (has_color)
            attachments.push_back(color_attachment);
        if (has_depth)
            attachments.push_back(depth_attachment);

        VkRenderPassCreateInfo render_pass_info = {};
        render_pass_info.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
        render_pass_info.attachmentCount = static_cast<uint32_t>(attachments.size());
        render_pass_info.pAttachments = attachments.data();
        render_pass_info.subpassCount = 1;
        render_pass_info.pSubpasses = &subpass;

        // TODO
        VkSubpassDependency dependency = {};
        dependency.srcSubpass = VK_SUBPASS_EXTERNAL;
        dependency.dstSubpass = 0;
        dependency.srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
        dependency.srcAccessMask = 0;
        dependency.dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
        dependency.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_READ_BIT | VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;

        render_pass_info.dependencyCount = 1;
        render_pass_info.pDependencies = &dependency;

        VkRenderPass render_pass = VK_NULL_HANDLE;

        vkCreateRenderPass(raw_device->device, &render_pass_info, nullptr, &render_pass);
        render_pass_cache[key] = {render_pass, 0};

        return render_pass;
    }

    void VulkanFboCache::Reset() {
        LOG("destroy render passes and framebuffers")
        for (auto pair : framebuffer_cache) {
            render_pass_ref_count[pair.first.render_pass]--;
            vkDestroyFramebuffer(raw_device->device, pair.second.handle, nullptr);
        }
        framebuffer_cache.clear();
        for (const auto &cached : render_pass_cache) {
            vkDestroyRenderPass(raw_device->device, cached.second.handle, nullptr);
        }
        render_pass_cache.clear();
    }

} // namespace RHI
} // namespace Squid

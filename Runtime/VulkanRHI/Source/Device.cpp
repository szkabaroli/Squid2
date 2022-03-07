#include "Device.h"

namespace Squid {
namespace RHI {

    PFN_vkSetDebugUtilsObjectNameEXT vkSetDebugUtilsObjectNameEXT;
    PFN_vkCmdBeginDebugUtilsLabelEXT vkCmdBeginDebugUtilsLabelEXT;
    PFN_vkCmdEndDebugUtilsLabelEXT vkCmdEndDebugUtilsLabelEXT;
    PFN_vkCmdInsertDebugUtilsLabelEXT vkCmdInsertDebugUtilsLabelEXT;

    VulkanDevice::VulkanDevice(
        std::tuple<uint32_t, uint32_t, uint32_t> queue_families,
        std::shared_ptr<RawInstance> raw_instance,
        VkPhysicalDevice physical)
        : id(RANDOM_64), raw_instance(raw_instance) {

        auto [gfx, compute, dma] = queue_families;

        this->gfx_queue = gfx;
        this->compute_queue = compute;
        this->transfer_queue = dma;

        LOG("Selected Queues: ")
        LOG("Graphics: {}", gfx)
        LOG("Compute: {}", compute)
        LOG("Transfer: {}", dma)

        std::set<uint32_t> used_queues = {gfx, compute, dma};

        float queue_priority = 1.0f;

        std::vector<VkDeviceQueueCreateInfo> queue_create_infos;
        for (const uint32_t queue : used_queues) {
            VkDeviceQueueCreateInfo create_info = {};
            create_info.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
            create_info.queueFamilyIndex = queue;
            create_info.queueCount = 1;
            create_info.pQueuePriorities = &queue_priority;
            queue_create_infos.push_back(create_info);
        }

        VkDeviceCreateInfo create_info = {};
        create_info.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;

        create_info.pEnabledFeatures = nullptr;
        create_info.enabledLayerCount = 0;
        create_info.ppEnabledLayerNames = nullptr;
        create_info.enabledExtensionCount = static_cast<uint32_t>(device_extensions.size());
        create_info.ppEnabledExtensionNames = device_extensions.data();

        create_info.pQueueCreateInfos = queue_create_infos.data();
        create_info.queueCreateInfoCount = static_cast<uint32_t>(queue_create_infos.size());

        this->raw_device = std::make_shared<RawDevice>();
        raw_device->physical = physical;

        vkCreateDevice(physical, &create_info, nullptr, &raw_device->device);

        // Extension functions:
        vkSetDebugUtilsObjectNameEXT =
            (PFN_vkSetDebugUtilsObjectNameEXT)vkGetDeviceProcAddr(raw_device->device, "vkSetDebugUtilsObjectNameEXT");
        vkCmdBeginDebugUtilsLabelEXT =
            (PFN_vkCmdBeginDebugUtilsLabelEXT)vkGetDeviceProcAddr(raw_device->device, "vkCmdBeginDebugUtilsLabelEXT");
        vkCmdEndDebugUtilsLabelEXT =
            (PFN_vkCmdEndDebugUtilsLabelEXT)vkGetDeviceProcAddr(raw_device->device, "vkCmdEndDebugUtilsLabelEXT");
        vkCmdInsertDebugUtilsLabelEXT =
            (PFN_vkCmdInsertDebugUtilsLabelEXT)vkGetDeviceProcAddr(raw_device->device, "vkCmdInsertDebugUtilsLabelEXT");

        // Get one queue from each selected family
        vkGetDeviceQueue(raw_device->device, gfx_queue, 0, &queues[gfx_queue]);
        vkGetDeviceQueue(raw_device->device, compute_queue, 0, &queues[compute_queue]);
        vkGetDeviceQueue(raw_device->device, compute_queue, 0, &queues[transfer_queue]);

        // Create Vma allocator
        VmaAllocatorCreateInfo allocator_info = {};
        allocator_info.physicalDevice = raw_device->physical;
        allocator_info.device = raw_device->device;

        vmaCreateAllocator(&allocator_info, &raw_device->allocator);

        fbo_cache = std::make_unique<VulkanFboCache>(raw_device);

        transfer_list_allocator = std::make_unique<VulkanCommandAllocator>(0, this->raw_device);
    }

    // == Load handles ====================================================================

    void VulkanDevice::LoadSwapchain(const SwapchainHandle &handle) {
        assert(handle.id != INVALID_HANDLE_ID);

        auto swapchain = std::make_unique<VulkanSwapchain>(handle.window_handle, raw_instance, raw_device);
        auto backbuffer = swapchain->GetRenderTarget();

        auto context = std::make_unique<SwapchainContext>();
        context->commandlist_count = {0};

        context->present_queue_family = swapchain->GetPresentFamily({gfx_queue, compute_queue, transfer_queue});

        // Create semaphores and fences
        VkSemaphoreCreateInfo semaphore_info = {};
        semaphore_info.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;

        VkFenceCreateInfo fence_info = {};
        fence_info.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
        fence_info.flags = 0;

        // TODO: destroy them

        for (auto i = 0; i < BACKBUFFER_COUNT; i++) {
            // context.cmd_allocators[i] = std::make_unique<VulkanCommandAllocator>(0, raw_device);
            vkCreateSemaphore(raw_device->device, &semaphore_info, nullptr, &context->frame_resources[i].acquire_sema);
            vkCreateSemaphore(raw_device->device, &semaphore_info, nullptr, &context->frame_resources[i].present_sema);
            vkCreateFence(raw_device->device, &fence_info, nullptr, &context->frame_resources[i].fence);
        }

        render_targets.insert(std::pair(handle.backbuffer.id, std::move(backbuffer)));
        swapchains.insert(std::pair(handle.id, std::move(swapchain)));

        // swap_contexts.insert(std::pahandle.backbuffer.id, std::move(context));

        swap_contexts.insert(std::pair(handle.backbuffer.id, std::move(context)));
    };

    void VulkanDevice::LoadRenderTarget(const RenderTargetHandle &handle) {
        assert(handle.id != INVALID_HANDLE_ID);

        auto render_target = std::make_unique<VulkanRenderTarget>(raw_device, VkFormat::VK_FORMAT_UNDEFINED);
        render_targets.insert(std::pair(handle.id, std::move(render_target)));
    };

    void VulkanDevice::LoadBuffer(const BufferHandle &handle) {
        assert(handle.id != INVALID_HANDLE_ID);
        auto queue_families = std::make_tuple(gfx_queue, compute_queue, transfer_queue);
        buffers.insert(std::pair(handle.id, std::make_unique<VulkanBuffer>(handle, queue_families, raw_device)));
    };

    void VulkanDevice::LoadTexture(const TextureHandle &handle) {
    
        assert(handle.id != INVALID_HANDLE_ID);
        textures.insert(std::pair(handle.id, std::make_unique<VulkanTexture>(handle, raw_device)));
    };

    void VulkanDevice::LoadPipeline(const ComputePipelineHandle &handle) {
        assert(handle.id != INVALID_HANDLE_ID);

        std::vector<VkDescriptorSetLayout> layouts;
        layouts.reserve(handle.descriptor_sets.size());
        for (const auto set : handle.descriptor_sets) {
            auto layout = descriptor_sets[set.id]->GetLayout();
            layouts.push_back(layout);
        }
        compute_pipelines.insert(
            std::pair(handle.id, std::make_unique<VulkanComputePipeline>(handle, layouts, raw_device)));
    };

    void VulkanDevice::LoadPipeline(const GraphicsPipelineHandle &handle) {
        assert(handle.id != INVALID_HANDLE_ID);

        VkRenderPass render_pass;
        if (handle.render_pass) {
            render_pass = render_passes[handle.render_pass->id];
        } else {
            VulkanFboCache::RenderPassKey key = {};
            key.final_color_layout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;
            key.final_depth_layout = VK_IMAGE_LAYOUT_UNDEFINED;
            key.color_format = VkFormat::VK_FORMAT_B8G8R8A8_UNORM;
            key.depth_format = VkFormat::VK_FORMAT_UNDEFINED;
            key.clear = ClearFlags::ColorClear;

            render_pass = fbo_cache->GetRenderPass(key);
        }

        std::vector<VkDescriptorSetLayout> layouts;
        layouts.reserve(handle.descriptor_sets.size());
        for (const auto set : handle.descriptor_sets) {
            auto layout = descriptor_sets[set.id]->GetLayout();
            layouts.push_back(layout);
        }

        gfx_pipelines.insert(
            std::pair(handle.id, std::make_unique<VulkanGraphicsPipeline>(handle, layouts, render_pass, raw_device)));
    };

    void VulkanDevice::LoadDescriptorSet(const DescriptorSetHandle &handle) {
        assert(handle.id != INVALID_HANDLE_ID);
        descriptor_sets.insert(std::pair(handle.id, std::make_unique<VulkanDescriptorSet>(handle, raw_device)));
    };

    void VulkanDevice::LoadRenderPass(const RenderPassHandle &handle) {
        assert(handle.id != INVALID_HANDLE_ID);

        VkResult res;

        VkImageView attachments[9] = {};
        VkAttachmentDescription attachment_descriptions[9] = {};
        VkAttachmentReference color_attachment_refs[8] = {};
        VkAttachmentReference depth_attachment_ref = {};

        VkSubpassDescription subpass = {};
        subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;

        auto StencilSupported = [](Format value) {
            switch (value) {
            case FORMAT_R32G8X24_TYPELESS:
            case FORMAT_D32_FLOAT_S8X24_UINT:
            case FORMAT_R24G8_TYPELESS:
            case FORMAT_D24_UNORM_S8_UINT:
                return true;
            }
        };

        auto ProcessAttachment = [&](u32 &valid_attachment_count, RenderPassAttachment attachment) {
            const TextureHandle *texture = attachment.texture;
            auto &t = textures[texture->id];

            attachment_descriptions[valid_attachment_count].format = ConvertFormat(texture->format);
            attachment_descriptions[valid_attachment_count].samples = (VkSampleCountFlagBits)texture->sample_count;

            // LoadOP
            switch (attachment.load_op) {
            default:
            case RenderPassAttachment::LOAD_OP_LOAD:
                attachment_descriptions[valid_attachment_count].loadOp = VK_ATTACHMENT_LOAD_OP_LOAD;
                break;
            case RenderPassAttachment::LOAD_OP_CLEAR:
                attachment_descriptions[valid_attachment_count].loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
                break;
            case RenderPassAttachment::LOAD_OP_DONTCARE:
                attachment_descriptions[valid_attachment_count].loadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
                break;
            }

            // StoreOP
            switch (attachment.store_op) {
            default:
            case RenderPassAttachment::STORE_OP_STORE:
                attachment_descriptions[valid_attachment_count].storeOp = VK_ATTACHMENT_STORE_OP_STORE;
                break;
            case RenderPassAttachment::STORE_OP_DONTCARE:
                attachment_descriptions[valid_attachment_count].storeOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
                break;
            }

            // Stencil dont care
            attachment_descriptions[valid_attachment_count].stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
            attachment_descriptions[valid_attachment_count].stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;

            attachment_descriptions[valid_attachment_count].initialLayout =
                ConvertImageLayout(attachment.initial_layout);
            attachment_descriptions[valid_attachment_count].finalLayout = ConvertImageLayout(attachment.final_layout);

            if (attachment.type == RenderPassAttachment::COLOR_ATTACHMENT) {
                attachments[valid_attachment_count] = t->rtv;

                if (attachments[valid_attachment_count] == VK_NULL_HANDLE) {
                    assert(0 && "Texture has no RTV");
                    // continue;
                }

                color_attachment_refs[subpass.colorAttachmentCount].attachment = valid_attachment_count;
                color_attachment_refs[subpass.colorAttachmentCount].layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

                subpass.colorAttachmentCount++;
                subpass.pColorAttachments = color_attachment_refs;
            }

            if (attachment.type == RenderPassAttachment::DEPTH_STENCIL_ATTACHMENT) {

                attachments[valid_attachment_count] = t->dsv;

                if (attachments[valid_attachment_count] == VK_NULL_HANDLE) {
                    assert(0 && "Texture has no DSV");
                    // continue;
                }

                if (StencilSupported(texture->format)) {
                    switch (attachment.load_op) {
                    default:
                    case RenderPassAttachment::LOAD_OP_LOAD:
                        attachment_descriptions[valid_attachment_count].stencilLoadOp = VK_ATTACHMENT_LOAD_OP_LOAD;
                        break;
                    case RenderPassAttachment::LOAD_OP_CLEAR:
                        attachment_descriptions[valid_attachment_count].stencilLoadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
                        break;
                    case RenderPassAttachment::LOAD_OP_DONTCARE:
                        attachment_descriptions[valid_attachment_count].stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
                        break;
                    }

                    switch (attachment.store_op) {
                    default:
                    case RenderPassAttachment::STORE_OP_STORE:
                        attachment_descriptions[valid_attachment_count].stencilStoreOp = VK_ATTACHMENT_STORE_OP_STORE;
                        break;
                    case RenderPassAttachment::STORE_OP_DONTCARE:
                        attachment_descriptions[valid_attachment_count].stencilStoreOp =
                            VK_ATTACHMENT_STORE_OP_DONT_CARE;
                        break;
                    }
                }

                depth_attachment_ref.attachment = valid_attachment_count;
                depth_attachment_ref.layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

                if (attachment_descriptions[valid_attachment_count].initialLayout ==
                    VK_IMAGE_LAYOUT_DEPTH_STENCIL_READ_ONLY_OPTIMAL) {
                    depth_attachment_ref.layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_READ_ONLY_OPTIMAL;
                }

                subpass.pDepthStencilAttachment = &depth_attachment_ref;
            }

            valid_attachment_count++;
            // int subresource = handle.attachments[i].subresource;
            // auto texture_internal_state = to_internal(texture);
        };

        u32 valid_attachment_count = 0;
        for (u32 i = 0; i < handle.num_attachments; ++i) {
            ProcessAttachment(valid_attachment_count, handle.color[i]);
        }

        if (handle.ds.texture) {
            ProcessAttachment(valid_attachment_count, handle.ds);
        }

        // handle.num_attachments = valid_attachment_count;

        VkRenderPassCreateInfo render_pass_info = {};
        render_pass_info.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
        render_pass_info.attachmentCount = valid_attachment_count;
        render_pass_info.pAttachments = attachment_descriptions;
        render_pass_info.subpassCount = 1;
        render_pass_info.pSubpasses = &subpass;

        VkRenderPass render_pass;
        res = vkCreateRenderPass(raw_device->device, &render_pass_info, nullptr, &render_pass);
        assert(res == VK_SUCCESS && "Failed to create RenderPass");

        render_passes.insert(std::pair(handle.id, render_pass));
    };

    // == Unload handles ====================================================================

    void VulkanDevice::UnloadSwapchain(const SwapchainHandle &handle) {
        render_targets.erase(handle.backbuffer.id);
        swapchains.erase(handle.id);
    };

    void VulkanDevice::UnloadRenderTarget(const RenderTargetHandle &handle) {
        assert(handle.id != INVALID_HANDLE_ID);
        render_targets.erase(handle.id);
    };

    void VulkanDevice::UnloadBuffer(const BufferHandle &handle) {
        assert(handle.id != INVALID_HANDLE_ID);
        buffers.erase(handle.id);
    };

    void VulkanDevice::UnloadTexture(const TextureHandle &handle) {
        assert(handle.id != INVALID_HANDLE_ID);
        textures.erase(handle.id);
    };

    void VulkanDevice::UnloadPipeline(const ComputePipelineHandle &handle) {
        assert(handle.id != INVALID_HANDLE_ID);
        compute_pipelines.erase(handle.id);
    };

    void VulkanDevice::UnloadPipeline(const GraphicsPipelineHandle &handle) {
        assert(handle.id != INVALID_HANDLE_ID);
        gfx_pipelines.erase(handle.id);
    };

    void VulkanDevice::UnloadDescriptorSet(const DescriptorSetHandle &handle) {
        assert(handle.id != INVALID_HANDLE_ID);
        descriptor_sets.erase(handle.id);
    };

    void VulkanDevice::UnloadRenderPass(const RenderPassHandle &handle) { assert(handle.id != INVALID_HANDLE_ID); };

    // == Query Handles ==========================================================

    bool VulkanDevice::HasSwapchain(const SwapchainHandle &handle) const {
        assert(handle.id != INVALID_HANDLE_ID);

        auto el = swapchains.find(handle.id);
        return el != swapchains.end();
    };

    bool VulkanDevice::HasRenderTarget(const RenderTargetHandle &handle) const {
        assert(handle.id != INVALID_HANDLE_ID);

        auto el = render_targets.find(handle.id);
        return el != render_targets.end();
    };

    bool VulkanDevice::HasBuffer(const BufferHandle &handle) const {
        assert(handle.id != INVALID_HANDLE_ID);

        auto el = buffers.find(handle.id);
        return el != buffers.end();
    };

    bool VulkanDevice::HasTexture(const TextureHandle &handle) const {
        assert(handle.id != INVALID_HANDLE_ID);

        auto el = textures.find(handle.id);
        return el != textures.end();
    };

    bool VulkanDevice::HasPipeline(const ComputePipelineHandle &handle) const {
        assert(handle.id != INVALID_HANDLE_ID);

        auto el = compute_pipelines.find(handle.id);
        return el != compute_pipelines.end();
    };

    bool VulkanDevice::HasPipeline(const GraphicsPipelineHandle &handle) const {
        assert(handle.id != INVALID_HANDLE_ID);

        auto el = gfx_pipelines.find(handle.id);
        return el != gfx_pipelines.end();
    };

    bool VulkanDevice::HasDescriptorSet(const DescriptorSetHandle &handle) const {
        assert(handle.id != INVALID_HANDLE_ID);

        auto el = descriptor_sets.find(handle.id);
        return el != descriptor_sets.end();
    };

    bool VulkanDevice::HasRenderPass(const RenderPassHandle &handle) const {
        assert(handle.id != INVALID_HANDLE_ID);
        return true;
    };

    // == Set names ==============================================================

    void VulkanDevice::SetName(const TextureHandle &handle, const std::string &name) const {
        VkResult res;

        const auto &texture = textures.at(handle.id);

        VkDebugUtilsObjectNameInfoEXT info = {};
        info.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_OBJECT_NAME_INFO_EXT;
        info.pObjectName = name.c_str();
        info.objectType = VK_OBJECT_TYPE_IMAGE;
        info.objectHandle = (uint64_t)texture->GetImage();

        res = vkSetDebugUtilsObjectNameEXT(raw_device->device, &info);
        assert(res == VK_SUCCESS);

        // Sampler

        std::string sampler_name = name;
        sampler_name = sampler_name.append(" - Sampler");

        VkDebugUtilsObjectNameInfoEXT sampler_info = {};
        sampler_info.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_OBJECT_NAME_INFO_EXT;
        sampler_info.pObjectName = sampler_name.c_str();
        sampler_info.objectType = VK_OBJECT_TYPE_SAMPLER;
        sampler_info.objectHandle = (uint64_t)texture->GetSampler();

        res = vkSetDebugUtilsObjectNameEXT(raw_device->device, &sampler_info);
        assert(res == VK_SUCCESS);

        // RTV
        if (texture->rtv != VK_NULL_HANDLE) {
            std::string rtv_name = name;
            rtv_name = rtv_name.append(" - RTV");

            VkDebugUtilsObjectNameInfoEXT sampler_info = {};
            sampler_info.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_OBJECT_NAME_INFO_EXT;
            sampler_info.pObjectName = rtv_name.c_str();
            sampler_info.objectType = VK_OBJECT_TYPE_IMAGE_VIEW;
            sampler_info.objectHandle = (uint64_t)texture->rtv;

            res = vkSetDebugUtilsObjectNameEXT(raw_device->device, &sampler_info);
            assert(res == VK_SUCCESS);
        }

        // SRV
        if (texture->srv != VK_NULL_HANDLE) {
            std::string srv_name = name;
            srv_name = srv_name.append(" - SRV");

            VkDebugUtilsObjectNameInfoEXT srv_info = {};
            srv_info.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_OBJECT_NAME_INFO_EXT;
            srv_info.pObjectName = srv_name.c_str();
            srv_info.objectType = VK_OBJECT_TYPE_IMAGE_VIEW;
            srv_info.objectHandle = (uint64_t)texture->srv;

            res = vkSetDebugUtilsObjectNameEXT(raw_device->device, &srv_info);
            assert(res == VK_SUCCESS);
        }

        // DSV
        if (texture->dsv != VK_NULL_HANDLE) {
            std::string dsv_name = name;
            dsv_name = dsv_name.append(" - DSV");

            VkDebugUtilsObjectNameInfoEXT sampler_info = {};
            sampler_info.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_OBJECT_NAME_INFO_EXT;
            sampler_info.pObjectName = dsv_name.c_str();
            sampler_info.objectType = VK_OBJECT_TYPE_IMAGE_VIEW;
            sampler_info.objectHandle = (uint64_t)texture->dsv;

            res = vkSetDebugUtilsObjectNameEXT(raw_device->device, &sampler_info);
            assert(res == VK_SUCCESS);
        }

        // UAV
        if (texture->uav != VK_NULL_HANDLE) {
            std::string uav_name = name;
            uav_name = uav_name.append(" - UAV");

            VkDebugUtilsObjectNameInfoEXT sampler_info = {};
            sampler_info.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_OBJECT_NAME_INFO_EXT;
            sampler_info.pObjectName = uav_name.c_str();
            sampler_info.objectType = VK_OBJECT_TYPE_IMAGE_VIEW;
            sampler_info.objectHandle = (uint64_t)texture->uav;

            res = vkSetDebugUtilsObjectNameEXT(raw_device->device, &sampler_info);
            assert(res == VK_SUCCESS);
        }
    }

    void VulkanDevice::SetName(const BufferHandle &handle, const std::string &name) const {
        const auto &buffer = buffers.at(handle.id);

        VkDebugUtilsObjectNameInfoEXT info = {};
        info.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_OBJECT_NAME_INFO_EXT;
        info.pObjectName = name.c_str();
        info.objectType = VK_OBJECT_TYPE_BUFFER;
        info.objectHandle = (uint64_t)buffer->GetBuffer();

        VkResult res = vkSetDebugUtilsObjectNameEXT(raw_device->device, &info);
        assert(res == VK_SUCCESS);
    }

    void VulkanDevice::SetName(const RenderPassHandle &handle, const std::string &name) const {
        const auto *render_pass = render_passes.at(handle.id);

        VkDebugUtilsObjectNameInfoEXT info = {};
        info.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_OBJECT_NAME_INFO_EXT;
        info.pObjectName = name.c_str();
        info.objectType = VK_OBJECT_TYPE_RENDER_PASS;
        info.objectHandle = (uint64_t)render_pass;

        VkResult res = vkSetDebugUtilsObjectNameEXT(raw_device->device, &info);
        assert(res == VK_SUCCESS);
    }

    void VulkanDevice::BeginFrameEXP(const SwapchainHandle &handle) {
        assert(this->HasSwapchain(handle));

        swapchains[handle.id]->AcquireImage(*swap_contexts[handle.backbuffer.id].get());
        current_backbuffer_id = static_cast<uint64_t>(handle.backbuffer.id);
    };

    void VulkanDevice::EndFrameEXP(const SwapchainHandle &handle) {
        assert(current_backbuffer_id != INVALID_HANDLE_ID);
        assert(this->HasSwapchain(handle));

        auto &context = swap_contexts[current_backbuffer_id];
        VkResult res;

        // Deffered command buffers
        {
            VkCommandBuffer cmd_lists[COMMANDLIST_COUNT];
            CommandList cmds[COMMANDLIST_COUNT];
            uint32_t counter = 0;

            CommandList cmd;
            while (context->active_commandlists.pop_front(cmd)) {
                res = vkEndCommandBuffer(GetCommandBuffer(cmd));
                assert(res == VK_SUCCESS);

                cmd_lists[counter] = GetCommandBuffer(cmd);
                cmds[counter] = cmd;
                counter++;

                context->free_commandlists.push_back(cmd);
            }

            // Submit all used cmd list to queue
            VkPipelineStageFlags wait_stages[] = {VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT};

            VkSubmitInfo submit_info = {};
            submit_info.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
            submit_info.commandBufferCount = counter;
            submit_info.pCommandBuffers = cmd_lists;
            submit_info.waitSemaphoreCount = 1;
            submit_info.pWaitSemaphores = &context->frame_resources[context->current_frame].acquire_sema;
            submit_info.pWaitDstStageMask = wait_stages;
            submit_info.signalSemaphoreCount = 1;
            submit_info.pSignalSemaphores = &context->frame_resources[context->current_frame].present_sema;

            // TODO: Remove hard coded queue
            vkQueueSubmit(queues[gfx_queue], 1, &submit_info, context->frame_resources[context->current_frame].fence);
        }

        swapchains[handle.id]->Present(queues[context->present_queue_family], *context.get());
    };

    void VulkanDevice::ResizeTexture(const TextureHandle &handle, u32 width, u32 height) {
        LOG("resize texture")

        auto &context = swap_contexts[current_backbuffer_id];

        textures.erase(handle.id);
        textures.insert(std::pair(handle.id, std::make_unique<VulkanTexture>(handle, raw_device)));
        // auto texture_binding = texture_bindings[handle.id];
        // handle.id = RANDOM_32;
        // descriptor_sets[texture_binding.first]->SetTexture(texture_binding.second, handle);
    };

    void *VulkanDevice::MapBuffer(const BufferHandle &handle) {
        assert(this->HasBuffer(handle));

        void *data;
        vmaMapMemory(raw_device->allocator, buffers[handle.id]->GetAllocation(), &data);
        return data;
    }

    void VulkanDevice::UnmapBuffer(const BufferHandle &handle) {
        assert(this->HasBuffer(handle));
        vmaUnmapMemory(raw_device->allocator, buffers[handle.id]->GetAllocation());
    };

    void VulkanDevice::BindBuffer(const DescriptorSetHandle &set, uint32_t binding, const BufferHandle &buffer) {
        assert(this->HasDescriptorSet(set));
        assert(this->HasBuffer(buffer));

        descriptor_sets[set.id]->SetBuffer(binding, buffers[buffer.id].get());
    }

    void VulkanDevice::BindTexture(const DescriptorSetHandle &set, uint32_t binding, const TextureHandle &texture) {
        assert(this->HasDescriptorSet(set));
        assert(this->HasTexture(texture));

        descriptor_sets[set.id]->SetTexture(binding, texture);
    }

    // == Device methods ========================================================

    void VulkanDevice::RebuildSwapchain(const SwapchainHandle &handle) {
        assert(this->HasSwapchain(handle));

        swapchains[handle.id]->Recreate(handle.window_handle);

        auto it = render_targets.find(handle.backbuffer.id);
        if (it != render_targets.end())
            it->second = swapchains[handle.id]->GetRenderTarget();
    };

    void VulkanDevice::QueueSubmit(QueueType queue, const CommandList &list) {
        auto cmd_buffer = GetCommandBuffer(list);

        vkEndCommandBuffer(cmd_buffer);

        VkSubmitInfo submit_info = {};
        submit_info.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
        submit_info.commandBufferCount = 1;
        submit_info.pCommandBuffers = &cmd_buffer;

        // onscreen(only on present queue) with render semaphore
        auto &context = swap_contexts[current_backbuffer_id];

        if (list.transfer) {
            submit_info.waitSemaphoreCount = 0;
            submit_info.signalSemaphoreCount = 0;
        } else {
            VkPipelineStageFlags wait_stages[] = {VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT};
            submit_info.waitSemaphoreCount = 1;
            submit_info.pWaitSemaphores = &context->frame_resources[context->current_frame].acquire_sema;
            submit_info.pWaitDstStageMask = wait_stages;
            submit_info.signalSemaphoreCount = 1;
            submit_info.pSignalSemaphores = &context->frame_resources[context->current_frame].present_sema;
        }

        VkQueue selected_queue;
        switch (queue) {
        case QueueType::GRAPHICS:
            selected_queue = queues[gfx_queue];
            break;
        case QueueType::COMPUTE:
            selected_queue = queues[compute_queue];
            break;
        case QueueType::TRANSFER:
            selected_queue = queues[transfer_queue];
            break;
        }

        if (list.transfer) {
            vkQueueSubmit(selected_queue, 1, &submit_info, VK_NULL_HANDLE);
            vkQueueWaitIdle(selected_queue);
        } else {
            vkQueueSubmit(selected_queue, 1, &submit_info, context->frame_resources[context->current_frame].fence);
        }
    };

    CommandList VulkanDevice::BeginCommandListEXP() {
        assert(current_backbuffer_id != INVALID_HANDLE_ID);

        VkResult res;
        CommandList cmd;

        auto &context = swap_contexts[current_backbuffer_id];

        // if there is no free commnd buffer then create one
        if (!context->free_commandlists.pop_front(cmd)) {
            cmd.id = context->commandlist_count.fetch_add(1);
            assert(cmd.id < COMMANDLIST_COUNT);

            for (auto &resources : context->frame_resources) {

                VkCommandPoolCreateInfo pool_info = {};
                pool_info.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
                pool_info.queueFamilyIndex = gfx_queue;
                pool_info.flags = 0; // Optional

                res = vkCreateCommandPool(raw_device->device, &pool_info, nullptr, &resources.cmd_pools[cmd.id]);
                assert(res == VK_SUCCESS);

                VkCommandBufferAllocateInfo commandBufferInfo = {};
                commandBufferInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
                commandBufferInfo.commandBufferCount = 1;
                commandBufferInfo.commandPool = resources.cmd_pools[cmd.id];
                commandBufferInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;

                res = vkAllocateCommandBuffers(raw_device->device, &commandBufferInfo, &resources.cmd_buffers[cmd.id]);
                assert(res == VK_SUCCESS);
            }
        }

        // Reset the command pool and the buffers allocated form it
        res = vkResetCommandPool(raw_device->device, GetFrameResources().cmd_pools[cmd.id], 0);
        assert(res == VK_SUCCESS);

        // Start record the command buffer
        VkCommandBufferBeginInfo begin_info = {};
        begin_info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
        begin_info.flags = VK_COMMAND_BUFFER_USAGE_SIMULTANEOUS_USE_BIT;
        begin_info.pInheritanceInfo = nullptr; // Optional

        res = vkBeginCommandBuffer(GetFrameResources().cmd_buffers[cmd.id], &begin_info);
        assert(res == VK_SUCCESS);

        context->active_commandlists.push_back(cmd);
        return cmd;
    };

    CommandList VulkanDevice::BeginTransferList() {

        CommandList list;
        list.transfer = true;
        list.id = 0;

        // transfer_lists
        if (transfer_buffers[0] == VK_NULL_HANDLE) {
            transfer_buffers[0] = transfer_list_allocator->Allocate();
        }

        VkCommandBufferBeginInfo begin_info = {};
        begin_info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
        begin_info.flags = VK_COMMAND_BUFFER_USAGE_SIMULTANEOUS_USE_BIT;
        begin_info.pInheritanceInfo = nullptr;

        vkBeginCommandBuffer(transfer_buffers[0], &begin_info);

        return list;
    };

    // == Render passes ======================================================================

    void VulkanDevice::BeginRenderPassEXP(const CommandList &cmd, const RenderPassHandle &render_pass) {
        assert(this->HasRenderPass(render_pass));

        auto cmd_buffer = GetCommandBuffer(cmd);

        // TODO: remove hacky solution
        auto vk_render_pass = render_passes[render_pass.id];
        auto height = render_pass.color[0].texture->height;
        auto width = render_pass.color[0].texture->width;

        VulkanFboCache::FboKey key = {};
        key.render_pass = vk_render_pass;
        key.width = width;
        key.height = height;

        const Squid::RHI::TextureHandle *texture;
        for (size_t i = 0; i < render_pass.num_attachments; i++) {
            texture = render_pass.color[i].texture;
            key.attachments[i] = textures[texture->id]->rtv;
        }

        if (render_pass.ds.texture) {
            texture = render_pass.ds.texture;
            key.attachments[render_pass.num_attachments] = textures[texture->id]->dsv;
        }

        auto framebuffer = fbo_cache->GetFramebuffer(key);

        std::array<VkClearValue, 2> clear_values = {};
        clear_values[0].color = {0.0f, 0.0f, 0.0f, 1.0f};
        clear_values[1].depthStencil = {1.0f, 0};

        VkRect2D area;
        area.extent = {width, height};
        area.offset = {0, 0};

        VkRenderPassBeginInfo render_pass_begin_info = {};
        render_pass_begin_info.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
        render_pass_begin_info.clearValueCount = static_cast<uint32_t>(clear_values.size());
        render_pass_begin_info.pClearValues = clear_values.data();
        render_pass_begin_info.renderPass = vk_render_pass;
        render_pass_begin_info.framebuffer = framebuffer;
        render_pass_begin_info.renderArea = area;

        vkCmdBeginRenderPass(cmd_buffer, &render_pass_begin_info, VK_SUBPASS_CONTENTS_INLINE);
    }

    void VulkanDevice::BeginRenderPass(const CommandList &cmd, const RenderTargetHandle &render_target) {
        assert(this->HasRenderTarget(render_target));

        auto cmd_buffer = GetCommandBuffer(cmd);

        VulkanFboCache::RenderPassKey renderpass_key = {};
        renderpass_key.final_color_layout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;
        renderpass_key.final_depth_layout = VK_IMAGE_LAYOUT_UNDEFINED;
        renderpass_key.color_format = VkFormat::VK_FORMAT_B8G8R8A8_UNORM;
        renderpass_key.depth_format = VkFormat::VK_FORMAT_UNDEFINED;
        renderpass_key.clear = ClearFlags::ColorClear;
        auto render_pass = fbo_cache->GetRenderPass(renderpass_key);

        auto &context = swap_contexts[render_target.id];
        auto attachment = render_targets[render_target.id]->GetAttachments(context->current_frame);

        auto width = render_targets[render_target.id]->GetWidth();
        auto height = render_targets[render_target.id]->GetHeight();

        VulkanFboCache::FboKey key = {};
        key.render_pass = render_pass;
        key.attachments[0] = attachment[0];
        key.width = width;
        key.height = height;
        auto framebuffer = fbo_cache->GetFramebuffer(key);

        VkClearColorValue clear_color = {0};
        clear_color.float32[0] = 0.0;
        clear_color.float32[1] = 0.0;
        clear_color.float32[2] = 0.0;
        clear_color.float32[3] = 1.0;
        VkClearDepthStencilValue clear_ds = {1.0f, 0};
        VkClearValue clear_value = {};
        clear_value.depthStencil = clear_ds;

        VkRect2D area;
        area.extent = {width, height};
        area.offset = {0, 0};

        VkRenderPassBeginInfo render_pass_begin_info = {};
        render_pass_begin_info.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
        render_pass_begin_info.clearValueCount = 1;
        render_pass_begin_info.pClearValues = &clear_value;
        render_pass_begin_info.renderPass = render_pass;
        render_pass_begin_info.framebuffer = framebuffer;
        render_pass_begin_info.renderArea = area;

        vkCmdBeginRenderPass(cmd_buffer, &render_pass_begin_info, VK_SUBPASS_CONTENTS_INLINE);
    };

    void VulkanDevice::EndRenderPass(const CommandList &cmd) {
        auto cmd_buffer = GetCommandBuffer(cmd);
        vkCmdEndRenderPass(cmd_buffer);
    };

    // == Bindings ==================================================================

    void VulkanDevice::BindScissorRects(const CommandList &cmd, uint32_t rects_count, const Rect *rects) {
        auto cmd_buffer = GetCommandBuffer(cmd);

        std::vector<VkRect2D> scissor_rects(rects_count);
        for (uint32_t i = 0; i < rects_count; i++) {
            scissor_rects[i].offset.x = rects[i].x;
            scissor_rects[i].offset.y = rects[i].y;
            scissor_rects[i].extent.height = rects[i].height;
            scissor_rects[i].extent.width = rects[i].width;
        }

        vkCmdSetScissor(cmd_buffer, 0, rects_count, scissor_rects.data());
    };

    void VulkanDevice::BindViewports(const CommandList &cmd, uint32_t viewports_count, const Viewport *viewports) {
        auto cmd_buffer = GetCommandBuffer(cmd);

        std::vector<VkViewport> vk_viewports(viewports_count);
        for (uint32_t i = 0; i < viewports_count; i++) {
            vk_viewports[i].x = static_cast<float>(viewports[i].x);
            vk_viewports[i].y = static_cast<float>(viewports[i].y);
            vk_viewports[i].height = static_cast<float>(viewports[i].height);
            vk_viewports[i].width = static_cast<float>(viewports[i].width);
            vk_viewports[i].minDepth = viewports[i].min_depth;
            vk_viewports[i].maxDepth = viewports[i].max_depth;
        }

        vkCmdSetViewport(cmd_buffer, 0, viewports_count, vk_viewports.data());
    };

    void VulkanDevice::BindVertexBuffer(const CommandList &cmd, const BufferHandle &vertex_buffer, uint32_t slot) {
        assert(this->HasBuffer(vertex_buffer));

        auto cmd_buffer = GetCommandBuffer(cmd);

        VkBuffer vertex_buffers[] = {buffers[vertex_buffer.id]->GetBuffer()};
        VkDeviceSize offsets[] = {0};
        vkCmdBindVertexBuffers(cmd_buffer, 0, 1, vertex_buffers, offsets);
    };

    void VulkanDevice::BindIndexBuffer(
        const CommandList &cmd, const BufferHandle &index_buffer, uint32_t offset, IndexFormat index_format) {
        assert(this->HasBuffer(index_buffer));

        auto cmd_buffer = GetCommandBuffer(cmd);

        auto buffer = buffers[index_buffer.id]->GetBuffer();
        vkCmdBindIndexBuffer(
            cmd_buffer, buffer, 0,
            index_format == IndexFormat::INDEX_16BIT ? VK_INDEX_TYPE_UINT16 : VK_INDEX_TYPE_UINT32);
    };

    // TODO
    // SPEED: merge multiple bind calls into one with count
    void VulkanDevice::BindDescriptorSet(
        const CommandList &cmd, const GraphicsPipelineHandle &pso, const DescriptorSetHandle &set, u32 index) {
        assert(current_backbuffer_id != INVALID_HANDLE_ID);
        assert(this->HasPipeline(pso));
        assert(this->HasDescriptorSet(set));

        auto cmd_buffer = GetCommandBuffer(cmd);

        auto allocated =
            descriptor_sets[set.id]->GetDescriptorSet(this->swap_contexts[current_backbuffer_id]->current_frame, textures);

        vkCmdBindDescriptorSets(
            cmd_buffer, VK_PIPELINE_BIND_POINT_GRAPHICS, gfx_pipelines[pso.id]->get_layout(), index, 1, &allocated, 0,
            nullptr);
    };

    void VulkanDevice::BindPipelineState(const CommandList &cmd, const GraphicsPipelineHandle &pso) {
        assert(this->HasPipeline(pso));

        auto cmd_buffer = GetCommandBuffer(cmd);
        vkCmdBindPipeline(cmd_buffer, VK_PIPELINE_BIND_POINT_GRAPHICS, gfx_pipelines[pso.id]->get_pipeline());
    };

    // == Draw, Dispatch ==============================================================
    void VulkanDevice::DrawIndexed(
        const CommandList &cmd, uint32_t index_count, uint32_t start_index, uint32_t vertex_offset) {
        auto cmd_buffer = GetCommandBuffer(cmd);

        vkCmdDrawIndexed(cmd_buffer, index_count, 1, start_index, vertex_offset, 0);
    };

    // == Resource Copies ===========================================================

    void VulkanDevice::Copy(const CommandList &cmd, const BufferHandle &src, const BufferHandle &dst) {
        assert(this->HasBuffer(src));
        assert(this->HasBuffer(dst));

        auto cmd_buffer = GetCommandBuffer(cmd);

        auto src_buffer = buffers[src.id]->GetBuffer();
        auto dst_buffer = buffers[dst.id]->GetBuffer();

        VkBufferCopy copy_region = {};
        copy_region.srcOffset = 0;
        copy_region.dstOffset = 0;
        copy_region.size = src.size;

        vkCmdCopyBuffer(cmd_buffer, src_buffer, dst_buffer, 1, &copy_region);
    };

    void
    VulkanDevice::Copy(const CommandList &cmd, const BufferHandle &src, const TextureHandle &dst, u64 layer_offset) {
        assert(this->HasBuffer(src));
        assert(this->HasTexture(dst));

        auto cmd_buffer = GetCommandBuffer(cmd);

        auto src_buffer = buffers[src.id]->GetBuffer();
        auto dst_texture = textures[dst.id]->GetImage();

        this->Transition(cmd_buffer, dst_texture, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL);

        std::vector<VkBufferImageCopy> copies;
        copies.reserve(dst.layers);

        for (auto i = 0; i < dst.layers; i++) {
            VkBufferImageCopy copy_region = {};
            copy_region.bufferOffset = i * layer_offset;
            copy_region.bufferImageHeight = 0;
            copy_region.bufferRowLength = 0;

            copy_region.imageExtent = {dst.width, dst.height, 1};
            copy_region.imageOffset = {0, 0, 0};

            copy_region.imageSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT; // Colour attachment
            copy_region.imageSubresource.baseArrayLayer = i;                     // 0
            copy_region.imageSubresource.layerCount = 1;
            copy_region.imageSubresource.mipLevel = 0;

            copies.push_back(copy_region);
        }

        vkCmdCopyBufferToImage(
            cmd_buffer, src_buffer, dst_texture, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, copies.size(), copies.data());

        this->Transition(
            cmd_buffer, dst_texture, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);
    };

    void VulkanDevice::Barrier(const CommandList &cmd, const TextureHandle &handle, ImageLayout new_layout_in) {
        assert(this->HasTexture(handle));

        auto cmd_buffer = GetCommandBuffer(cmd);

        VkImageLayout old_layout = ConvertImageLayout(handle.layout);
        VkImageLayout new_layout = ConvertImageLayout(new_layout_in);

        VkImageMemoryBarrier barrier = {};
        barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
        barrier.oldLayout = old_layout;
        barrier.newLayout = new_layout;
        barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
        barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
        barrier.image = textures[handle.id]->GetImage();
        barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
        barrier.subresourceRange.baseMipLevel = 0;
        barrier.subresourceRange.levelCount = 1;
        barrier.subresourceRange.baseArrayLayer = 0;
        barrier.subresourceRange.layerCount = 1;
        barrier.srcAccessMask = ConvertImageLayoutToAccess(handle.layout);
        barrier.dstAccessMask = ConvertImageLayoutToAccess(new_layout_in);

        VkPipelineStageFlags source_stage;
        VkPipelineStageFlags destination_stage;

        // old stage
        switch (handle.layout) {
        case ImageLayout::GENERAL:
            source_stage = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
            break;
        case ImageLayout::UNDEFINED:
            source_stage = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
            break;
        case ImageLayout::RENDER_TARGET:
            source_stage = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
            break;
        case ImageLayout::SHADER_RESOURCE:
            source_stage = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
            break;
        case ImageLayout::DEPTH_STENCIL:
            source_stage = VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT | VK_PIPELINE_STAGE_LATE_FRAGMENT_TESTS_BIT;
            break;
        case ImageLayout::DEPTH_STENCIL_READONLY:
            source_stage = VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT | VK_PIPELINE_STAGE_LATE_FRAGMENT_TESTS_BIT;
            break;
        case ImageLayout::UNORDERED_ACCESS:
            source_stage = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
            break;
        }

        switch (new_layout_in) {
        case ImageLayout::GENERAL:
            destination_stage = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
            break;
        case ImageLayout::UNDEFINED:
            destination_stage = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
            break;
        case ImageLayout::RENDER_TARGET:
            destination_stage = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
            break;
        case ImageLayout::SHADER_RESOURCE:
            destination_stage = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
            break;
        case ImageLayout::DEPTH_STENCIL:
            destination_stage = VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT | VK_PIPELINE_STAGE_LATE_FRAGMENT_TESTS_BIT;
            break;
        case ImageLayout::DEPTH_STENCIL_READONLY:
            destination_stage = VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT | VK_PIPELINE_STAGE_LATE_FRAGMENT_TESTS_BIT;
            break;
        case ImageLayout::UNORDERED_ACCESS:
            destination_stage = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
            break;
        }

        vkCmdPipelineBarrier(cmd_buffer, source_stage, destination_stage, 0, 0, nullptr, 0, nullptr, 1, &barrier);
    }

    void VulkanDevice::Transition(
        VkCommandBuffer cmd_buffer, VkImage image, VkImageLayout old_layout, VkImageLayout new_layout) {

        VkImageMemoryBarrier barrier = {};
        barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
        barrier.oldLayout = old_layout;
        barrier.newLayout = new_layout;
        barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
        barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
        barrier.image = image;
        barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
        barrier.subresourceRange.baseMipLevel = 0;
        barrier.subresourceRange.levelCount = 1;
        barrier.subresourceRange.baseArrayLayer = 0;
        barrier.subresourceRange.layerCount = 1;

        VkPipelineStageFlags source_stage;
        VkPipelineStageFlags destination_stage;

        if (old_layout == VK_IMAGE_LAYOUT_UNDEFINED && new_layout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL) {
            barrier.srcAccessMask = 0;
            barrier.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;

            source_stage = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
            destination_stage = VK_PIPELINE_STAGE_TRANSFER_BIT;
        } else if (
            old_layout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL &&
            new_layout == VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL) {
            barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
            barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;

            source_stage = VK_PIPELINE_STAGE_TRANSFER_BIT;
            destination_stage = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
        } else {
            throw std::invalid_argument("unsupported layout transition!");
        }

        vkCmdPipelineBarrier(cmd_buffer, source_stage, destination_stage, 0, 0, nullptr, 0, nullptr, 1, &barrier);
    }

    VulkanDevice::~VulkanDevice() {
        vkDeviceWaitIdle(raw_device->device);
        LOG("destroying presentation syncronization primitives")
        for (auto &swap : swap_contexts) {
            for (auto i = 0; i < BACKBUFFER_COUNT; i++) {
                vkDestroySemaphore(raw_device->device, swap.second->frame_resources[i].acquire_sema, nullptr);
                vkDestroySemaphore(raw_device->device, swap.second->frame_resources[i].present_sema, nullptr);
                vkDestroyFence(raw_device->device, swap.second->frame_resources[i].fence, nullptr);
            }
        }
    }

} // namespace RHI
} // namespace Squid
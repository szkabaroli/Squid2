#pragma once
#include <pch.h>
#include <thread>
#include <utility>
#include <optional>

#include "Buffer.h"
#include "CommandAllocator.h"
#include "DescriptorSet.h"
#include "FboCache.h"
#include "Pipeline.h"
#include "RenderTarget.h"
#include "Swapchain.h"
#include "Texture.h"

namespace Squid {
namespace RHI {

    const std::vector<const char *> device_extensions = {
        VK_KHR_SWAPCHAIN_EXTENSION_NAME,
        // VK_EXT_DESCRIPTOR_INDEXING_EXTENSION_NAME,
        // VK_KHR_MAINTENANCE3_EXTENSION_NAME,
        // VK_KHR_RAY_TRACING_EXTENSION_NAME,
        // VK_KHR_PIPELINE_LIBRARY_EXTENSION_NAME,
        // VK_KHR_DEFERRED_HOST_OPERATIONS_EXTENSION_NAME,
        // VK_KHR_BUFFER_DEVICE_ADDRESS_EXTENSION_NAME
    };

    class VulkanDevice : public Device {
        friend class VulkanAdapter;

    public:
        VulkanDevice(
            std::tuple<uint32_t, uint32_t, uint32_t> queue_families,
            std::shared_ptr<RawInstance> raw_instance,
            VkPhysicalDevice physical);

        void LoadSwapchain(const SwapchainHandle &handle) override;
        void LoadRenderTarget(const RenderTargetHandle &handle) override;
        void LoadBuffer(const BufferHandle &handle) override;
        void LoadTexture(const TextureHandle &handle) override;
        void LoadPipeline(const ComputePipelineHandle &handle) override;
        void LoadPipeline(const GraphicsPipelineHandle &handle) override;
        void LoadDescriptorSet(const DescriptorSetHandle &handle) override;
        void LoadRenderPass(const RenderPassHandle &handle) override;

        void UnloadSwapchain(const SwapchainHandle &handle) override;
        void UnloadRenderTarget(const RenderTargetHandle &handle) override;
        void UnloadBuffer(const BufferHandle &handle) override;
        void UnloadTexture(const TextureHandle &handle) override;
        void UnloadPipeline(const ComputePipelineHandle &handle) override;
        void UnloadPipeline(const GraphicsPipelineHandle &handle) override;
        void UnloadDescriptorSet(const DescriptorSetHandle &handle) override;
        void UnloadRenderPass(const RenderPassHandle &handle) override;

        bool HasSwapchain(const SwapchainHandle &handle) const override;
        bool HasRenderTarget(const RenderTargetHandle &handle) const override;
        bool HasBuffer(const BufferHandle &handle) const override;
        bool HasTexture(const TextureHandle &handle) const override;
        bool HasPipeline(const ComputePipelineHandle &handle) const override;
        bool HasPipeline(const GraphicsPipelineHandle &handle) const override;
        bool HasDescriptorSet(const DescriptorSetHandle &handle) const override;
        bool HasRenderPass(const RenderPassHandle &handle) const override;

        void SetName(const BufferHandle &handle, const std::string &name) const override;
        void SetName(const TextureHandle &handle, const std::string &name) const override;
        void SetName(const RenderPassHandle &handle, const std::string &name) const override;

        // == Command list =============================================================
        // CommandList BeginList() override;
        CommandList BeginTransferList() override;
        CommandList BeginCommandListEXP() override;

        void BeginRenderPassEXP(const CommandList &cmd, const RenderPassHandle &render_pass) override;
        void BeginRenderPass(const CommandList &cmd, const RenderTargetHandle &render_target) override;
        void EndRenderPass(const CommandList &cmd) override;

        virtual void BeginFrameEXP(const SwapchainHandle &handle) override;
        virtual void EndFrameEXP(const SwapchainHandle &handle) override;

        // == Binding ==================================================================

        void BindScissorRects(const CommandList &cmd, uint32_t rects_count, const Rect *rects) override;
        void BindViewports(const CommandList &cmd, uint32_t viewports_count, const Viewport *pViewports) override;

        void BindVertexBuffer(const CommandList &cmd, const BufferHandle &vertex_buffer, uint32_t slot) override;
        void BindIndexBuffer(
            const CommandList &cmd,
            const BufferHandle &index_buffer,
            uint32_t offset,
            IndexFormat index_format) override;

        void BindDescriptorSet(
            const CommandList &cmd,
            const GraphicsPipelineHandle &pso,
            const DescriptorSetHandle &set,
            u32 index) override;
        void BindPipelineState(const CommandList &cmd, const GraphicsPipelineHandle &pso) override;

        // == Draw, Dispatch ==============================================================
        void DrawIndexed(
            const CommandList &cmd, uint32_t index_count, uint32_t start_index, uint32_t vertex_offset) override;

        // == Resource Copies ===========================================================

        void Copy(const CommandList &cmd, const BufferHandle &dst, const BufferHandle &src) override;
        void Copy(const CommandList &cmd, const BufferHandle &dst, const TextureHandle &src, u64 layer_offset) override;

        void BindBuffer(const DescriptorSetHandle &set, uint32_t binding, const BufferHandle &buffer) override;
        void BindTexture(const DescriptorSetHandle &set, uint32_t bindng, const TextureHandle &handle) override;

        void Barrier(const CommandList &cmd, const TextureHandle &handle, ImageLayout new_layout) override;

        void *MapBuffer(const BufferHandle &handle) override;
        void UnmapBuffer(const BufferHandle &handle) override;

        void QueueSubmit(QueueType queue, const CommandList &list) override;

        void RebuildSwapchain(const SwapchainHandle &handle) override;

        ~VulkanDevice();

    private:
        constexpr static uint32_t COMMANDLIST_MAX_COUNT = 16;

        inline VkCommandBuffer GetCommandBuffer(const CommandList &list) {
            if (list.transfer) {
                return transfer_buffers[list.id];
            } else {
                assert(current_backbuffer_id != INVALID_HANDLE_ID);
                return swap_contexts[current_backbuffer_id]
                    ->frame_resources[swap_contexts[current_backbuffer_id]->current_frame]
                    .cmd_buffers[list.id];
            }
        };

        inline FrameResources GetFrameResources() {
            assert(current_backbuffer_id != INVALID_HANDLE_ID);
            auto &context = swap_contexts[current_backbuffer_id];
            return context->frame_resources[context->current_frame];
        }

        void Transition(VkCommandBuffer cmd_buffer, VkImage image, VkImageLayout old_layout, VkImageLayout new_layout);

        // Selected queue families
        uint32_t gfx_queue;
        uint32_t compute_queue;
        uint32_t transfer_queue;
        std::map<uint32_t, VkQueue> queues;

        // Map of resource handle ids and abstracted vulkan objects
        std::unordered_map<uint64_t, std::unique_ptr<VulkanBuffer>> buffers;
        std::unordered_map<uint64_t, std::unique_ptr<VulkanTexture>> textures;
        std::unordered_map<uint64_t, std::unique_ptr<VulkanComputePipeline>> compute_pipelines;
        std::unordered_map<uint64_t, std::unique_ptr<VulkanGraphicsPipeline>> gfx_pipelines;
        std::unordered_map<uint64_t, std::unique_ptr<VulkanDescriptorSet>> descriptor_sets;
        std::unordered_map<uint64_t, std::unique_ptr<VulkanSwapchain>> swapchains;
        std::unordered_map<uint64_t, std::unique_ptr<VulkanRenderTarget>> render_targets;

        std::unordered_map<uint64_t, VkRenderPass> render_passes;

        // Swapchain and backbuffer render taraget mapping is 1 <-> 1
        // backbuffer_id -> Swapchain Context
        uint32_t current_backbuffer_id = INVALID_HANDLE_ID;
        std::unordered_map<uint64_t, std::unique_ptr<SwapchainContext>> swap_contexts;

        std::unique_ptr<VulkanFboCache> fbo_cache;
        std::unique_ptr<VulkanCommandAllocator> transfer_list_allocator;
        VkCommandBuffer transfer_buffers[4] = {VK_NULL_HANDLE};

        // Offscreen command buffers
        // One for each command list
        // std::unordered_map<uint64_t, VkCommandBuffer> offscreen_cmd_buffers;

        // std::unordered_map<uint64_t, std::unique_ptr<VulkanCommandAllocator>> cmd_allocators;

        // Ref counted raw handles
        std::shared_ptr<RawInstance> raw_instance;
        std::shared_ptr<RawDevice> raw_device;

        uint64_t id;
    };

} // namespace RHI
} // namespace Squid

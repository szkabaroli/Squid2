#pragma once

namespace Squid {
namespace RHI {

    struct CommandList;

    enum class QueueType { GRAPHICS, COMPUTE, TRANSFER };

    class Device {
        friend struct BufferHandle;
        friend struct DescriptorSetHandle;

    public:
        virtual ~Device() {} // <= important!

        virtual void LoadSwapchain(const SwapchainHandle &handle) = 0;
        virtual void LoadRenderTarget(const RenderTargetHandle &handle) = 0;
        virtual void LoadBuffer(const BufferHandle &handle) = 0;
        virtual void LoadTexture(const TextureHandle &handle) = 0;
        virtual void LoadPipeline(const ComputePipelineHandle &handle) = 0;
        virtual void LoadPipeline(const GraphicsPipelineHandle &handle) = 0;
        virtual void LoadDescriptorSet(const DescriptorSetHandle &handle) = 0;
        virtual void LoadRenderPass(const RenderPassHandle &handle) = 0;

        virtual void UnloadSwapchain(const SwapchainHandle &handle) = 0;
        virtual void UnloadRenderTarget(const RenderTargetHandle &handle) = 0;
        virtual void UnloadBuffer(const BufferHandle &handle) = 0;
        virtual void UnloadTexture(const TextureHandle &handle) = 0;
        virtual void UnloadPipeline(const ComputePipelineHandle &handle) = 0;
        virtual void UnloadPipeline(const GraphicsPipelineHandle &handle) = 0;
        virtual void UnloadDescriptorSet(const DescriptorSetHandle &handle) = 0;
        virtual void UnloadRenderPass(const RenderPassHandle &handle) = 0;

        virtual bool HasRenderTarget(const RenderTargetHandle &handle) const = 0;
        virtual bool HasBuffer(const BufferHandle &handle) const = 0;
        virtual bool HasTexture(const TextureHandle &handle) const = 0;
        virtual bool HasPipeline(const ComputePipelineHandle &handle) const = 0;
        virtual bool HasPipeline(const GraphicsPipelineHandle &handle) const = 0;
        virtual bool HasDescriptorSet(const DescriptorSetHandle &handle) const = 0;
        virtual bool HasSwapchain(const SwapchainHandle &handle) const = 0;
        virtual bool HasRenderPass(const RenderPassHandle &handle) const = 0;

        virtual void SetName(const BufferHandle &handle, const std::string &name) const = 0;
        virtual void SetName(const TextureHandle &handle, const std::string &name) const = 0;
        virtual void SetName(const RenderPassHandle &handle, const std::string &name) const = 0;

        virtual void BeginFrameEXP(const SwapchainHandle &handle) = 0;
        virtual void EndFrameEXP(const SwapchainHandle &handle) = 0;

        virtual void QueueSubmit(QueueType queue, const CommandList &list) = 0;

        // == Command list =============================================================
        virtual CommandList BeginCommandListEXP() = 0;
        // virtual CommandList BeginList() = 0;
        virtual CommandList BeginTransferList() = 0;

        virtual void BeginRenderPassEXP(const CommandList &cmd, const RenderPassHandle &render_pass) = 0;

        virtual void BeginRenderPass(const CommandList &cmd, const RenderTargetHandle &render_target) = 0;
        virtual void EndRenderPass(const CommandList &cmd) = 0;

        // == Binding ==================================================================

        virtual void BindScissorRects(const CommandList &cmd, u32 rects_count, const Rect *rects) = 0;
        virtual void BindViewports(const CommandList &cmd, u32 viewports_count, const Viewport *pViewports) = 0;

        virtual void BindVertexBuffer(const CommandList &cmd, const BufferHandle &vertex_buffer, u32 slot) = 0;
        virtual void BindIndexBuffer(
            const CommandList &cmd,
            const BufferHandle &indexBuffer,
            uint32_t offset,
            IndexFormat index_format = IndexFormat::INDEX_16BIT) = 0;

        virtual void BindDescriptorSet(
            const CommandList &cmd, const GraphicsPipelineHandle &pso, const DescriptorSetHandle &set, u32 index) = 0;
        virtual void BindPipelineState(const CommandList &cmd, const GraphicsPipelineHandle &pso) = 0;

        // == Draw, Dispatch ==============================================================

        virtual void
        DrawIndexed(const CommandList &cmd, u32 index_count, u32 start_index, u32 vertex_offset) = 0;

        // virtual void
        // Dispatch(const CommandList &cmd, uint32_t thread_group_x, uint32_t thread_group_y, uint32_t thread_group_z) =
        // 0;

        // == Resource Copies ===========================================================

        // Buffer -> Buffer
        virtual void Copy(const CommandList &cmd, const BufferHandle &dst, const BufferHandle &src) = 0;
        // Buffer -> Texture
        virtual void Copy(const CommandList &cmd, const BufferHandle &dst, const TextureHandle &src, u64 layer_offset = 0) = 0;
        // Texture -> Texture
        // virtual void Copy(const CommandList &cmd, const TextureHandle &dst, const TextureHandle &src) = 0;
        // Texture -> Buffer
        // virtual void Copy(const CommandList &cmd, const TextureHandle &dst, const BufferHandle &src) = 0;

        // Pipeline Barrier
        virtual void Barrier(const CommandList &cmd, const TextureHandle &handle, ImageLayout new_layout) = 0;
        // virtual void Barrier(const GPUBarrier* barriers, uint32_t numBarriers) = 0;

        virtual void BindBuffer(const DescriptorSetHandle &set, u32 bindng, const BufferHandle &handle) = 0;
        virtual void BindTexture(const DescriptorSetHandle &set, u32 bindng, const TextureHandle &handle) = 0;

        virtual void *MapBuffer(const BufferHandle &handle) = 0;
        virtual void UnmapBuffer(const BufferHandle &handle) = 0;

        virtual void ResizeTexture(const TextureHandle &handle, u32 width, u32 height) = 0;

        virtual void RebuildSwapchain(const SwapchainHandle &handle) = 0;
    };

} // namespace RHI
} // namespace Squid

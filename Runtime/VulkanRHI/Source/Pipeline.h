#pragma once
#include "Raw.h"
#include <pch.h>

namespace Squid { namespace RHI {

    class VulkanPipeline {
    public:
        VulkanPipeline(std::shared_ptr<RawDevice> raw_device) : raw_device(raw_device) {}
        VkPipeline get_pipeline() { return pipeline; };
        inline VkPipelineLayout get_layout() { return pipeline_layout; };

    protected:
        VkPipelineLayout pipeline_layout;
        VkPipeline pipeline;
        std::shared_ptr<RawDevice> raw_device;
    };

    class VulkanComputePipeline : public VulkanPipeline {
    public:
        VulkanComputePipeline(
            const ComputePipelineHandle &handle,
            std::vector<VkDescriptorSetLayout> layouts,
            std::shared_ptr<RawDevice> device);
        ~VulkanComputePipeline();
    };

    class VulkanGraphicsPipeline : public VulkanPipeline {
    public:
        VulkanGraphicsPipeline(
            const GraphicsPipelineHandle &handle,
            std::vector<VkDescriptorSetLayout> layouts,
            VkRenderPass render_pass,
            std::shared_ptr<RawDevice> raw_device);
        ~VulkanGraphicsPipeline();

    private:
        VkShaderModule vertex_module;
        VkShaderModule pixel_module;
    };

}} // namespace Squid::RHI

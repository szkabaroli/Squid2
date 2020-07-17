#include "Pipeline.h"

namespace Squid {
namespace RHI {

    VkPrimitiveTopology convert_primitive_topology(PrimitiveTopology topo) {
        switch (topo) {
        case PrimitiveTopology::TRIANGLE_FAN:
            return VK_PRIMITIVE_TOPOLOGY_TRIANGLE_FAN;
        case PrimitiveTopology::TRIANGLE_LIST:
            return VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
        case PrimitiveTopology::TRIANGLE_STRIP:
            return VK_PRIMITIVE_TOPOLOGY_TRIANGLE_STRIP;
        default:
            return VK_PRIMITIVE_TOPOLOGY_MAX_ENUM;
        }
    }

    VkShaderModule create_shader_module(const std::string &code, VkDevice device) {
        VkShaderModule shader_module;

        VkShaderModuleCreateInfo create_info = {};
        create_info.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
        create_info.codeSize = code.size();
        create_info.pCode = reinterpret_cast<const uint32_t *>(code.data());
        vkCreateShaderModule(device, &create_info, nullptr, &shader_module);

        return shader_module;
    };

    // Compute pipeline

    VulkanComputePipeline::VulkanComputePipeline(
        const ComputePipelineHandle &handle,
        std::vector<VkDescriptorSetLayout> layouts,
        std::shared_ptr<RawDevice> device)
        : VulkanPipeline(device) {
        // Pipeline layout
        // handle.descriptor_set.descriptors;
        VkPipelineLayoutCreateInfo pipeline_layout_info = {};
        pipeline_layout_info.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
        pipeline_layout_info.setLayoutCount = static_cast<uint32_t>(layouts.size());
        pipeline_layout_info.pSetLayouts = layouts.data();

        vkCreatePipelineLayout(raw_device->device, &pipeline_layout_info, nullptr, &pipeline_layout);

        // Pipeline
        auto compute_module = create_shader_module(handle.compute_shader, raw_device->device);

        VkPipelineShaderStageCreateInfo shader_info = {};
        shader_info.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
        shader_info.module = compute_module;
        shader_info.stage = VK_SHADER_STAGE_COMPUTE_BIT;
        shader_info.pName = "main";

        VkComputePipelineCreateInfo pipeline_info = {};
        pipeline_info.sType = VK_STRUCTURE_TYPE_COMPUTE_PIPELINE_CREATE_INFO;
        pipeline_info.flags = 0;
        pipeline_info.layout = pipeline_layout;
        pipeline_info.stage = shader_info;

        vkCreateComputePipelines(raw_device->device, VK_NULL_HANDLE, 1, &pipeline_info, nullptr, &pipeline);
        vkDestroyShaderModule(raw_device->device, compute_module, nullptr);
    }

    VulkanComputePipeline::~VulkanComputePipeline() {
        LOG("destroy pipeline")
        vkDestroyPipelineLayout(raw_device->device, pipeline_layout, nullptr);
        vkDestroyPipeline(raw_device->device, pipeline, nullptr);
    }

    // Graphics pipeline

    VulkanGraphicsPipeline::VulkanGraphicsPipeline(
        const GraphicsPipelineHandle &handle,
        std::vector<VkDescriptorSetLayout> layouts,
        VkRenderPass render_pass,
        std::shared_ptr<RawDevice> raw_device)
        : VulkanPipeline(raw_device) {
        uint32_t layout_size = 0;

        static_assert(sizeof(float) == 4);
        static_assert(sizeof(float) * 2 == 8);
        static_assert(sizeof(float) * 3 == 12);
        static_assert(sizeof(float) * 4 == 16);

        auto count = handle.vertex_layout.input_count;

        std::vector<VkVertexInputAttributeDescription> vertex_attributes(handle.vertex_layout.input_count);

        for (auto i = 0; i < count; i++) {

            vertex_attributes[i].binding = 0;
            vertex_attributes[i].location = i;

            switch (handle.vertex_layout.inputs[i].type) {
            case VertexType::FLOAT:
                layout_size += 4;
                vertex_attributes[i].format = VK_FORMAT_R32_SFLOAT;
                vertex_attributes[i].offset = handle.vertex_layout.inputs[i].offset;
                break;
            case VertexType::FLOAT2:
                layout_size += 8;
                vertex_attributes[i].format = VK_FORMAT_R32G32_SFLOAT;
                vertex_attributes[i].offset = handle.vertex_layout.inputs[i].offset;
                break;
            case VertexType::FLOAT3:
                layout_size += 12;
                vertex_attributes[i].format = VK_FORMAT_R32G32B32_SFLOAT;
                vertex_attributes[i].offset = handle.vertex_layout.inputs[i].offset;
                break;
            case VertexType::FLOAT4:
                layout_size += 16;
                vertex_attributes[i].format = VK_FORMAT_R8G8B8A8_UNORM;
                vertex_attributes[i].offset = handle.vertex_layout.inputs[i].offset;
                break;
            }
        }

        VkVertexInputBindingDescription binding_description = {};
        binding_description.binding = 0;
        binding_description.stride = handle.vertex_layout.stride;
        binding_description.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;

        VkPipelineVertexInputStateCreateInfo vertex_input_info = {};
        vertex_input_info.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
        vertex_input_info.vertexBindingDescriptionCount = 1;
        vertex_input_info.pVertexBindingDescriptions = &binding_description;
        vertex_input_info.vertexAttributeDescriptionCount = static_cast<uint32_t>(vertex_attributes.size());
        vertex_input_info.pVertexAttributeDescriptions = vertex_attributes.data();

        VkPipelineInputAssemblyStateCreateInfo input_assembly = {};
        input_assembly.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
        input_assembly.topology = convert_primitive_topology(handle.topology);
        input_assembly.primitiveRestartEnable = handle.primitive_restart ? VK_TRUE : VK_FALSE;

        VkViewport viewport = {};
        viewport.x = 0.0f;
        viewport.y = 0.0f;
        // TODO
        viewport.width = (float)1920;
        viewport.height = (float)1080;
        viewport.minDepth = 0.0f;
        viewport.maxDepth = 1.0f;

        VkRect2D scissor = {};
        scissor.offset = {0, 0};
        scissor.extent = {1920, 1080};

        VkPipelineViewportStateCreateInfo viewport_state = {};
        viewport_state.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
        viewport_state.viewportCount = 1;
        viewport_state.pViewports = &viewport;
        viewport_state.scissorCount = 1;
        viewport_state.pScissors = &scissor;

        VkPipelineRasterizationStateCreateInfo rasterizer = {};
        rasterizer.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
        rasterizer.depthClampEnable = VK_FALSE;
        rasterizer.rasterizerDiscardEnable = VK_FALSE;
        rasterizer.polygonMode = VK_POLYGON_MODE_FILL;
        rasterizer.lineWidth = 1.0f;
        rasterizer.cullMode = ConvertCullMode(handle.cull_mode);
        rasterizer.frontFace = VK_FRONT_FACE_CLOCKWISE;
        rasterizer.depthBiasEnable = VK_FALSE;
        rasterizer.depthBiasConstantFactor = 0.0f; // Optional
        rasterizer.depthBiasClamp = 0.0f;          // Optional
        rasterizer.depthBiasSlopeFactor = 0.0f;    // Optional

        VkPipelineMultisampleStateCreateInfo multisampling = {};
        multisampling.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
        multisampling.sampleShadingEnable = VK_FALSE;
        multisampling.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;
        multisampling.minSampleShading = 1.0f;          // Optional
        multisampling.pSampleMask = nullptr;            // Optional
        multisampling.alphaToCoverageEnable = VK_FALSE; // Optional
        multisampling.alphaToOneEnable = VK_FALSE;      // Optional

        VkPipelineDepthStencilStateCreateInfo depth_stencil = {};
        depth_stencil.sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO;
        depth_stencil.depthTestEnable = handle.depth_test ? VK_TRUE : VK_FALSE;
        depth_stencil.depthWriteEnable = handle.depth_write ? VK_TRUE : VK_FALSE;

        depth_stencil.depthCompareOp = ConvertCompareOp(handle.compare_op);
        depth_stencil.depthBoundsTestEnable = VK_FALSE;
        depth_stencil.minDepthBounds = 0.0f; // Optional
        depth_stencil.maxDepthBounds = 1.0f; // Optional
        depth_stencil.stencilTestEnable = handle.stencil_test ? VK_TRUE : VK_FALSE;
        depth_stencil.front = {}; // Optional
        depth_stencil.back = {};  // Optional

        auto &rt_0_blend = handle.blend_state.rt_blends[0];

        VkPipelineColorBlendAttachmentState cb_attachment = {};
        cb_attachment.colorWriteMask =
            VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
        cb_attachment.blendEnable = handle.blend_state.enabled ? VK_TRUE : VK_FALSE;

        cb_attachment.srcColorBlendFactor = ConvertBlend(rt_0_blend.src_blend);
        // VK_BLEND_FACTOR_SRC_ALPHA;           // Optional
        cb_attachment.dstColorBlendFactor = ConvertBlend(rt_0_blend.dst_blend);
        // VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA; // Optional
        cb_attachment.colorBlendOp = ConvertBlendOp(rt_0_blend.blend_op);
        // VK_BLEND_OP_ADD; // Optional

        cb_attachment.srcAlphaBlendFactor = ConvertBlend(rt_0_blend.src_blend_alpha);
        // VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA; // Optional
        cb_attachment.dstAlphaBlendFactor = ConvertBlend(rt_0_blend.dst_blend_alpha);
        // VK_BLEND_FACTOR_ZERO;                // Optional
        cb_attachment.alphaBlendOp = ConvertBlendOp(rt_0_blend.blend_op_alpha);
        // VK_BLEND_OP_ADD; // Optional

        VkPipelineColorBlendStateCreateInfo color_blending = {};
        color_blending.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
        color_blending.logicOpEnable = VK_FALSE;
        color_blending.logicOp = VK_LOGIC_OP_COPY; // Optional
        color_blending.attachmentCount = 1;
        color_blending.pAttachments = &cb_attachment;
        color_blending.blendConstants[0] = 0.0f; // Optional
        color_blending.blendConstants[1] = 0.0f; // Optional
        color_blending.blendConstants[2] = 0.0f; // Optional
        color_blending.blendConstants[3] = 0.0f; // Optional

        VkDynamicState dynamic_states[] = {VK_DYNAMIC_STATE_VIEWPORT, VK_DYNAMIC_STATE_LINE_WIDTH,
                                           VK_DYNAMIC_STATE_SCISSOR};

        VkPipelineDynamicStateCreateInfo dynamic_state_info = {};
        dynamic_state_info.sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
        dynamic_state_info.dynamicStateCount = 3;
        dynamic_state_info.pDynamicStates = dynamic_states;

        // Dynamic states
        std::vector<VkPipelineShaderStageCreateInfo> shader_stages;

        if (handle.vertex_shader.length() > 0) {
            vertex_module = create_shader_module(handle.vertex_shader, raw_device->device);

            VkPipelineShaderStageCreateInfo shader_info = {};
            shader_info.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
            shader_info.module = vertex_module;
            shader_info.stage = VK_SHADER_STAGE_VERTEX_BIT;
            shader_info.pName = "mainVS";

            shader_stages.push_back(shader_info);
        }

        if (handle.pixel_shader.length() > 0) {
            pixel_module = create_shader_module(handle.pixel_shader, raw_device->device);

            VkPipelineShaderStageCreateInfo shader_info = {};
            shader_info.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
            shader_info.module = pixel_module;
            shader_info.stage = VK_SHADER_STAGE_FRAGMENT_BIT;
            shader_info.pName = "mainPS";

            shader_stages.push_back(shader_info);
        }

        VkPipelineLayoutCreateInfo pipeline_layout_info = {};
        pipeline_layout_info.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
        pipeline_layout_info.setLayoutCount = static_cast<uint32_t>(layouts.size());
        pipeline_layout_info.pSetLayouts = layouts.data();
        pipeline_layout_info.pushConstantRangeCount = 0;    // Optional
        pipeline_layout_info.pPushConstantRanges = nullptr; // Optional

        vkCreatePipelineLayout(raw_device->device, &pipeline_layout_info, nullptr, &pipeline_layout);

        VkGraphicsPipelineCreateInfo pipeline_info = {};
        pipeline_info.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
        pipeline_info.stageCount = static_cast<uint32_t>(shader_stages.size());
        pipeline_info.pStages = shader_stages.data();
        pipeline_info.pVertexInputState = &vertex_input_info;
        pipeline_info.pInputAssemblyState = &input_assembly;
        pipeline_info.pViewportState = &viewport_state;
        pipeline_info.pRasterizationState = &rasterizer;
        pipeline_info.pMultisampleState = &multisampling;
        pipeline_info.pDepthStencilState = &depth_stencil; // Optional
        pipeline_info.pColorBlendState = &color_blending;
        pipeline_info.pDynamicState = &dynamic_state_info; // Optional
        pipeline_info.layout = pipeline_layout;
        pipeline_info.basePipelineHandle = VK_NULL_HANDLE; // Optional
        pipeline_info.basePipelineIndex = -1;              // Optional
        pipeline_info.renderPass = render_pass;
        pipeline_info.subpass = 0;

        vkCreateGraphicsPipelines(raw_device->device, VK_NULL_HANDLE, 1, &pipeline_info, nullptr, &pipeline);

        if (pixel_module != VK_NULL_HANDLE) {
            vkDestroyShaderModule(raw_device->device, pixel_module, nullptr);
        }

        if (vertex_module != VK_NULL_HANDLE) {
            vkDestroyShaderModule(raw_device->device, vertex_module, nullptr);
        }
    }

    VulkanGraphicsPipeline::~VulkanGraphicsPipeline() {
        vkDestroyPipelineLayout(raw_device->device, pipeline_layout, nullptr);
        vkDestroyPipeline(raw_device->device, pipeline, nullptr);
    }

} // namespace RHI
} // namespace Squid

#pragma once
#include <vector>
#include <array>
#include <Core/Random.h>

namespace Squid {
namespace RHI {

    // Enumerations

    enum class ResourceView : u8 {
        SRV,
        UAV,
        RTV,
        DSV,
    };

    enum class ImageLayout : u8 {
        UNDEFINED,              // discard contents
        GENERAL,                // supports everything
        RENDER_TARGET,          // render target, write enabled
        DEPTH_STENCIL,          // depth stencil, write enabled
        DEPTH_STENCIL_READONLY, // depth stencil, read only
        SHADER_RESOURCE,        // shader resource, read only
        SHADER_RESOURCE_READONLY,
        UNORDERED_ACCESS,       // shader resource, write enabled
        TRANSFER_SRC,           // copy from
        TRANSFER_DST,           // copy to
    };

    enum Format : u16 {
        FORMAT_UNKNOWN,

        // 4 comp 32bit
        FORMAT_R32G32B32A32_FLOAT,
        FORMAT_R32G32B32A32_UINT,
        FORMAT_R32G32B32A32_SINT,

        // 3 comp 32bit
        FORMAT_R32G32B32_FLOAT,
        FORMAT_R32G32B32_UINT,
        FORMAT_R32G32B32_SINT,

        // 4 comp 16bit
        FORMAT_R16G16B16A16_FLOAT,
        FORMAT_R16G16B16A16_UNORM,
        FORMAT_R16G16B16A16_UINT,
        FORMAT_R16G16B16A16_SNORM,
        FORMAT_R16G16B16A16_SINT,

        // 3 comp 16bit
        FORMAT_R16G16B16_FLOAT,

        // 2 comp 32bit
        FORMAT_R32G32_FLOAT,
        FORMAT_R32G32_UINT,
        FORMAT_R32G32_SINT,

        // depth + stencil (alias)
        FORMAT_R32G8X24_TYPELESS,
        // depth + stencil
        FORMAT_D32_FLOAT_S8X24_UINT,

        FORMAT_R10G10B10A2_UNORM,
        FORMAT_R10G10B10A2_UINT,
        FORMAT_R11G11B10_FLOAT,
        
        FORMAT_R8G8B8A8_UNORM,
        FORMAT_R8G8B8A8_UNORM_SRGB,
        FORMAT_R8G8B8A8_UINT,
        FORMAT_R8G8B8A8_SNORM,
        FORMAT_R8G8B8A8_SINT,
        FORMAT_B8G8R8A8_UNORM,
        FORMAT_B8G8R8A8_UNORM_SRGB,
        
        FORMAT_R16G16_FLOAT,
        FORMAT_R16G16_UNORM,
        FORMAT_R16G16_UINT,
        FORMAT_R16G16_SNORM,
        FORMAT_R16G16_SINT,
        FORMAT_R32_FLOAT,
        FORMAT_R32_UINT,
        FORMAT_R32_SINT,
        
        FORMAT_R32_TYPELESS, // depth (alias)
        FORMAT_D32_FLOAT,    // depth
        FORMAT_R24G8_TYPELESS,    // depth + stencil (alias)
        FORMAT_D24_UNORM_S8_UINT, // depth + stencil
        FORMAT_D16_UNORM, // depth
        FORMAT_R16_TYPELESS, // depth (alias)

        FORMAT_R8G8_UNORM,
        FORMAT_R8G8_UINT,
        FORMAT_R8G8_SNORM,
        FORMAT_R8G8_SINT,

        FORMAT_R16_FLOAT,
        FORMAT_R16_UNORM,
        FORMAT_R16_UINT,
        FORMAT_R16_SNORM,
        FORMAT_R16_SINT,

        FORMAT_R8_UNORM,
        FORMAT_R8_UINT,
        FORMAT_R8_SNORM,
        FORMAT_R8_SINT,

        FORMAT_BC1_UNORM,
        FORMAT_BC1_UNORM_SRGB,
        FORMAT_BC2_UNORM,
        FORMAT_BC2_UNORM_SRGB,
        FORMAT_BC3_UNORM,
        FORMAT_BC3_UNORM_SRGB,
        FORMAT_BC4_UNORM,
        FORMAT_BC4_SNORM,
        FORMAT_BC5_UNORM,
        FORMAT_BC5_SNORM,
        FORMAT_BC6H_UF16,
        FORMAT_BC6H_SF16,
        FORMAT_BC7_UNORM,
        FORMAT_BC7_UNORM_SRGB
    };

    /*enum Usage : uint8_t {
        USAGE_DEFAULT,
        USAGE_IMMUTABLE,
        USAGE_DYNAMIC,
        USAGE_STAGING,
    };*/

    enum class IndexFormat : u8 {
        INDEX_16BIT,
        INDEX_32BIT,
    };

    enum ShaderStage : u8 {
        SHADER_STAGE_COMPUTE_STAGE = 1 << 1,
        SHADER_STAGE_VERTEX_STAGE = 1 << 2,
        SHADER_STAGE_PIXEL_STAGE = 1 << 3,
        SHADER_STAGE_HULL_STAGE = 1 << 4,
        SHADER_STAGE_DOMAIN_STAGE = 1 << 5
    };

    enum class Blend : u8 {
        ZERO,
        ONE,
        SRC_COLOR,
        INV_SRC_COLOR,
        SRC_ALPHA,
        INV_SRC_ALPHA,
        DEST_ALPHA,
        INV_DEST_ALPHA,
        DEST_COLOR,
        INV_DEST_COLOR,
        SRC_ALPHA_SAT,
        BLEND_FACTOR,
        INV_BLEND_FACTOR,
        SRC1_COLOR,
        INV_SRC1_COLOR,
        SRC1_ALPHA,
        INV_SRC1_ALPHA
    };

    enum class BlendOp : u8 { ADD, SUBTRACT, REV_SUBTRACT, MIN, MAX };

    enum class CullMode : u8 {
        NONE,
        FRONT,
        BACK,
    };

    enum class CompareOp : u8 {
        NEVER,
        LESS,
        EQUAL,
        LESS_EQUAL,
        GREATER,
        NOT_EQUAL,
        GREATER_EQUAL,
        ALWAYS,
    };

    // Device handles

    struct Handle {
        Handle() : id(RANDOM_32) {}
        uint32_t id;
    };

    static constexpr uint32_t INVALID_HANDLE_ID = 0;

    struct BufferHandle : Handle {
        enum Usage : uint8_t {
            VERTEX_BUFFER = 1 << 1,
            STORAGE_BUFFER = 1 << 2,
            INDEX_BUFFER = 1 << 3,
            UNIFORM_BUFFER = 1 << 4,
            TRANSFER_SRC = 1 << 5,
            TRANSFER_DST = 1 << 6,
            ALL = VERTEX_BUFFER | INDEX_BUFFER | STORAGE_BUFFER | UNIFORM_BUFFER | TRANSFER_DST | TRANSFER_SRC
        };

        Usage usage;
        uint64_t size;
        bool cpu_access;
    };

    struct TextureHandle : Handle {

        enum class Type : uint8_t {
            TEXTURE_1D,
            TEXTURE_2D,
            TEXTURE_3D,
            TEXTURE_CUBE,
        } type = Type::TEXTURE_2D;

        enum Usage : uint8_t {
            RENDER_TARGET_VIEW = 1 << 1,
            UNORDERED_ACCESS_VIEW = 1 << 2,
            SHADER_RESOURCE_VIEW = 1 << 3,
            DEPTH_STENCIL_VIEW = 1 << 4
            // SAMPLED = 1 << 4,
            // TRANSFER_SRC = 1 << 5,
            // TRANSFER_DST = 1 << 6
        };

        // Tiling tiling = Tiling::OPTIMAL;
        Format format = FORMAT_UNKNOWN;
        ImageLayout layout = ImageLayout::UNDEFINED;

        uint32_t usage_flags = 0;
        uint64_t size = 0;

        uint32_t width = 0;
        uint32_t height = 0;
        uint8_t depth = 1;

        uint8_t layers = 1;
        uint8_t mip_levels = 1;
        uint8_t sample_count = 1;
    };

    struct RenderPassAttachment {
        // int subresource = -1;

        enum Type {
            COLOR_ATTACHMENT,
            DEPTH_STENCIL_ATTACHMENT,
        } type = COLOR_ATTACHMENT;

        enum LoadOp {
            LOAD_OP_LOAD,
            LOAD_OP_CLEAR,
            LOAD_OP_DONTCARE,
        } load_op = LOAD_OP_LOAD;

        const TextureHandle *texture = nullptr;

        enum StoreOp {
            STORE_OP_STORE,
            STORE_OP_DONTCARE,
        } store_op = STORE_OP_STORE;

        ImageLayout initial_layout = ImageLayout::GENERAL;
        ImageLayout final_layout = ImageLayout::GENERAL;
    };

    struct RenderPassHandle : Handle {
        uint32_t num_attachments = 0;
        // 8 color + 1 depth
        RenderPassAttachment color[8] = {};
        RenderPassAttachment ds;
    };

    struct Descriptor {
        enum Type : uint8_t { Storage = 1, Uniform = 2, Sampler = 3 } type;
        u32 shader_stage;
        uint16_t count;
        uint16_t binding;
    };

    struct DescriptorSetHandle : Handle {
        std::vector<Descriptor> descriptors;
    };

    struct ComputePipelineHandle : Handle {
        std::vector<DescriptorSetHandle> descriptor_sets;
        std::string compute_shader;
    };

    enum class PrimitiveTopology {
        TRIANGLE_LIST = 1,
        TRIANGLE_STRIP = 2,
        TRIANGLE_FAN = 3,
    };

    enum class VertexType : uint8_t { FLOAT, FLOAT2, FLOAT3, FLOAT4 };

    struct VertexInput {
        VertexType type;     // 1 byte
        uint8_t binding = 0; // 1 byte
        uint16_t offset = 0; // 2 byte
    };

    static_assert(sizeof(VertexInput) == 4);

    struct VertexLayout {
        size_t input_count = 0;
        std::array<VertexInput, 32> inputs;
        uint32_t stride;
    };

    struct RenderTargetBlend {
        Blend src_blend = Blend::ONE;
        Blend dst_blend = Blend::ZERO;
        BlendOp blend_op = BlendOp::ADD;

        Blend src_blend_alpha = Blend::ONE;
        Blend dst_blend_alpha = Blend::ZERO;
        BlendOp blend_op_alpha = BlendOp::ADD;

        int write_mask = 0;
    };

    struct BlendState {
        bool enabled = true;
        bool alpha_to_coverage = false;
        RenderTargetBlend rt_blends[8];
    };

    struct PipelineResource {
        enum Type : u8 { StorageBuffer = 1, UniformBuffer = 2, Sampler = 3 } type;
        u32 shader_stage;
        u16 count;
        u16 binding;
    };

    struct PipelineResourcesHandle : Handle {
        std::vector<PipelineResource> descriptors;
    };

    struct GraphicsPipelineHandle : Handle {
        VertexLayout vertex_layout;
        PrimitiveTopology topology = PrimitiveTopology::TRIANGLE_LIST;

        CullMode cull_mode = CullMode::BACK; 
        CompareOp compare_op = CompareOp::ALWAYS; 
        bool primitive_restart = false;
        bool stencil_test = true;
        bool depth_test = true;
        bool depth_write = true;

        BlendState blend_state;

        std::string pixel_shader;
        std::string vertex_shader;

        std::vector<DescriptorSetHandle> descriptor_sets;
        RenderPassHandle *render_pass = nullptr;
    };

    struct RenderTargetHandle : Handle {
        bool _offscreen = true;
    };

    struct SwapchainHandle : Handle {
        void *window_handle;
        RenderTargetHandle backbuffer;
    };

} // namespace RHI
} // namespace Squid

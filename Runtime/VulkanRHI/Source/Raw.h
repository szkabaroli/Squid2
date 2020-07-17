#pragma once
#include <pch.h>

#define VK_DEBUG_OBJECT(device, vk_object, vk_objectType, name)                                                        \
    {                                                                                                                  \
        auto vkSetDebugUtilsObjectNameEXT =                                                                            \
            (PFN_vkSetDebugUtilsObjectNameEXT)vkGetDeviceProcAddr(device, "vkSetDebugUtilsObjectNameEXT");             \
        VkDebugUtilsObjectNameInfoEXT name_info = {};                                                                  \
        name_info.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_OBJECT_NAME_INFO_EXT;                                          \
        name_info.objectType = vk_objectType;                                                                          \
        name_info.objectHandle = (uint64_t)vk_object;                                                                  \
        name_info.pObjectName = name;                                                                                  \
        vkSetDebugUtilsObjectNameEXT(device, &name_info);                                                              \
    }

namespace Squid {
namespace RHI {

    constexpr VkFormat ConvertFormat(Format format) {
        switch (format) {
        case FORMAT_UNKNOWN:
            return VK_FORMAT_UNDEFINED;
            break;
        case FORMAT_R32G32B32A32_FLOAT:
            return VK_FORMAT_R32G32B32A32_SFLOAT;
            break;
        case FORMAT_R32G32B32A32_UINT:
            return VK_FORMAT_R32G32B32A32_UINT;
            break;
        case FORMAT_R32G32B32A32_SINT:
            return VK_FORMAT_R32G32B32A32_SINT;
            break;
        case FORMAT_R32G32B32_FLOAT:
            return VK_FORMAT_R32G32B32_SFLOAT;
            break;
        case FORMAT_R32G32B32_UINT:
            return VK_FORMAT_R32G32B32_UINT;
            break;
        case FORMAT_R32G32B32_SINT:
            return VK_FORMAT_R32G32B32_SINT;
            break;
        case FORMAT_R16G16B16A16_FLOAT:
            return VK_FORMAT_R16G16B16A16_SFLOAT;
            break;
        case FORMAT_R16G16B16A16_UNORM:
            return VK_FORMAT_R16G16B16A16_UNORM;
            break;
        case FORMAT_R16G16B16A16_UINT:
            return VK_FORMAT_R16G16B16A16_UINT;
            break;
        case FORMAT_R16G16B16A16_SNORM:
            return VK_FORMAT_R16G16B16A16_SNORM;
            break;
        case FORMAT_R16G16B16A16_SINT:
            return VK_FORMAT_R16G16B16A16_SINT;
            break;
        case FORMAT_R32G32_FLOAT:
            return VK_FORMAT_R32G32_SFLOAT;
            break;
        case FORMAT_R32G32_UINT:
            return VK_FORMAT_R32G32_UINT;
            break;
        case FORMAT_R32G32_SINT:
            return VK_FORMAT_R32G32_SINT;
            break;
        case FORMAT_R32G8X24_TYPELESS:
            return VK_FORMAT_D32_SFLOAT_S8_UINT;
            break;
        case FORMAT_D32_FLOAT_S8X24_UINT:
            return VK_FORMAT_D32_SFLOAT_S8_UINT;
            break;
        case FORMAT_R10G10B10A2_UNORM:
            return VK_FORMAT_A2B10G10R10_UNORM_PACK32;
            break;
        case FORMAT_R10G10B10A2_UINT:
            return VK_FORMAT_A2B10G10R10_UINT_PACK32;
            break;
        case FORMAT_R11G11B10_FLOAT:
            return VK_FORMAT_B10G11R11_UFLOAT_PACK32;
            break;
        case FORMAT_R8G8B8A8_UNORM:
            return VK_FORMAT_R8G8B8A8_UNORM;
            break;
        case FORMAT_R8G8B8A8_UNORM_SRGB:
            return VK_FORMAT_R8G8B8A8_SRGB;
            break;
        case FORMAT_R8G8B8A8_UINT:
            return VK_FORMAT_R8G8B8A8_UINT;
            break;
        case FORMAT_R8G8B8A8_SNORM:
            return VK_FORMAT_R8G8B8A8_SNORM;
            break;
        case FORMAT_R8G8B8A8_SINT:
            return VK_FORMAT_R8G8B8A8_SINT;
            break;
        case FORMAT_R16G16_FLOAT:
            return VK_FORMAT_R16G16_SFLOAT;
            break;
        case FORMAT_R16G16_UNORM:
            return VK_FORMAT_R16G16_UNORM;
            break;
        case FORMAT_R16G16_UINT:
            return VK_FORMAT_R16G16_UINT;
            break;
        case FORMAT_R16G16_SNORM:
            return VK_FORMAT_R16G16_SNORM;
            break;
        case FORMAT_R16G16_SINT:
            return VK_FORMAT_R16G16_SINT;
            break;
        case FORMAT_R16G16B16_FLOAT:
            return VK_FORMAT_R16G16B16_SFLOAT;
            break;
        case FORMAT_R32_TYPELESS:
            return VK_FORMAT_D32_SFLOAT;
            break;
        case FORMAT_D32_FLOAT:
            return VK_FORMAT_D32_SFLOAT;
            break;
        case FORMAT_R32_FLOAT:
            return VK_FORMAT_R32_SFLOAT;
            break;
        case FORMAT_R32_UINT:
            return VK_FORMAT_R32_UINT;
            break;
        case FORMAT_R32_SINT:
            return VK_FORMAT_R32_SINT;
            break;
        case FORMAT_R24G8_TYPELESS:
            return VK_FORMAT_D24_UNORM_S8_UINT;
            break;
        case FORMAT_D24_UNORM_S8_UINT:
            return VK_FORMAT_D24_UNORM_S8_UINT;
            break;
        case FORMAT_R8G8_UNORM:
            return VK_FORMAT_R8G8_UNORM;
            break;
        case FORMAT_R8G8_UINT:
            return VK_FORMAT_R8G8_UINT;
            break;
        case FORMAT_R8G8_SNORM:
            return VK_FORMAT_R8G8_SNORM;
            break;
        case FORMAT_R8G8_SINT:
            return VK_FORMAT_R8G8_SINT;
            break;
        case FORMAT_R16_TYPELESS:
            return VK_FORMAT_D16_UNORM;
            break;
        case FORMAT_R16_FLOAT:
            return VK_FORMAT_R16_SFLOAT;
            break;
        case FORMAT_D16_UNORM:
            return VK_FORMAT_D16_UNORM;
            break;
        case FORMAT_R16_UNORM:
            return VK_FORMAT_R16_UNORM;
            break;
        case FORMAT_R16_UINT:
            return VK_FORMAT_R16_UINT;
            break;
        case FORMAT_R16_SNORM:
            return VK_FORMAT_R16_SNORM;
            break;
        case FORMAT_R16_SINT:
            return VK_FORMAT_R16_SINT;
            break;
        case FORMAT_R8_UNORM:
            return VK_FORMAT_R8_UNORM;
            break;
        case FORMAT_R8_UINT:
            return VK_FORMAT_R8_UINT;
            break;
        case FORMAT_R8_SNORM:
            return VK_FORMAT_R8_SNORM;
            break;
        case FORMAT_R8_SINT:
            return VK_FORMAT_R8_SINT;
            break;
        case FORMAT_BC1_UNORM:
            return VK_FORMAT_BC1_RGBA_UNORM_BLOCK;
            break;
        case FORMAT_BC1_UNORM_SRGB:
            return VK_FORMAT_BC1_RGBA_SRGB_BLOCK;
            break;
        case FORMAT_BC2_UNORM:
            return VK_FORMAT_BC2_UNORM_BLOCK;
            break;
        case FORMAT_BC2_UNORM_SRGB:
            return VK_FORMAT_BC2_SRGB_BLOCK;
            break;
        case FORMAT_BC3_UNORM:
            return VK_FORMAT_BC3_UNORM_BLOCK;
            break;
        case FORMAT_BC3_UNORM_SRGB:
            return VK_FORMAT_BC3_SRGB_BLOCK;
            break;
        case FORMAT_BC4_UNORM:
            return VK_FORMAT_BC4_UNORM_BLOCK;
            break;
        case FORMAT_BC4_SNORM:
            return VK_FORMAT_BC4_SNORM_BLOCK;
            break;
        case FORMAT_BC5_UNORM:
            return VK_FORMAT_BC5_UNORM_BLOCK;
            break;
        case FORMAT_BC5_SNORM:
            return VK_FORMAT_BC5_SNORM_BLOCK;
            break;
        case FORMAT_B8G8R8A8_UNORM:
            return VK_FORMAT_B8G8R8A8_UNORM;
            break;
        case FORMAT_B8G8R8A8_UNORM_SRGB:
            return VK_FORMAT_B8G8R8A8_SRGB;
            break;
        case FORMAT_BC6H_UF16:
            return VK_FORMAT_BC6H_UFLOAT_BLOCK;
            break;
        case FORMAT_BC6H_SF16:
            return VK_FORMAT_BC6H_SFLOAT_BLOCK;
            break;
        case FORMAT_BC7_UNORM:
            return VK_FORMAT_BC7_UNORM_BLOCK;
            break;
        case FORMAT_BC7_UNORM_SRGB:
            return VK_FORMAT_BC7_SRGB_BLOCK;
            break;
        }
        return VK_FORMAT_UNDEFINED;
    }

    constexpr VkImageLayout ConvertImageLayout(ImageLayout layout) {
        switch (layout) {
        case ImageLayout::GENERAL:
            return VK_IMAGE_LAYOUT_GENERAL;
        case ImageLayout::RENDER_TARGET:
            return VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
        case ImageLayout::DEPTH_STENCIL:
            return VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
        case ImageLayout::DEPTH_STENCIL_READONLY:
            return VK_IMAGE_LAYOUT_DEPTH_STENCIL_READ_ONLY_OPTIMAL;
        case ImageLayout::SHADER_RESOURCE:
            return VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
        case ImageLayout::UNORDERED_ACCESS:
            return VK_IMAGE_LAYOUT_GENERAL;
        case ImageLayout::TRANSFER_SRC:
            return VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL;
        case ImageLayout::TRANSFER_DST:
            return VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
        }
        return VK_IMAGE_LAYOUT_UNDEFINED;
    }

    constexpr VkAccessFlags ConvertImageLayoutToAccess(ImageLayout layout) {
        VkAccessFlags flags = 0;

        switch (layout) {
        case ImageLayout::GENERAL:
            flags |= VK_ACCESS_SHADER_READ_BIT;
            flags |= VK_ACCESS_SHADER_WRITE_BIT;
            flags |= VK_ACCESS_TRANSFER_READ_BIT;
            flags |= VK_ACCESS_TRANSFER_WRITE_BIT;
            flags |= VK_ACCESS_MEMORY_READ_BIT;
            flags |= VK_ACCESS_MEMORY_WRITE_BIT;
            break;
        case ImageLayout::RENDER_TARGET:
            flags |= VK_ACCESS_SHADER_WRITE_BIT;
            break;
        case ImageLayout::DEPTH_STENCIL:
            flags |= VK_ACCESS_SHADER_WRITE_BIT;
            break;
        case ImageLayout::DEPTH_STENCIL_READONLY:
            flags |= VK_ACCESS_SHADER_READ_BIT;
            break;
        case ImageLayout::SHADER_RESOURCE:
            flags |= VK_ACCESS_SHADER_READ_BIT;
            break;
        case ImageLayout::UNORDERED_ACCESS:
            flags |= VK_ACCESS_SHADER_READ_BIT;
            flags |= VK_ACCESS_SHADER_WRITE_BIT;
            break;
        case ImageLayout::TRANSFER_SRC:
            flags |= VK_ACCESS_TRANSFER_READ_BIT;
            break;
        case ImageLayout::TRANSFER_DST:
            flags |= VK_ACCESS_TRANSFER_WRITE_BIT;
            break;
        }

        return flags;
    }

    constexpr VkAccessFlags ConvertBufferUsageToAccess() {
        VkAccessFlags flags = 0;
        return flags;
    }

    constexpr VkBlendFactor ConvertBlend(Blend value) {
        switch (value) {
        case Blend::ZERO:
            return VK_BLEND_FACTOR_ZERO;
        case Blend::ONE:
            return VK_BLEND_FACTOR_ONE;
        case Blend::SRC_COLOR:
            return VK_BLEND_FACTOR_SRC_COLOR;
        case Blend::SRC_ALPHA:
            return VK_BLEND_FACTOR_SRC_ALPHA;
        case Blend::SRC_ALPHA_SAT:
            return VK_BLEND_FACTOR_SRC_ALPHA_SATURATE;
        case Blend::SRC1_COLOR:
            return VK_BLEND_FACTOR_SRC1_COLOR;
        case Blend::SRC1_ALPHA:
            return VK_BLEND_FACTOR_SRC1_ALPHA;
        case Blend::INV_SRC_COLOR:
            return VK_BLEND_FACTOR_ONE_MINUS_SRC_COLOR;
        case Blend::INV_SRC_ALPHA:
            return VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA;
        case Blend::INV_SRC1_COLOR:
            return VK_BLEND_FACTOR_ONE_MINUS_SRC1_COLOR;
        case Blend::INV_SRC1_ALPHA:
            return VK_BLEND_FACTOR_ONE_MINUS_SRC1_ALPHA;
        case Blend::DEST_COLOR:
            return VK_BLEND_FACTOR_DST_COLOR;
        case Blend::DEST_ALPHA:
            return VK_BLEND_FACTOR_DST_ALPHA;
        }

        assert(0 && "Unsupported Blend");
    }

    constexpr VkBlendOp ConvertBlendOp(BlendOp value) {
        switch (value) {
        case BlendOp::ADD:
            return VK_BLEND_OP_ADD;
        case BlendOp::SUBTRACT:
            return VK_BLEND_OP_SUBTRACT;
        case BlendOp::REV_SUBTRACT:
            return VK_BLEND_OP_REVERSE_SUBTRACT;
        case BlendOp::MIN:
            return VK_BLEND_OP_MIN;
        case BlendOp::MAX:
            return VK_BLEND_OP_MAX;
        }

        assert(0 && "Unsupported BlendOP");
    }

    constexpr VkCullModeFlagBits ConvertCullMode(CullMode value) {
        switch (value) {
        case CullMode::FRONT:
            return VK_CULL_MODE_FRONT_BIT;
        case CullMode::BACK:
            return VK_CULL_MODE_BACK_BIT;
        case CullMode::NONE:
            return VK_CULL_MODE_NONE;
        }

        assert(0 && "Unsupported CullMode");
    }

    constexpr VkCompareOp ConvertCompareOp(CompareOp value) {
        switch (value) {
        case CompareOp::NEVER:
            return VK_COMPARE_OP_NEVER;
        case CompareOp::EQUAL:
            return VK_COMPARE_OP_EQUAL;
        case CompareOp::NOT_EQUAL:
            return VK_COMPARE_OP_NOT_EQUAL;
        case CompareOp::ALWAYS:
            return VK_COMPARE_OP_ALWAYS;
        case CompareOp::GREATER:
            return VK_COMPARE_OP_GREATER;
        case CompareOp::GREATER_EQUAL:
            return VK_COMPARE_OP_GREATER_OR_EQUAL;
        case CompareOp::LESS:
            return VK_COMPARE_OP_LESS;
        case CompareOp::LESS_EQUAL:
            return VK_COMPARE_OP_LESS_OR_EQUAL;
        }

        assert(0 && "Unsupported CompareOp");
    }

    struct RawInstance {
        VkInstance instance;
        RawInstance(VkInstance instance) : instance(instance) {}
        ~RawInstance() {
            LOG_INFO("destroying raw vulkan instance");
            vkDestroyInstance(instance, nullptr);
        }
    };

    struct RawDevice {
        VkDevice device;
        VkPhysicalDevice physical;
        VmaAllocator allocator;

        VkPhysicalDeviceMaintenance3Properties maintance3_props;
        VkPhysicalDeviceRayTracingPropertiesKHR ray_tracing_props;

        ~RawDevice() {
            LOG_INFO("destroying raw vulkan device and memory allocator");
            vmaDestroyAllocator(allocator);
            vkDestroyDevice(device, nullptr);
        }
    };

} // namespace RHI
} // namespace Squid

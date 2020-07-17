#pragma once
#include "Buffer.h"
#include "Raw.h"
#include "Texture.h"
#include <cassert>
#include <pch.h>

namespace Squid {
namespace RHI {

    class VulkanDescriptorSet {
    public:
        VulkanDescriptorSet(const DescriptorSetHandle &handle, std::shared_ptr<RawDevice> raw);
        ~VulkanDescriptorSet();

        void SetBuffer(uint32_t binding, VulkanBuffer *buffer);
        void SetTexture(uint32_t binding, const TextureHandle &handle);
        // void Free(VkDescriptorSet descriptor_set);
        void Update(uint32_t index, std::unordered_map<uint64_t, std::unique_ptr<VulkanTexture>> &textures);
        VkDescriptorSet GetDescriptorSet(uint32_t index, std::unordered_map<uint64_t, std::unique_ptr<VulkanTexture>> &textures);

        inline VkDescriptorSetLayout GetLayout() const { return descriptor_layout; }

    private:
        static constexpr uint32_t descriptor_set_num = 3;
        static constexpr uint32_t MAX_BINDING = 10;

        VkDescriptorPool descriptor_pool;
        

        // uint64_t buffer_bindings[MAX_BINDING];
        // uint64_t texture_bindings[MAX_BINDING];

        bool dirty_sets[descriptor_set_num] = {true, true, true};

        VkDescriptorSet descriptor_sets[3];
        VkDescriptorSetLayout descriptor_layout;

        std::unordered_map<uint32_t, VulkanBuffer *> buffer_bindings;
        std::unordered_map<uint32_t, uint32_t> texture_bindings;
        std::shared_ptr<RawDevice> raw_device;
        VulkanDevice *device;
    };

} // namespace RHI
} // namespace Squid
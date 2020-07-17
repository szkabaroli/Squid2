#include "DescriptorSet.h"

namespace Squid {
namespace RHI {

    VulkanDescriptorSet::VulkanDescriptorSet(
        const DescriptorSetHandle &handle, std::shared_ptr<RawDevice> raw)
        : raw_device(raw) {
        uint32_t storage_count = 0;
        uint32_t uniform_count = 0;
        uint32_t sampler_count = 0;

        std::vector<VkDescriptorSetLayoutBinding> descriptor_bindings;
        descriptor_bindings.reserve(handle.descriptors.size());

        for (const auto &descriptor : handle.descriptors) {

            VkDescriptorSetLayoutBinding descriptor_binding = {};
            descriptor_binding.binding = descriptor.binding;
            descriptor_binding.descriptorCount = descriptor.count;
            descriptor_binding.pImmutableSamplers = nullptr;
            descriptor_binding.stageFlags = 0;

            if (descriptor.shader_stage & SHADER_STAGE_PIXEL_STAGE)
                descriptor_binding.stageFlags |= VK_SHADER_STAGE_FRAGMENT_BIT;

            if (descriptor.shader_stage & SHADER_STAGE_VERTEX_STAGE)
                descriptor_binding.stageFlags |= VK_SHADER_STAGE_VERTEX_BIT;

            if (descriptor.shader_stage & SHADER_STAGE_COMPUTE_STAGE)
                descriptor_binding.stageFlags |= VK_SHADER_STAGE_COMPUTE_BIT;

            // Calc descripor pool sizes
            switch (descriptor.type) {
            case Descriptor::Type::Storage:
                descriptor_binding.descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
                storage_count++;
                break;
            case Descriptor::Type::Uniform:
                descriptor_binding.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
                uniform_count++;
                break;
            case Descriptor::Type::Sampler:
                descriptor_binding.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
                sampler_count++;
                break;
            }

            descriptor_bindings.push_back(descriptor_binding);
        }

        VkDescriptorSetLayoutCreateInfo descriptor_set_layout_info = {};
        descriptor_set_layout_info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
        descriptor_set_layout_info.pBindings = descriptor_bindings.data();
        descriptor_set_layout_info.bindingCount = static_cast<uint32_t>(descriptor_bindings.size());

        vkCreateDescriptorSetLayout(raw_device->device, &descriptor_set_layout_info, nullptr, &descriptor_layout);

        // Descriptor pool
        // TODO: Correct size
        std::vector<VkDescriptorPoolSize> pool_sizes;
        if (storage_count != 0) {
            VkDescriptorPoolSize pool_size = {};
            pool_size.type = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
            pool_size.descriptorCount = storage_count * descriptor_set_num;
            pool_sizes.push_back(pool_size);
        }

        if (uniform_count != 0) {
            VkDescriptorPoolSize pool_size = {};
            pool_size.type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
            pool_size.descriptorCount = uniform_count * descriptor_set_num;
            pool_sizes.push_back(pool_size);
        }

        if (sampler_count != 0) {
            VkDescriptorPoolSize pool_size = {};
            pool_size.type = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
            pool_size.descriptorCount = sampler_count * descriptor_set_num;
            pool_sizes.push_back(pool_size);
        }

        VkDescriptorPoolCreateInfo descriptor_pool_info = {};
        descriptor_pool_info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
        descriptor_pool_info.pPoolSizes = pool_sizes.data();
        descriptor_pool_info.poolSizeCount = static_cast<uint32_t>(pool_sizes.size());
        descriptor_pool_info.maxSets = descriptor_set_num;
        descriptor_pool_info.flags = 0;

        vkCreateDescriptorPool(raw_device->device, &descriptor_pool_info, nullptr, &descriptor_pool);

        // Allocate number or sets
        VkDescriptorSetLayout layouts[3] = {descriptor_layout, descriptor_layout, descriptor_layout};
        VkDescriptorSetAllocateInfo alloc_info = {};
        alloc_info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
        alloc_info.descriptorPool = descriptor_pool;
        alloc_info.descriptorSetCount = 3;
        alloc_info.pSetLayouts = layouts;
        vkAllocateDescriptorSets(raw_device->device, &alloc_info, descriptor_sets);
    }

    VulkanDescriptorSet::~VulkanDescriptorSet() {
        LOG("destroying descriptor pool and sets")

        vkFreeDescriptorSets(raw_device->device, descriptor_pool, descriptor_set_num, descriptor_sets);
        for (auto i = 0; i < descriptor_set_num; i++) {
            vkDestroyDescriptorSetLayout(raw_device->device, descriptor_layout, nullptr);
        }
        vkDestroyDescriptorPool(raw_device->device, descriptor_pool, nullptr);
    }

    void VulkanDescriptorSet::Update(uint32_t index, std::unordered_map<uint64_t, std::unique_ptr<VulkanTexture>> &textures) {
        if (dirty_sets[index]) {
            std::vector<VkWriteDescriptorSet> writes;

            for (const auto binding : buffer_bindings) {
                VkDescriptorBufferInfo descriptor_buffer_info = {};
                descriptor_buffer_info.buffer = binding.second->GetBuffer();
                descriptor_buffer_info.offset = 0;
                descriptor_buffer_info.range = binding.second->GetSize();

                VkWriteDescriptorSet descriptor_write = {};
                descriptor_write.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
                descriptor_write.dstBinding = binding.first;
                descriptor_write.dstArrayElement = 0;
                descriptor_write.dstSet = descriptor_sets[index];
                descriptor_write.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
                descriptor_write.descriptorCount = 1;
                descriptor_write.pBufferInfo = &descriptor_buffer_info;
                descriptor_write.pImageInfo = nullptr;
                descriptor_write.pTexelBufferView = nullptr;

                writes.push_back(descriptor_write);
            }

            for (const auto binding : texture_bindings) {
                VkDescriptorImageInfo descriptor_image_info = {};
                descriptor_image_info.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
                descriptor_image_info.imageView = textures[binding.second]->GetView();
                descriptor_image_info.sampler = textures[binding.second]->GetSampler();

                VkWriteDescriptorSet descriptor_write = {};
                descriptor_write.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
                descriptor_write.dstBinding = binding.first;
                descriptor_write.dstArrayElement = 0;
                descriptor_write.dstSet = descriptor_sets[index];
                descriptor_write.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
                descriptor_write.descriptorCount = 1;
                descriptor_write.pBufferInfo = nullptr;
                descriptor_write.pImageInfo = &descriptor_image_info;
                descriptor_write.pTexelBufferView = nullptr;

                writes.push_back(descriptor_write);

                dirty_sets[index] = false;
            }

            vkUpdateDescriptorSets(raw_device->device, static_cast<uint32_t>(writes.size()), writes.data(), 0, nullptr);
        }
    };

    VkDescriptorSet VulkanDescriptorSet::GetDescriptorSet(uint32_t index, std::unordered_map<uint64_t, std::unique_ptr<VulkanTexture>> &textures) {
        if (dirty_sets[index]) {

            std::vector<VkDescriptorBufferInfo> buffer_infos;
            for (const auto binding : buffer_bindings) {
                VkDescriptorBufferInfo buffer_info = {};
                buffer_info.buffer = binding.second->GetBuffer();
                buffer_info.offset = 0;
                buffer_info.range = binding.second->GetSize();
                buffer_infos.push_back(buffer_info);
            }

            std::vector<VkDescriptorImageInfo> image_infos;
            for (const auto binding : texture_bindings) {
                VkDescriptorImageInfo image_info = {};
                image_info.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
                image_info.imageView = textures[binding.second]->GetView();
                image_info.sampler = textures[binding.second]->GetSampler();
                image_infos.push_back(image_info);
                dirty_sets[index] = false;
            }

            std::vector<VkWriteDescriptorSet> writes;

            if (buffer_infos.size() > 0) {
                VkWriteDescriptorSet descriptor_write = {};
                descriptor_write.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
                descriptor_write.dstSet = descriptor_sets[index];
                descriptor_write.dstBinding = 0;
                descriptor_write.dstArrayElement = 0;
                descriptor_write.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
                descriptor_write.descriptorCount = buffer_infos.size();
                descriptor_write.pBufferInfo = buffer_infos.data();
                descriptor_write.pImageInfo = nullptr;
                descriptor_write.pTexelBufferView = nullptr;

                writes.push_back(descriptor_write);
            }

            if (image_infos.size() > 0) {
                VkWriteDescriptorSet descriptor_write = {};
                descriptor_write.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
                descriptor_write.dstSet = descriptor_sets[index];
                descriptor_write.dstBinding = buffer_infos.size();
                descriptor_write.dstArrayElement = 0;
                descriptor_write.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
                descriptor_write.descriptorCount = image_infos.size();
                descriptor_write.pBufferInfo = nullptr;
                descriptor_write.pImageInfo = image_infos.data();
                descriptor_write.pTexelBufferView = nullptr;

                writes.push_back(descriptor_write);
            }

            vkUpdateDescriptorSets(raw_device->device, static_cast<uint32_t>(writes.size()), writes.data(), 0, nullptr);
            dirty_sets[index] = false;
        }

        // Increment current set before returning
        // if (current_set == descriptor_set_num - 1)
        //     current_set = 0;
        // else
        //     current_set++;

        return descriptor_sets[index];
    };

    void VulkanDescriptorSet::SetBuffer(uint32_t binding, VulkanBuffer *buffer) {
        assert(binding < MAX_BINDING);
        buffer_bindings.insert(std::pair(binding, buffer));
        dirty_sets[0] = true;
        dirty_sets[1] = true;
        dirty_sets[2] = true;
    }

    void VulkanDescriptorSet::SetTexture(uint32_t binding, const TextureHandle &handle) {
        assert(binding < MAX_BINDING);
        texture_bindings[binding] = handle.id;
        dirty_sets[0] = true;
        dirty_sets[1] = true;
        dirty_sets[2] = true;
    }

} // namespace RHI
} // namespace Squid

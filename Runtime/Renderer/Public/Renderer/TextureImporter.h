#pragma once
#include <RHI/Module.h>
#include <Core/Profiling.h>
#include <string>

#include <stb_image.h>

namespace Squid {
namespace Renderer {

    class TextureImporter {
    private:
        RHI::CommandList transfer_list;
        RHI::Device *device;
        std::vector<RHI::BufferHandle> staging_buffers;

    public:
        TextureImporter(RHI::Device *device, RHI::CommandList list) : device(device), transfer_list(list) {}

        RHI::TextureHandle FromFile(
            const std::string &file,
            const std::string &name,
            RHI::Format format = RHI::FORMAT_R8G8B8A8_UNORM_SRGB,
            RHI::TextureHandle::Usage usage = RHI::TextureHandle::Usage::SHADER_RESOURCE_VIEW) {

            PROFILING_SCOPE

            // Load imgage to local memory
            i32 width, height, channels;
            stbi_uc *pixels = stbi_load(file.c_str(), &width, &height, &channels, 4);

            u64 image_size = width * height * 4;

            if (!pixels) {
                throw std::runtime_error("failed to load texture image!");
            }

            // Create a staging cpu accessible GPU buffer
            RHI::BufferHandle staging_texture;
            staging_texture.cpu_access = true;
            staging_texture.size = image_size;
            staging_texture.usage = RHI::BufferHandle::Usage::TRANSFER_SRC;
            device->LoadBuffer(staging_texture);
            staging_buffers.push_back(staging_texture);

            // Fill the staging buffer
            void *texture_data = device->MapBuffer(staging_texture);
            memcpy(texture_data, pixels, image_size);
            device->UnmapBuffer(staging_texture);

            // Free local image memory
            stbi_image_free(pixels);

            // Create device local texture
            RHI::TextureHandle texture;
            texture.height = height;
            texture.width = width;
            texture.depth = 1;
            texture.size = image_size;
            texture.format = format;
            texture.mip_levels = 1;
            texture.sample_count = 1;
            texture.usage_flags = usage;
            device->LoadTexture(texture);
            device->SetName(texture, name);

            // Issue the copy command
            device->Copy(transfer_list, staging_texture, texture);

            return std::move(texture);
        };

        RHI::TextureHandle FromEnvFile(
            const std::array<const std::string, 6> &files,
            const std::string &name,
            RHI::Format format = RHI::FORMAT_R32G32B32A32_FLOAT,
            RHI::TextureHandle::Usage usage = RHI::TextureHandle::Usage::SHADER_RESOURCE_VIEW) {

            PROFILING_SCOPE

            // SPEED: task group
            // Load imgage to local memory
            i32 hdr_width, hdr_height, hdr_channels;

            f32 *face_data_pos_x = stbi_loadf(files[0].c_str(), &hdr_width, &hdr_height, &hdr_channels, 4);
            f32 *face_data_neg_x = stbi_loadf(files[1].c_str(), &hdr_width, &hdr_height, &hdr_channels, 4);
            f32 *face_data_pos_y = stbi_loadf(files[2].c_str(), &hdr_width, &hdr_height, &hdr_channels, 4);
            f32 *face_data_neg_y = stbi_loadf(files[3].c_str(), &hdr_width, &hdr_height, &hdr_channels, 4);
            f32 *face_data_pos_z = stbi_loadf(files[4].c_str(), &hdr_width, &hdr_height, &hdr_channels, 4);
            f32 *face_data_neg_z = stbi_loadf(files[5].c_str(), &hdr_width, &hdr_height, &hdr_channels, 4);

            u64 image_size = (hdr_width * hdr_height * 4);
            u64 image_data_size = image_size * sizeof(float);

            if (!face_data_pos_x[0] || !face_data_neg_x[0]) {
                throw std::runtime_error("failed to load hdr texture image!");
            }

            // Create a staging cpu accessible GPU buffer
            RHI::BufferHandle staging_texture;
            staging_texture.cpu_access = true;
            staging_texture.size = image_data_size * 6;
            staging_texture.usage = RHI::BufferHandle::Usage::TRANSFER_SRC;
            device->LoadBuffer(staging_texture);
            staging_buffers.push_back(staging_texture);

            // Fill the staging buffer
            f32 *texture_hdr_data = (f32 *)device->MapBuffer(staging_texture);
            memcpy(&texture_hdr_data[image_size * 0], face_data_pos_x, image_data_size);
            memcpy(&texture_hdr_data[image_size * 1], face_data_neg_x, image_data_size);
            memcpy(&texture_hdr_data[image_size * 2], face_data_pos_y, image_data_size);
            memcpy(&texture_hdr_data[image_size * 3], face_data_neg_y, image_data_size);
            memcpy(&texture_hdr_data[image_size * 4], face_data_pos_z, image_data_size);
            memcpy(&texture_hdr_data[image_size * 5], face_data_neg_z, image_data_size);
            device->UnmapBuffer(staging_texture);

            // Free local image memory
            stbi_image_free(face_data_pos_x);
            stbi_image_free(face_data_neg_x);
            stbi_image_free(face_data_pos_y);
            stbi_image_free(face_data_neg_y);
            stbi_image_free(face_data_pos_z);
            stbi_image_free(face_data_neg_z);

            // Create device local texture
            RHI::TextureHandle texture;
            texture.height = hdr_height;
            texture.width = hdr_width;
            texture.layers = 6;
            texture.type = RHI::TextureHandle::Type::TEXTURE_CUBE;
            texture.format = format; // 128bit alias 16byte format
            texture.mip_levels = 1;
            texture.sample_count = 1;
            texture.usage_flags = usage;
            device->LoadTexture(texture);
            device->SetName(texture, name);

            // Issue the copy command
            device->Copy(transfer_list, staging_texture, texture, image_data_size);

            return std::move(texture);
        };

        void Upload() {
            PROFILING_SCOPE

            device->QueueSubmit(RHI::QueueType::GRAPHICS, transfer_list);
            for (const auto &buffer : staging_buffers) {
                device->UnloadBuffer(buffer);
            }
        }
    };

} // namespace Renderer
} // namespace Squid
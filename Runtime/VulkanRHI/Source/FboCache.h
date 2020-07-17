#pragma once
#include "Raw.h"
#include <pch.h>
#include <tsl/robin_map.h>
#include <unordered_map>

// File uses code from:
// https://github.com/google/filament/blob/master/filament/backend/src/vulkan/VulkanFboCache.h
/*
 * Copyright (C) 2018 The Android Open Source Project
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

namespace Squid {
namespace RHI {

    enum ClearFlags : uint32_t { ColorClear = 1 << 0, DepthClear = 1 << 1 };

    class VulkanFboCache {
    public:
        // RenderPass
        struct alignas(8) RenderPassKey {
            VkImageLayout final_color_layout; // 4 byte enum
            VkImageLayout final_depth_layout; // 4 byte enum
            VkFormat color_format;            // 4 byte enum
            VkFormat depth_format;            // 4 byte enum
            ClearFlags clear;                 // 4 byte enum
            uint32_t padding0;                // 4 byte padding
        };
        static_assert(sizeof(VkFormat) == 4, "VkFormat has unexpected size.");
        static_assert(sizeof(RenderPassKey) == 24, "RenderPassKey has unexpected size.");

        struct RenderPassVal {
            VkRenderPass handle = VK_NULL_HANDLE;
            uint32_t timestamp;
        };

        using RenderPassHash = MurmurHash<RenderPassKey>;

        struct RenderPassEq {
            bool operator()(const RenderPassKey &k1, const RenderPassKey &k2) const;
        };

        // Framebuffer object
        struct FboKey {
            VkRenderPass render_pass;                      // 8 bytes
            VkImageView attachments[9] = {VK_NULL_HANDLE}; // 72 bytes
            uint32_t width;
            uint32_t height;
        };

        struct FboVal {
            VkFramebuffer handle = VK_NULL_HANDLE;
            uint32_t timestamp;
        };

        using FboKeyHash = MurmurHash<FboKey>;

        struct FboKeyEqualFn {
            bool operator()(const FboKey &k1, const FboKey &k2) const;
        };

        static_assert(sizeof(VkRenderPass) == 8, "VkRenderPass has unexpected size.");
        static_assert(sizeof(VkImageView) == 8, "VkImageView has unexpected size.");
        static_assert(sizeof(FboKey) == 88, "FboKey has unexpected size.");

        // Cache class
        VulkanFboCache(std::shared_ptr<RawDevice> raw_device);
        ~VulkanFboCache();

        void Reset();

        VkFramebuffer GetFramebuffer(const FboKey &key);
        VkRenderPass GetRenderPass(RenderPassKey key);

    private:
        std::shared_ptr<RawDevice> raw_device;

        tsl::robin_map<FboKey, FboVal, FboKeyHash, FboKeyEqualFn> framebuffer_cache = {};
        tsl::robin_map<RenderPassKey, RenderPassVal, RenderPassHash, RenderPassEq> render_pass_cache;
        tsl::robin_map<VkRenderPass, uint32_t> render_pass_ref_count;
    };

} // namespace RHI
} // namespace Squid

#pragma once

#include <array>
#include <iostream>
#include <memory>
#include <set>
#include <string>
#include <map>

#include <Core/Log.h>
#include <Core/Murmur.h>
#include <Core/Random.h>
#include <RHI/Module.h>
#include <unordered_map>
#include <algorithm>

#ifdef SQUID_WIN32
#define VK_USE_PLATFORM_WIN32_KHR
#elif SQUID_LINUX
#define VK_USE_PLATFORM_XCB_KHR
#define VK_USE_PLATFORM_XLIB_KHR
#define VK_USE_PLATFORM_XLIB_XRANDR_EXT
#elif SQUID_APPLE
#define VK_USE_PLATFORM_WAYLAND_KHR
#define VK_USE_PLATFORM_METAL_EXT
#define VK_USE_PLATFORM_MACOS_MVK
#define VK_USE_PLATFORM_IOS_MVK
#elif SQUID_ANDROID
#define VK_USE_PLATFORM_ANDROID_KHR
#define VK_USE_PLATFORM_FUCHSIA
#else
#error Platform not supported
#endif

#include "vk_mem_alloc.h"
// #define VK_USE_PLATFORM_VI_NN
// #define VK_USE_PLATFORM_GGP

#include <SDL2/SDL_vulkan.h>
#include <vulkan/vulkan_beta.h>
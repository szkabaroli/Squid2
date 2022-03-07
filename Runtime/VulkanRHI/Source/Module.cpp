#define VMA_IMPLEMENTATION
#include "Module.h"
#include <Core/Log.h>

namespace Squid {
namespace RHI {

    std::unique_ptr<Device> VulkanAdapter::CreateDevice() {
        uint32_t families_count = 0;
        vkGetPhysicalDeviceQueueFamilyProperties(physical_device, &families_count, nullptr);

        std::vector<VkQueueFamilyProperties> family_properties(families_count);
        vkGetPhysicalDeviceQueueFamilyProperties(physical_device, &families_count, family_properties.data());

        uint32_t indexer = 0;
        for (const auto props : family_properties) {
            QueueFamily family;
            family.index = indexer;
            family.props = props;
            this->queue_families.push_back(family);
            indexer++;
        }

        auto queue_families = this->find_queue_families();

        // Create the logical device
        auto device = std::make_unique<VulkanDevice>(queue_families, this->raw_instance, physical_device);

        return device;
    };

    std::tuple<uint32_t, uint32_t, uint32_t> VulkanAdapter::find_queue_families() {
        std::optional<uint32_t> graphics_queue_family_index = std::nullopt;
        std::optional<uint32_t> compute_queue_family_index = std::nullopt;
        std::optional<uint32_t> transfer_queue_family_index = std::nullopt;

        bool allow_async_compute = true;
        bool allow_present_on_compute = true;

        for (const auto family : this->queue_families) {
            auto queue_family_props = family.props;
            auto queue_family_index = family.index;

            if ((queue_family_props.queueFlags & VK_QUEUE_GRAPHICS_BIT) == VK_QUEUE_GRAPHICS_BIT) {
                graphics_queue_family_index = queue_family_index;
            }

            if ((queue_family_props.queueFlags & VK_QUEUE_COMPUTE_BIT) == VK_QUEUE_COMPUTE_BIT) {
                if (compute_queue_family_index == std::nullopt &&
                    (allow_async_compute != false || allow_present_on_compute != false) &&
                    graphics_queue_family_index != queue_family_index) {
                    compute_queue_family_index = queue_family_index;
                }
            }

            if ((queue_family_props.queueFlags & VK_QUEUE_TRANSFER_BIT) == VK_QUEUE_TRANSFER_BIT) {
                // Prefer a non-gfx transfer queue
                if (transfer_queue_family_index == std::nullopt &&
                    (queue_family_props.queueFlags & VK_QUEUE_COMPUTE_BIT) != VK_QUEUE_GRAPHICS_BIT &&
                    (queue_family_props.queueFlags & VK_QUEUE_COMPUTE_BIT) != VK_QUEUE_COMPUTE_BIT) {
                    transfer_queue_family_index = queue_family_index;
                }
            }
        }

        // Fallback queues
        if (graphics_queue_family_index == std::nullopt) {
            throw std::runtime_error("The adapter not support graphics operations");
        }

        if (compute_queue_family_index == std::nullopt) {
            // If we didn't find a dedicated Queue, use the default one
            compute_queue_family_index = graphics_queue_family_index;
        }

        if (transfer_queue_family_index == std::nullopt) {
            // If we didn't find a dedicated Queue, use the default one
            transfer_queue_family_index = compute_queue_family_index;
        }

        return std::make_tuple(
            graphics_queue_family_index.value(), compute_queue_family_index.value(),
            transfer_queue_family_index.value());
    }

    static VKAPI_ATTR VkBool32 VKAPI_CALL DebugCallback(
        VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
        VkDebugUtilsMessageTypeFlagsEXT messageType,
        const VkDebugUtilsMessengerCallbackDataEXT *pCallbackData,
        void *pUserData) {
        LOG_ERROR("{}", pCallbackData->pMessage);
        return VK_FALSE;
    }

    bool VulkanModule::Initialize() {
        // Create instence here
        VkInstance instance = VK_NULL_HANDLE;

        VkApplicationInfo app_info = {};
        app_info.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
        app_info.pApplicationName = "Squid";
        app_info.pEngineName = "Squid";
        app_info.applicationVersion = VK_MAKE_VERSION(1, 0, 0);
        app_info.engineVersion = VK_MAKE_VERSION(1, 0, 0);
        app_info.apiVersion = VK_API_VERSION_1_1;

        VkInstanceCreateInfo create_info = {};
        create_info.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
        create_info.pApplicationInfo = &app_info;
        create_info.enabledExtensionCount = static_cast<uint32_t>(extensions.size());
        create_info.ppEnabledExtensionNames = extensions.data();
        create_info.enabledLayerCount = static_cast<uint32_t>(validation_layers.size());
        create_info.ppEnabledLayerNames = validation_layers.data();

        vkCreateInstance(&create_info, nullptr, &instance);

        VkDebugUtilsMessengerEXT debug_messenger;

        VkDebugUtilsMessengerCreateInfoEXT debug_info = {};
        debug_info.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT;
        debug_info.messageSeverity = VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT |
                                     VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT |
                                     VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT;
        debug_info.messageType = VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT |
                                 VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT |
                                 VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT;
        debug_info.pfnUserCallback = DebugCallback;
        debug_info.pUserData = nullptr; // Optional

        auto func =
            (PFN_vkCreateDebugUtilsMessengerEXT)vkGetInstanceProcAddr(instance, "vkCreateDebugUtilsMessengerEXT");
        if (func != nullptr) {
            func(instance, &debug_info, nullptr, &debug_messenger);
        }

        raw_instance = std::make_shared<RawInstance>(std::move(instance));

        return true;
    }

    std::vector<std::unique_ptr<Adapter>> VulkanModule::EnumerateAdapters() {
        // TODO: Asserts
        VkResult result;

        uint32_t device_count = 0;
        result = vkEnumeratePhysicalDevices(raw_instance->instance, &device_count, nullptr);

        std::vector<VkPhysicalDevice> physical_devices(device_count);
        result = vkEnumeratePhysicalDevices(raw_instance->instance, &device_count, physical_devices.data());

        std::vector<std::unique_ptr<Adapter>> adapters;

        for (const auto device : physical_devices) {

            VkPhysicalDeviceProperties properties;
            vkGetPhysicalDeviceProperties(device, &properties);

            AdapterInfo info;
            info.id = properties.deviceID;
            info.name = properties.deviceName;
            info.vendor_id = properties.vendorID;

            switch (properties.deviceType) {
            case VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU:
                info.type = AdapterType::DEDICATED;
                break;
            case VK_PHYSICAL_DEVICE_TYPE_INTEGRATED_GPU:
                info.type = AdapterType::INTEGRATED;
                break;
            case VK_PHYSICAL_DEVICE_TYPE_VIRTUAL_GPU:
                info.type = AdapterType::VIRTUAL;
                break;
            case VK_PHYSICAL_DEVICE_TYPE_CPU:
                info.type = AdapterType::CPU;
                break;
            default:
                info.type = AdapterType::OTHER;
                break;
            }

            auto adapter = std::make_unique<VulkanAdapter>();

            adapter->info = info;
            adapter->raw_instance = this->raw_instance;
            adapter->physical_device = device;

            adapters.push_back(std::move(adapter));
        }

        return adapters;
    };

    IMPLEMENT_MODULE(VulkanModule, VulkanRHI);

} // namespace RHI
} // namespace Squid

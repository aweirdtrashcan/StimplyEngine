#include "vulkan_internals.h"
#include <vulkan/vulkan_core.h>

bool select_physical_device(internal_vulkan_renderer_state* state) {
    uint32_t physical_device_count = 0;
    vk_result(vkEnumeratePhysicalDevices(state->instance, &physical_device_count, nullptr));
    list<VkPhysicalDevice> physical_devices(physical_device_count);
    vk_result(vkEnumeratePhysicalDevices(state->instance, &physical_device_count, physical_devices.data()));

    // TODO: Better algorithm for GPU Detection
    size_t vram_count = 0;
    VkPhysicalDevice selected_device = nullptr;
    VkPhysicalDeviceProperties selected_physical_device;
    uint32_t max_push_constant_size = 0;

    for (uint32_t i = 0; i < physical_device_count; i++) {
        VkPhysicalDeviceProperties device_properties;
        VkPhysicalDeviceMemoryProperties memory_properties;
        VkPhysicalDeviceFeatures device_features;

        max_push_constant_size = device_properties.limits.maxPushConstantsSize;

        vkGetPhysicalDeviceProperties(physical_devices[i], &device_properties);
        vkGetPhysicalDeviceMemoryProperties(physical_devices[i], &memory_properties);
        vkGetPhysicalDeviceFeatures(physical_devices[i], &device_features);

        size_t device_vram = 0;
        for (uint32_t h = 0; h < memory_properties.memoryHeapCount; h++) {
            device_vram += memory_properties.memoryHeaps[h].size;
        }

        if (device_vram > vram_count) {
            selected_device = physical_devices[i];
            selected_physical_device = device_properties;
        }
    }

    Logger::info("Selecting Physical Device: ", selected_physical_device.deviceName);

    if (!selected_device) {
        return false;
    }

    state->physical_device = selected_device;
    state->max_push_constant_size = max_push_constant_size;

    return true;
}

bool create_logical_device(internal_vulkan_renderer_state* state, const VkPhysicalDeviceFeatures* features) {
    float priority[] = { 1.0f };
    VkDeviceQueueCreateInfo queue_create_info;
    queue_create_info.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
    queue_create_info.pNext = nullptr;
    queue_create_info.flags = 0;
    queue_create_info.queueFamilyIndex = state->graphics_queue_index;
    queue_create_info.queueCount = 1;
    queue_create_info.pQueuePriorities = priority;

    list<const char*> required_extensions;

    if (!enumerate_required_device_extensions(state, required_extensions)) {
        Logger::fatal("Failed to acquire required device extension");
        return false;
    }

    VkDeviceCreateInfo create_info;
    create_info.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
    create_info.pNext = nullptr;
    create_info.flags = 0;
    create_info.queueCreateInfoCount = 1;
    create_info.pQueueCreateInfos = &queue_create_info;
    create_info.enabledLayerCount = 0;
    create_info.ppEnabledLayerNames = nullptr;
    create_info.enabledExtensionCount = static_cast<uint32_t>(required_extensions.size());
    create_info.ppEnabledExtensionNames = required_extensions.data();
    create_info.pEnabledFeatures = features;

    vk_result(vkCreateDevice(state->physical_device, &create_info, state->allocator, &state->logical_device));

    return true;
}

bool enumerate_required_device_extensions(const internal_vulkan_renderer_state* state, list<const char*>& out_required_extensions) {
    uint32_t supported_extension_count = 0;
    list<VkExtensionProperties> supported_extensions;

    vk_result(vkEnumerateDeviceExtensionProperties(state->physical_device, nullptr, &supported_extension_count, nullptr));
    
    supported_extensions.resize(supported_extension_count);

    vk_result(vkEnumerateDeviceExtensionProperties(state->physical_device, nullptr, &supported_extension_count, supported_extensions.data()));

    for (const VkExtensionProperties& property : supported_extensions) {
        Logger::debug("Supported Device Extensions: %s", property.extensionName);
    }

    list<const char*> requested_extensions(2);
    requested_extensions.push_back("VK_KHR_maintenance1");
    requested_extensions.push_back("VK_KHR_swapchain");
    //requested_extensions.push_back("VK_KHR_pipeline_library");
    //requested_extensions.push_back("VK_EXT_graphics_pipeline_library");

    for (const char* requested_extension : requested_extensions) {
        bool supported = false;
        for (uint32_t j = 0; j < supported_extension_count; j++) {
            if (strcmp(requested_extension, supported_extensions[j].extensionName) == 0) {
                supported = true;
                break;
            }
        }

        if (!supported) {
            Logger::fatal("Extension %s is not supported by your GPU!", requested_extension);
            return false;
        }
    }

    out_required_extensions = requested_extensions;

    return true;
}

bool get_physical_device_queues(internal_vulkan_renderer_state* state) {
    state->graphics_queue_index = UINT32_MAX;
    state->graphics_queue = 0;

    uint32_t queue_count = 0;
    vkGetPhysicalDeviceQueueFamilyProperties(state->physical_device, &queue_count, nullptr);

    list<VkQueueFamilyProperties> family_properties(queue_count);
    vkGetPhysicalDeviceQueueFamilyProperties(state->physical_device, &queue_count, family_properties.data());

    for (uint32_t i = 0; i < queue_count; i++) {
        if (family_properties[i].queueFlags & VK_QUEUE_GRAPHICS_BIT) {
            state->graphics_queue_index = i;
        }
    }

    if (state->graphics_queue_index == UINT32_MAX) {
        return false;
    }

    vkGetDeviceQueue(state->logical_device, state->graphics_queue_index, 0, &state->graphics_queue);

    if (state->graphics_queue == 0) {
        return false;
    }

    return true;
}

bool destroy_device(const internal_vulkan_renderer_state* state) {
    vkDestroyDevice(state->logical_device, state->allocator);

    return true;
}
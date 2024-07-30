#include "vulkan_internals.h"

#include <core/logger.h>

#include <SDL2/SDL.h>
#include <SDL2/SDL_vulkan.h>

bool create_vulkan_instance(internal_vulkan_renderer_state* state, const char* name) {
    VkApplicationInfo app_info;
    app_info.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
    app_info.pNext = nullptr;
    app_info.pApplicationName = name;
    app_info.applicationVersion = VK_MAKE_API_VERSION(0, 1, 0, 0);
    app_info.pEngineName = "Stimply Engine";
    app_info.engineVersion = VK_MAKE_API_VERSION(0, 1, 0, 0);
    app_info.apiVersion = VK_MAKE_API_VERSION(0, 1, 3, 0);

    list<const char*> extension_properties;

    if (!enumerate_required_instance_extensions(state, extension_properties)) {
        Logger::fatal("One or more instance extensions are not supported!");
        return false;
    }

    list<const char*> layer_properties;

    if (!enumerate_required_instance_layers(state, layer_properties)) {
        Logger::fatal("One or more instance layers are not supported!");
        return false;
    }

    VkDebugUtilsMessengerCreateInfoEXT debug_create_info = get_debug_utils_messenger_create_info();

    VkInstanceCreateInfo instance_create_info;
    instance_create_info.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
    instance_create_info.pNext = &debug_create_info;
    instance_create_info.flags = 0;
    instance_create_info.pApplicationInfo = &app_info;
    instance_create_info.enabledLayerCount = static_cast<uint32_t>(layer_properties.size());
    instance_create_info.ppEnabledLayerNames = layer_properties.data();
    instance_create_info.enabledExtensionCount = static_cast<uint32_t>(extension_properties.size());
    instance_create_info.ppEnabledExtensionNames = extension_properties.data();

    if (vkCreateInstance(&instance_create_info, state->allocator, &state->instance) != VK_SUCCESS) {
        return false;
    }

    return true;
}

bool enumerate_required_instance_extensions(const internal_vulkan_renderer_state* state, list<const char*>& out_required_extensions) {
    list<VkExtensionProperties> supported_extensions;
    uint32_t supported_extension_count = 0;
    
    vk_result(vkEnumerateInstanceExtensionProperties(nullptr, &supported_extension_count, nullptr));

    supported_extensions.resize(supported_extension_count);

    vk_result(vkEnumerateInstanceExtensionProperties(nullptr, &supported_extension_count, supported_extensions.data()));

    for (const VkExtensionProperties& extension : supported_extensions) {
        Logger::debug("Supported Instance Extensions: %s", extension.extensionName);
    }

    uint32_t required_extensions_count = 0;
    SDL_Vulkan_GetInstanceExtensions((SDL_Window*)state->window, &required_extensions_count, nullptr);
    out_required_extensions.resize(required_extensions_count);
    SDL_Vulkan_GetInstanceExtensions((SDL_Window*)state->window, &required_extensions_count, out_required_extensions.data());

#ifdef DEBUG
    out_required_extensions.push_back("VK_EXT_debug_utils");
#endif

    for (uint32_t i = 0; i < (uint32_t)out_required_extensions.size(); i++) {
        bool found = false;

        for (uint32_t f = 0; f < supported_extension_count; f++) {
            if (strcmp(out_required_extensions[i], supported_extensions[f].extensionName) == 0) {
                found = true;
            }
        }

        if (!found) {
            return false;
        }
    }

    return true;
}

bool enumerate_required_instance_layers(const internal_vulkan_renderer_state* state, list<const char*>& out_required_layers) {
    list<VkLayerProperties> supported_layers;
    uint32_t supported_layer_count = 0;

    vk_result(vkEnumerateInstanceLayerProperties(&supported_layer_count, nullptr));

    supported_layers.resize(supported_layer_count);

    vk_result(vkEnumerateInstanceLayerProperties(&supported_layer_count, supported_layers.data()));

    for (const VkLayerProperties& property : supported_layers) {
        Logger::debug("Supported Instance Layers: %s", property.layerName);
    }

    const char* validation = "VK_LAYER_KHRONOS_validation";

    for (uint32_t i = 0; i < supported_layer_count; i++) {
#ifdef DEBUG
        if (strcmp(supported_layers[i].layerName, validation) == 0) {
            out_required_layers.push_back(validation);
        }
#endif
    }

    return true;
}
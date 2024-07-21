#include "vulkan_internals.h"

VkDebugUtilsMessengerCreateInfoEXT get_debug_utils_messenger_create_info() {
    VkDebugUtilsMessengerCreateInfoEXT create_info;
    create_info.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT;
    create_info.pNext = nullptr;
    create_info.flags = 0;
    create_info.messageSeverity = VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT;
    create_info.messageType = VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT;
    create_info.pfnUserCallback = debug_utils_callback;
    create_info.pUserData = nullptr;

    return create_info;
}

bool enable_validation_layer(internal_vulkan_renderer_state* state) {
#ifdef DEBUG
    PFN_vkCreateDebugUtilsMessengerEXT func = 
        (PFN_vkCreateDebugUtilsMessengerEXT)vkGetInstanceProcAddr(state->instance, "vkCreateDebugUtilsMessengerEXT");

    if (!func) return false;

    VkDebugUtilsMessengerCreateInfoEXT create_info = get_debug_utils_messenger_create_info();
    vk_result(func(state->instance, &create_info, state->allocator, &state->messenger));
#endif
    return true;
}
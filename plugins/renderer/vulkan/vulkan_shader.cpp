#include "vulkan_internals.h"

bool create_shader_module(const internal_vulkan_renderer_state* state, const char* shader_path, VkShaderModule* out_shader_module) {
    binary_info shader = Platform::read_binary(shader_path);
    
    if (shader.size) {
        VkShaderModuleCreateInfo create_info;
        create_info.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
        create_info.pNext = nullptr;
        create_info.flags = 0;
        create_info.codeSize = shader.size;
        create_info.pCode = (uint32_t*)shader.binary;

        vk_result(vkCreateShaderModule(state->logical_device, &create_info, state->allocator, out_shader_module));

        return true;
    }

    return false;
}

bool destroy_shader_module(const internal_vulkan_renderer_state* state, VkShaderModule shader_module) {
    vkDestroyShaderModule(state->logical_device, shader_module, state->allocator);
    return true;
}

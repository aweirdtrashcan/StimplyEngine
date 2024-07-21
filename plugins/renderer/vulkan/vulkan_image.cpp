#include "vulkan_internals.h"

bool create_depth_buffer(internal_vulkan_renderer_state* state, const VkSurfaceCapabilitiesKHR& surface_capabilities) {
    state->depth_buffer.format = VK_FORMAT_D24_UNORM_S8_UINT;
    state->depth_buffer.width = surface_capabilities.currentExtent.width;
    state->depth_buffer.height = surface_capabilities.currentExtent.height;

    VkImageCreateInfo create_info;
    create_info.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
    create_info.pNext = nullptr;
    create_info.flags = 0;
    create_info.imageType = VK_IMAGE_TYPE_2D;
    create_info.format = state->depth_buffer.format;
    create_info.extent.width = state->depth_buffer.width;
    create_info.extent.height = state->depth_buffer.height;
    create_info.extent.depth = 1;
    create_info.mipLevels = state->mip_levels;
    create_info.arrayLayers = 1;
    create_info.samples = state->samples;
    create_info.tiling = VK_IMAGE_TILING_OPTIMAL;
    create_info.usage = VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT;
    create_info.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
    create_info.queueFamilyIndexCount = 0;
    create_info.pQueueFamilyIndices = nullptr;
    create_info.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;

    vk_result(vkCreateImage(state->logical_device, &create_info, state->allocator, &state->depth_buffer.image));

    VkMemoryRequirements memory_requirements;
    vkGetImageMemoryRequirements(state->logical_device, state->depth_buffer.image, &memory_requirements);

    uint32_t memory_index; 
    if (!find_memory_type_index(state, memory_requirements.memoryTypeBits, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, &memory_index)) {
        return false;
    }

    VkMemoryAllocateInfo allocate_info;
    allocate_info.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
    allocate_info.pNext = nullptr;
    allocate_info.allocationSize = memory_requirements.size;
    allocate_info.memoryTypeIndex = memory_index;
    
    vk_result(vkAllocateMemory(state->logical_device, &allocate_info, state->allocator, &state->depth_buffer.memory));

    vk_result(vkBindImageMemory(state->logical_device, state->depth_buffer.image, state->depth_buffer.memory, 0));
    
    VkImageViewCreateInfo view_create_info;
    view_create_info.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
    view_create_info.pNext = nullptr;
    view_create_info.flags = 0;
    view_create_info.image = state->depth_buffer.image;
    view_create_info.viewType = VK_IMAGE_VIEW_TYPE_2D;
    view_create_info.format = state->depth_buffer.format;
    view_create_info.components.a = VK_COMPONENT_SWIZZLE_IDENTITY;
    view_create_info.components.r = VK_COMPONENT_SWIZZLE_IDENTITY;
    view_create_info.components.g = VK_COMPONENT_SWIZZLE_IDENTITY;
    view_create_info.components.b = VK_COMPONENT_SWIZZLE_IDENTITY;
    view_create_info.subresourceRange.aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT;
    view_create_info.subresourceRange.baseMipLevel = 0;
    view_create_info.subresourceRange.levelCount = 1;
    view_create_info.subresourceRange.baseArrayLayer = 0;
    view_create_info.subresourceRange.layerCount = 1;

    vk_result(vkCreateImageView(state->logical_device, &view_create_info, state->allocator, &state->depth_buffer.view));

    return true;
}

bool destroy_depth_buffer(internal_vulkan_renderer_state* state) {
    if (state->depth_buffer.memory) {
        vkFreeMemory(state->logical_device, state->depth_buffer.memory, state->allocator);
    }
    if (state->depth_buffer.view) {
        vkDestroyImageView(state->logical_device, state->depth_buffer.view, state->allocator);
    }
    if (state->depth_buffer.image) {
        vkDestroyImage(state->logical_device, state->depth_buffer.image, state->allocator);
    }

    memset(&state->depth_buffer, 0, sizeof(state->depth_buffer));

    return true;
}

#include "vulkan_internals.h"
#include <vulkan/vulkan_core.h>

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

bool create_image(const internal_vulkan_renderer_state* state, VkImageType type, 
                  VkFormat format, const VkExtent3D* extent, uint32_t mip_levels,
                  VkImageUsageFlags usage, VkImageLayout initial_layout, vulkan_image* out_image) {
    if (!out_image) {
        Logger::warning("create_image: out_image can't be nullptr");
        return false;
    }

    memset(out_image, 0, sizeof(*out_image));

    VkImageCreateInfo create_info;
    create_info.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
    create_info.pNext = nullptr;
    create_info.flags = 0;
    create_info.imageType = type;
    create_info.format = format;
    create_info.extent = *extent;
    create_info.mipLevels = mip_levels;
    create_info.arrayLayers = 1;
    create_info.samples = state->samples;
    create_info.tiling = VK_IMAGE_TILING_OPTIMAL;
    create_info.usage = usage;
    create_info.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
    create_info.queueFamilyIndexCount = 0;
    create_info.pQueueFamilyIndices = nullptr;
    create_info.initialLayout = initial_layout;

    vk_result(vkCreateImage(
        state->logical_device, 
        &create_info, 
        state->allocator, 
        &out_image->image));

    VkMemoryRequirements memory_requirements;
    vkGetImageMemoryRequirements(state->logical_device, out_image->image, &memory_requirements);

    uint32_t memory_type_index = 0;

    VkResult res;

    // TODO: Fallback to system memory if video memory is not available.
    if (!find_memory_type_index(
        state, 
        memory_requirements.memoryTypeBits, 
        VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, 
        &memory_type_index)) {
        Logger::warning("create_image: Failed to find memory type index");
        goto cleanup;
    }

    VkMemoryAllocateInfo allocate_info;
    allocate_info.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
    allocate_info.pNext = nullptr;
    allocate_info.allocationSize = memory_requirements.size;
    allocate_info.memoryTypeIndex = memory_type_index;

    res = vkAllocateMemory(
        state->logical_device, 
        &allocate_info, 
        state->allocator, 
        &out_image->memory);

    if (res != VK_SUCCESS) {
        Logger::warning("create_image: Failed to allocate image memory");
        goto cleanup;
    }

    res = vkBindImageMemory(
        state->logical_device, 
        out_image->image, 
        out_image->memory, 
        0);

    if (res != VK_SUCCESS) {
        Logger::warning("create_image: Failed to bind image memory");
        goto cleanup;
    }

    out_image->width = extent->width;
    out_image->height = extent->height;
    out_image->format = format;
    out_image->mip_levels = mip_levels;

    return true;

cleanup:
    if (out_image->image) {
        vkDestroyImage(state->logical_device, out_image->image, state->allocator);
        out_image->image = nullptr;
    }
    if (out_image->memory) {
        vkFreeMemory(state->logical_device, out_image->memory, state->allocator);
        out_image->memory = nullptr;
    }

    return false;
}

bool create_image_view(const internal_vulkan_renderer_state* state, vulkan_image* image, VkImageViewType type, VkImageAspectFlags image_aspect) {
    VkImageViewCreateInfo create_info;
    create_info.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
    create_info.pNext = nullptr;
    create_info.flags = 0;
    create_info.image = image->image;
    create_info.viewType = type;
    create_info.format = image->format;
    create_info.components.a = VK_COMPONENT_SWIZZLE_IDENTITY;
    create_info.components.r = VK_COMPONENT_SWIZZLE_IDENTITY;
    create_info.components.g = VK_COMPONENT_SWIZZLE_IDENTITY;
    create_info.components.b = VK_COMPONENT_SWIZZLE_IDENTITY;
    create_info.subresourceRange.aspectMask = image_aspect;
    create_info.subresourceRange.baseMipLevel = 0;
    create_info.subresourceRange.levelCount = image->mip_levels;
    create_info.subresourceRange.baseArrayLayer = 0;
    create_info.subresourceRange.layerCount = 1;
    
    vk_result(vkCreateImageView(
        state->logical_device, 
        &create_info, 
        state->allocator, 
        &image->view));

    image->image_aspect = image_aspect;

    return true;
}

bool destroy_image(const internal_vulkan_renderer_state* state, vulkan_image* image) {
    vkDestroyImage(state->logical_device, image->image, state->allocator);
    image->image = nullptr;
    
    vkFreeMemory(state->logical_device, image->memory, state->allocator);
    image->memory = nullptr;

    return true;
}

bool destroy_image_view(const internal_vulkan_renderer_state* state, vulkan_image* image) {
    vkDestroyImageView(state->logical_device, image->view, state->allocator);
    image->view = nullptr;
   
    return true;
}

bool transition_image_layout(const internal_vulkan_renderer_state* state, VkCommandBuffer command_buffer, vulkan_image* image, 
                             VkImageLayout old_layout, VkImageLayout new_layout) {
    VkImageMemoryBarrier barrier;
    barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
    barrier.pNext = nullptr;
    barrier.oldLayout = old_layout;
    barrier.newLayout = new_layout;
    barrier.srcQueueFamilyIndex = state->graphics_queue_index;
    barrier.dstQueueFamilyIndex = state->graphics_queue_index;
    barrier.image = image->image;
    barrier.subresourceRange.aspectMask = image->image_aspect;
    barrier.subresourceRange.baseMipLevel = 0;
    barrier.subresourceRange.levelCount = image->mip_levels;
    barrier.subresourceRange.baseArrayLayer = 0;
    barrier.subresourceRange.layerCount = 1;

    VkPipelineStageFlags source_stage;
    VkPipelineStageFlags destination_stage;
    
    if (old_layout == VK_IMAGE_LAYOUT_UNDEFINED && new_layout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL) {
        source_stage = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
        destination_stage = VK_PIPELINE_STAGE_TRANSFER_BIT;

        barrier.srcAccessMask = 0;
        barrier.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
    } else if(old_layout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL && new_layout == VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL) {
        source_stage = VK_PIPELINE_STAGE_TRANSFER_BIT;
        destination_stage = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;

        barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
        barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;
    } else {
        Logger::warning("Unsupported layout transition");
        return false;
    }

    vkCmdPipelineBarrier(
        command_buffer,
        source_stage,
        destination_stage,
        0,
        0, nullptr,
        0, nullptr,
        1, &barrier
    );

    return true;
}

bool copy_buffer_to_image(VkCommandBuffer command_buffer, vulkan_image* destination_image, const gpu_buffer* source_buffer) {
    VkBufferImageCopy region;
    region.bufferOffset = 0;
    region.bufferRowLength = 0;
    region.bufferImageHeight = 0;
    region.imageSubresource.aspectMask = destination_image->image_aspect;
    region.imageSubresource.mipLevel = 0;
    region.imageSubresource.baseArrayLayer = 0;
    region.imageSubresource.layerCount = 1;
    region.imageOffset.x = 0;
    region.imageOffset.y = 0;
    region.imageOffset.z = 0;
    region.imageExtent.width = destination_image->width;
    region.imageExtent.height = destination_image->height;
    region.imageExtent.depth = 1;

    vkCmdCopyBufferToImage(
        command_buffer, 
        source_buffer->buffer, 
        destination_image->image, 
        VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 
        1, 
        &region);

    return true;
}
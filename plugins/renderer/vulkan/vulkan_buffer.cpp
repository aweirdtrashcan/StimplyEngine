#include "core/logger.h"
#include "vulkan_internals.h"
#include <cstdint>
#include <vulkan/vulkan_core.h>

bool find_memory_type_index(const internal_vulkan_renderer_state* state, uint32_t supported_memory_type, VkMemoryPropertyFlags property_flags, uint32_t* out_memory_type_index) {
    if (out_memory_type_index == nullptr) {
        Logger::fatal("find_memory_type_index: out_memory_type_index can't be nullptr");
        return false;
    }

    VkPhysicalDeviceMemoryProperties mem_properties;
    vkGetPhysicalDeviceMemoryProperties(state->physical_device, &mem_properties);

    for (uint32_t i = 0; i < mem_properties.memoryTypeCount; i++) {
        bool is_type_supported = (supported_memory_type & (1 << i));
        bool is_flag_supported = (property_flags & mem_properties.memoryTypes[i].propertyFlags) == property_flags;

        if (is_type_supported && is_flag_supported) {
            *out_memory_type_index = i;
            return true;
        }
    }

    return false;
}

bool create_uploader_buffer(const internal_vulkan_renderer_state* state, size_t size, gpu_buffer* out_gpu_buffer) {
    if (out_gpu_buffer == nullptr) {
        Logger::fatal("create_uploader_buffer: out_gpu_buffer can't be nullptr");
        return false;
    }

    Platform::zero_memory(out_gpu_buffer, sizeof(*out_gpu_buffer));

    VkBufferCreateInfo buffer_create;
    buffer_create.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
    buffer_create.pNext = nullptr;
    buffer_create.flags = 0;
    buffer_create.size = size;
    buffer_create.usage = VK_BUFFER_USAGE_TRANSFER_SRC_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT;
    buffer_create.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
    buffer_create.queueFamilyIndexCount = 0;
    buffer_create.pQueueFamilyIndices = nullptr;
    
    vk_result(vkCreateBuffer(state->logical_device, &buffer_create, state->allocator, &out_gpu_buffer->buffer));

    VkMemoryRequirements memory_requirements;
    vkGetBufferMemoryRequirements(state->logical_device, out_gpu_buffer->buffer, &memory_requirements);

    uint32_t property_flags = VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT;

    uint32_t memory_index;
    if (!find_memory_type_index(
        state,
        memory_requirements.memoryTypeBits, 
        property_flags, 
        &memory_index)) {
        return false;    
    }

    VkMemoryAllocateInfo allocate_info;
    allocate_info.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
    allocate_info.pNext = nullptr;
    allocate_info.allocationSize = memory_requirements.size;
    allocate_info.memoryTypeIndex = memory_index;

    vk_result(vkAllocateMemory(state->logical_device, &allocate_info, state->allocator, &out_gpu_buffer->memory));

    vk_result(vkBindBufferMemory(state->logical_device, out_gpu_buffer->buffer, out_gpu_buffer->memory, 0));

    out_gpu_buffer->size = memory_requirements.size;
    out_gpu_buffer->memory_property_flags = property_flags;

    vk_result(vkMapMemory(state->logical_device, out_gpu_buffer->memory, 0, out_gpu_buffer->size, 0, &out_gpu_buffer->memory_pointer));

    return true;
}

bool copy_to_upload_buffer(const internal_vulkan_renderer_state* state, void* source, size_t size, gpu_buffer* buffer) {
    if (!(buffer->memory_property_flags & VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT)) {
        Logger::debug("copy_to_upload_buffer can't be called on a buffer that's not host visible");
        return false;
    }
    
    memcpy(buffer->memory_pointer, source, size);

    VkMappedMemoryRange memory_range;
    memory_range.sType = VK_STRUCTURE_TYPE_MAPPED_MEMORY_RANGE;
    memory_range.pNext = nullptr;
    memory_range.memory = buffer->memory;
    memory_range.offset = 0;
    memory_range.size = buffer->size;

    vk_result(vkFlushMappedMemoryRanges(state->logical_device, 1, &memory_range));
    
    return true;
}

bool copy_to_gpu_buffer(VkCommandBuffer command_buffer, const gpu_buffer* source_upload_buffer, gpu_buffer* gpu_buffer, uint64_t destination_offset, uint64_t source_offset) {
    VkBufferCopy region;
    region.srcOffset = source_offset;
    region.size = source_upload_buffer->size;
    region.dstOffset = destination_offset;

    vkCmdCopyBuffer(command_buffer, source_upload_buffer->buffer, gpu_buffer->buffer, 1, &region);
    return true;
}

bool create_gpu_buffer(const internal_vulkan_renderer_state* state, size_t size, VkBufferUsageFlags usage, VkMemoryPropertyFlags memory_properties, gpu_buffer* out_gpu_buffer) {
    if (out_gpu_buffer == nullptr) {
        Logger::fatal("create_gpu_buffer: out_gpu_buffer can't be nullptr");
        return false;
    }

    Platform::zero_memory(out_gpu_buffer, sizeof(*out_gpu_buffer));

    VkBufferCreateInfo buffer_create;
    buffer_create.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
    buffer_create.pNext = nullptr;
    buffer_create.flags = 0;
    buffer_create.size = size;
    buffer_create.usage = usage;
    buffer_create.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
    buffer_create.queueFamilyIndexCount = 0;
    buffer_create.pQueueFamilyIndices = nullptr;
    
    VkResult res = vkCreateBuffer(state->logical_device, &buffer_create, state->allocator, &out_gpu_buffer->buffer);

    if (res != VK_SUCCESS) {
        Logger::debug("create_gpu_buffer: Failed to create vulkan buffer");
        return false;
    }

    VkMemoryRequirements memory_requirements;
    vkGetBufferMemoryRequirements(state->logical_device, out_gpu_buffer->buffer, &memory_requirements);

    uint32_t memory_index;
    if (!find_memory_type_index(
        state,
        memory_requirements.memoryTypeBits, 
        memory_properties, 
        &memory_index)) {
        Logger::debug("create_gpu_buffer: Failed to find memory type index");
        return false;    
    }

    VkMemoryAllocateInfo allocate_info;
    allocate_info.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
    allocate_info.pNext = nullptr;
    allocate_info.allocationSize = memory_requirements.size;
    allocate_info.memoryTypeIndex = memory_index;

    res = vkAllocateMemory(state->logical_device, &allocate_info, state->allocator, &out_gpu_buffer->memory);

    if (res != VK_SUCCESS) {
        Logger::debug("create_gpu_buffer: Failed to allocate buffer memory");
        goto cleanup;
    }

    res = vkBindBufferMemory(state->logical_device, out_gpu_buffer->buffer, out_gpu_buffer->memory, 0);

    if (res != VK_SUCCESS) {
        Logger::debug("create_gpu_buffer: Failed to bind buffer memory");
        goto cleanup;
    }

    if (memory_properties & VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT) {
        res = vkMapMemory(state->logical_device, out_gpu_buffer->memory, 0, VK_WHOLE_SIZE, 0, &out_gpu_buffer->memory_pointer);
        if (res != VK_SUCCESS) {
            Logger::debug("create_gpu_buffer: Failed to map buffer memory");
            goto cleanup;
        }
    }

    out_gpu_buffer->size = memory_requirements.size;
    out_gpu_buffer->memory_property_flags = memory_properties;

    return true;

cleanup:
    if (out_gpu_buffer->buffer) {
        vkDestroyBuffer(state->logical_device, out_gpu_buffer->buffer, state->allocator);
    }
    if (out_gpu_buffer->memory) {
        vkFreeMemory(state->logical_device, out_gpu_buffer->memory, state->allocator);
    }

    Platform::zero_memory(out_gpu_buffer, sizeof(*out_gpu_buffer));

    return false;
}

bool destroy_gpu_buffer(const internal_vulkan_renderer_state* state, gpu_buffer* buffer) {
    vkDestroyBuffer(state->logical_device, buffer->buffer, state->allocator);
    vkFreeMemory(state->logical_device, buffer->memory, state->allocator);
    Platform::zero_memory(buffer, sizeof(*buffer));

    return true;
}

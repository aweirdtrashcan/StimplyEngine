#include "core/logger.h"
#include "platform/platform.h"
#include "vulkan_defines.h"
#include "vulkan_internals.h"
#include <cstdint>
#include <vulkan/vulkan_core.h>

bool create_descriptor_pool(const internal_vulkan_renderer_state* state, VkDescriptorType* types, uint32_t type_count, VkDescriptorPoolCreateFlags flags, vulkan_descriptor_pool* out_descriptor_pool, uint32_t max_sets) {
    if (out_descriptor_pool == nullptr) {
        Logger::fatal("create_descriptor_pool: out_descriptor_pool can't be nullptr");
        return false;
    }

    list<VkDescriptorPoolSize> pool_sizes;
    pool_sizes.resize(type_count);

    for (uint32_t i = 0; i < type_count; i++) {
        pool_sizes[i].type = types[i];
        pool_sizes[i].descriptorCount = max_sets;
        out_descriptor_pool->type_bits = VkDescriptorType(out_descriptor_pool->type_bits | types[i]);
    }

    VkDescriptorPoolCreateInfo create_info;
    create_info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
    create_info.pNext = nullptr;
    create_info.flags = flags;
    create_info.maxSets = max_sets;
    create_info.poolSizeCount = type_count;
    create_info.pPoolSizes = pool_sizes.data();

    out_descriptor_pool->flags = flags;

    vk_result(vkCreateDescriptorPool(state->logical_device, &create_info, state->allocator, &out_descriptor_pool->pool));

    return true;
}

bool destroy_descriptor_pool(const internal_vulkan_renderer_state* state, vulkan_descriptor_pool* descriptor_pool) {
    vkDestroyDescriptorPool(state->logical_device, descriptor_pool->pool, state->allocator);
    Platform::ZeroMemory(descriptor_pool, sizeof(*descriptor_pool));
    return true;
}

bool allocate_descriptor_set(const internal_vulkan_renderer_state* state, vulkan_descriptor_pool* descriptor_pool, VkDescriptorSetLayout set_layout, vulkan_descriptor_set* out_descriptor_set) {
    if (out_descriptor_set == nullptr) {
        Logger::fatal("create_descriptor_set: out_descriptor_pool can't be nullptr");
        return false;
    }
    VkDescriptorSetAllocateInfo allocate_info;
    allocate_info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
    allocate_info.pNext = nullptr;
    allocate_info.descriptorPool = descriptor_pool->pool;
    allocate_info.descriptorSetCount = 1;
    allocate_info.pSetLayouts = &set_layout;
    
    out_descriptor_set->type_bits = descriptor_pool->type_bits;
    out_descriptor_set->set = nullptr;
    out_descriptor_set->set_layout = set_layout;
    vk_result(vkAllocateDescriptorSets(state->logical_device, &allocate_info, &out_descriptor_set->set));

    return true;
}

bool free_descriptor_set(const internal_vulkan_renderer_state* state, const vulkan_descriptor_pool* descriptor_pool, vulkan_descriptor_set* descriptor_set) {
    if (!(descriptor_pool->flags & VK_DESCRIPTOR_POOL_CREATE_FREE_DESCRIPTOR_SET_BIT)) {
        Logger::warning("free_descriptor_set: can't free a descriptor set that's allocated in a pool without the flag: VK_DESCRIPTOR_POOL_CREATE_FREE_DESCRIPTOR_SET_BIT");
        return false;
    }

    vkFreeDescriptorSets(state->logical_device, descriptor_pool->pool, 1, &descriptor_set->set);
    Platform::ZeroMemory(descriptor_set, sizeof(*descriptor_set));

    return true;
}

bool create_descriptor_set_layout(const internal_vulkan_renderer_state* state, uint32_t binding_count, const VkDescriptorSetLayoutBinding* bindings, VkDescriptorSetLayout* out_set_layout) {
    VkDescriptorSetLayoutCreateInfo create_info;
    create_info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
    create_info.pNext = nullptr;
    create_info.flags = 0;
    create_info.bindingCount = binding_count;
    create_info.pBindings = bindings;

    vk_result(vkCreateDescriptorSetLayout(state->logical_device, &create_info, state->allocator, out_set_layout));

    return true;
}

bool destroy_descriptor_set_layout(const internal_vulkan_renderer_state* state, VkDescriptorSetLayout layout) {
    vkDestroyDescriptorSetLayout(state->logical_device, layout, state->allocator);

    return true;
}

bool update_descriptor_set_for_buffer(const internal_vulkan_renderer_state* state, VkBuffer buffer, uint64_t offset, uint64_t size, vulkan_descriptor_set* destination_set, uint32_t binding) {
    VkDescriptorBufferInfo buffer_info;
    buffer_info.buffer = buffer;
    buffer_info.offset = offset;
    buffer_info.range = size;

    VkWriteDescriptorSet writes;
    writes.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
    writes.pNext = nullptr;
    writes.dstSet = destination_set->set;
    writes.dstBinding = binding;
    writes.dstArrayElement = 0;
    writes.descriptorCount = 1;
    writes.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
    writes.pImageInfo = nullptr;
    writes.pBufferInfo = &buffer_info;
    writes.pTexelBufferView = nullptr;

    vkUpdateDescriptorSets(state->logical_device, 1, &writes, 0, nullptr);

    return true;    
}

bool update_descriptor_set_for_texture(const internal_vulkan_renderer_state* state, const vulkan_texture* texture,
    vulkan_descriptor_set* destination_set, uint32_t binding) {
    
    VkDescriptorImageInfo image_info;
    image_info.sampler = texture->sampler;
    image_info.imageView = texture->image.view;
    image_info.imageLayout = texture->image.image_layout;

    VkWriteDescriptorSet writes;
    writes.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
    writes.pNext = nullptr;
    writes.dstSet = destination_set->set;
    writes.dstBinding = binding;
    writes.dstArrayElement = 0;
    writes.descriptorCount = 1;
    writes.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
    writes.pImageInfo = &image_info;
    writes.pBufferInfo = nullptr;
    writes.pTexelBufferView = nullptr;

    vkUpdateDescriptorSets(state->logical_device, 1, &writes, 0, nullptr);

    return true;    
}

bool bind_descriptor_set(
    const internal_vulkan_renderer_state* state, 
    VkCommandBuffer command_buffer, 
    const vulkan_descriptor_set* descriptor_set, 
    const vulkan_pipeline* pipeline) {
    vkCmdBindDescriptorSets(
        command_buffer, 
        VK_PIPELINE_BIND_POINT_GRAPHICS, 
        pipeline->layout, 
        0, 
        1, 
        &descriptor_set->set, 
        0,
        nullptr);

    return true;
}
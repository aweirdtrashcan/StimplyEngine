#include "vulkan_internals.h"

bool create_descriptor_pool(const internal_vulkan_renderer_state* state, VkDescriptorType type, bool can_be_freed, VkDescriptorPool* out_descriptor_pool, uint32_t max_sets) {
    if (out_descriptor_pool == nullptr) {
        Logger::fatal("create_descriptor_pool: out_descriptor_pool can't be nullptr");
        return false;
    }

    VkDescriptorPoolSize pool_size;
    pool_size.type = type;
    pool_size.descriptorCount = max_sets;

    VkDescriptorPoolCreateInfo create_info;
    create_info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
    create_info.pNext = nullptr;
    create_info.flags = can_be_freed ? VK_DESCRIPTOR_POOL_CREATE_FREE_DESCRIPTOR_SET_BIT : 0;
    create_info.maxSets = max_sets;
    create_info.poolSizeCount = 1;
    create_info.pPoolSizes = &pool_size;

    vk_result(vkCreateDescriptorPool(state->logical_device, &create_info, state->allocator, out_descriptor_pool));

    return true;
}

bool destroy_descriptor_pool(const internal_vulkan_renderer_state* state, VkDescriptorPool descriptor_pool) {
    vkDestroyDescriptorPool(state->logical_device, descriptor_pool, state->allocator);
    return true;
}

bool create_descriptor_set(const internal_vulkan_renderer_state* state, VkDescriptorPool descriptor_pool, VkDescriptorSet* out_descriptor_set) {
    if (out_descriptor_set == nullptr) {
        Logger::fatal("create_descriptor_set: out_descriptor_pool can't be nullptr");
        return false;
    }
    VkDescriptorSetAllocateInfo allocate_info;
    allocate_info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
    allocate_info.pNext = nullptr;
    allocate_info.descriptorPool = descriptor_pool;
    allocate_info.descriptorSetCount = 1;
    
    vk_result(vkAllocateDescriptorSets(state->logical_device, &allocate_info, out_descriptor_set));

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
#include "vulkan_internals.h"

bool create_fence(const internal_vulkan_renderer_state* state, VkFence* out_fence, bool create_signaled) {
    VkFenceCreateInfo create_info;
    create_info.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
    create_info.pNext = nullptr;
    create_info.flags = create_signaled ? VK_FENCE_CREATE_SIGNALED_BIT : 0;

    vk_result(vkCreateFence(state->logical_device, &create_info, state->allocator, out_fence));

    return true;
}

bool destroy_fence(const internal_vulkan_renderer_state* state, VkFence fence) {
    vkDestroyFence(state->logical_device, fence, state->allocator);

    return true;
}
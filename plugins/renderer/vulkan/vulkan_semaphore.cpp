#include "vulkan_internals.h"

bool create_semaphore(const internal_vulkan_renderer_state* state, VkSemaphore& out_semaphore) {
    VkSemaphoreCreateInfo create_info;
    create_info.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;
    create_info.pNext = nullptr;
    create_info.flags = 0;

    vk_result(vkCreateSemaphore(state->logical_device, &create_info, state->allocator, &out_semaphore));

    return true;
}
#include "vulkan_internals.h"

bool create_command_pool(const internal_vulkan_renderer_state* state, VkCommandPool* out_command_pool, uint32_t queueIndex) {
    if (out_command_pool == nullptr) {
        Logger::fatal("create_command_pool: out_command_pool can't be nullptr");
        return false;
    }

    VkCommandPoolCreateInfo create_info;
    create_info.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
    create_info.pNext = nullptr;
    create_info.queueFamilyIndex = queueIndex;
    create_info.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;

    vk_result(vkCreateCommandPool(state->logical_device, &create_info, state->allocator, out_command_pool));

    return true;
}

bool destroy_command_pool(const internal_vulkan_renderer_state* state, VkCommandPool command_pool) {
    vkDestroyCommandPool(state->logical_device, command_pool, state->allocator);
    return true;
}

bool allocate_command_buffers(const internal_vulkan_renderer_state* state, VkCommandPool command_pool, uint32_t command_buffer_count, VkCommandBuffer* out_command_buffer) {
    VkCommandBufferAllocateInfo allocate_info;
    allocate_info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
    allocate_info.pNext = nullptr;
    allocate_info.commandPool = command_pool;
    allocate_info.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
    allocate_info.commandBufferCount = command_buffer_count;

    vk_result(vkAllocateCommandBuffers(state->logical_device, &allocate_info, out_command_buffer));

    return true;
}

bool free_command_buffers(const internal_vulkan_renderer_state* state, uint32_t command_buffer_count, VkCommandPool command_pool, VkCommandBuffer* command_buffers) {
    vkFreeCommandBuffers(state->logical_device, command_pool, command_buffer_count, command_buffers);
    return true;
}

bool begin_command_buffer(VkCommandBuffer command_buffer) {
    VkCommandBufferBeginInfo begin_info;
    begin_info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
    begin_info.pNext = nullptr;
    begin_info.flags = 0;
    begin_info.pInheritanceInfo = nullptr;

    vk_result(vkBeginCommandBuffer(command_buffer, &begin_info));

    return true;
}

bool end_command_buffer(VkCommandBuffer command_buffer) {
    vkEndCommandBuffer(command_buffer);
    return true;
}

bool create_one_time_command_buffer(const internal_vulkan_renderer_state* state, VkCommandBuffer* out_command_buffer) {
    if (!allocate_command_buffers(state, state->command_pool, 1, out_command_buffer)) {
        return false;
    }

    begin_command_buffer(*out_command_buffer);

    return true;
}

bool end_one_time_command_buffer(const internal_vulkan_renderer_state* state, VkCommandBuffer command_buffer, VkQueue queue) {
    end_command_buffer(command_buffer);
    
    VkFence fence;
    create_fence(state, &fence, false);

    submit_command_queue(nullptr, nullptr, fence, queue, command_buffer);
    
    vk_result(vkWaitForFences(state->logical_device, 1, &fence, VK_TRUE, UINT64_MAX));

    free_command_buffers(state, 1, state->command_pool, &command_buffer);

    destroy_fence(state, fence);

    return true;
}

VkResult submit_command_queue(VkSemaphore wait_semaphore, VkSemaphore signal_semaphore, VkFence fence, VkQueue queue, VkCommandBuffer command_buffer) {
    VkPipelineStageFlags wait_stages[] = {
        VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT
    };
    
    VkSubmitInfo submit_info;
    submit_info.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
    submit_info.pNext = nullptr;
    submit_info.waitSemaphoreCount = wait_semaphore ? 1 : 0;
    submit_info.pWaitSemaphores = &wait_semaphore;
    submit_info.pWaitDstStageMask = wait_stages;
    submit_info.commandBufferCount = 1;
    submit_info.pCommandBuffers = &command_buffer;
    submit_info.signalSemaphoreCount = signal_semaphore ? 1 : 0;
    submit_info.pSignalSemaphores = &signal_semaphore;

    return vkQueueSubmit(queue, 1, &submit_info, fence);
}
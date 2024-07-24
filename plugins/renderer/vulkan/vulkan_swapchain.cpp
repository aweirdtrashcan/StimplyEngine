#include "vulkan_internals.h"

bool query_optimal_back_buffer_format(const internal_vulkan_renderer_state* state, VkSurfaceFormatKHR* format) {
    if (format == nullptr) return false;

    uint32_t format_count;
    
    vk_result(vkGetPhysicalDeviceSurfaceFormatsKHR(
        state->physical_device, 
        state->surface, 
        &format_count, 
        nullptr));

    list<VkSurfaceFormatKHR> surface_format;
    surface_format.resize(format_count);

    vk_result(vkGetPhysicalDeviceSurfaceFormatsKHR(
        state->physical_device, 
        state->surface, 
        &format_count, 
        surface_format.data()));

    for (VkSurfaceFormatKHR supported_format : surface_format) {
        if (supported_format.format == VK_FORMAT_B8G8R8A8_UNORM && supported_format.colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR) {
            *format = supported_format;
            return true;
        }
    }

    return false;
}

bool create_swapchain(internal_vulkan_renderer_state* state, const VkSurfaceCapabilitiesKHR& surface_capabilities) {   
    if (surface_capabilities.currentExtent.width == 0 || surface_capabilities.currentExtent.height == 0) {
        return false;
    }

    get_viewport_and_scissor(state->surface_capabilities, &state->viewport, &state->scissor);

    VkSwapchainCreateInfoKHR create_info;
    create_info.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
    create_info.pNext = nullptr;
    create_info.flags = 0;
    create_info.surface = state->surface;

    uint32_t min_image_count = surface_capabilities.minImageCount;

    // ask for at least double-buffer
    if (surface_capabilities.minImageCount < 2) {
        min_image_count = 2;
    }

    create_info.minImageCount = min_image_count;
    
    VkSurfaceFormatKHR surface_format;
    if (!query_optimal_back_buffer_format(state, &surface_format)) {
        return false;
    }

    create_info.imageFormat = surface_format.format;
    create_info.imageColorSpace = surface_format.colorSpace;
    create_info.imageExtent = surface_capabilities.currentExtent;
    create_info.imageArrayLayers = 1;
    create_info.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
    create_info.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
    create_info.queueFamilyIndexCount = 0;
    create_info.pQueueFamilyIndices = nullptr;
    create_info.preTransform = surface_capabilities.currentTransform;
    create_info.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
    create_info.presentMode = VK_PRESENT_MODE_MAILBOX_KHR;
    create_info.clipped = VK_TRUE;
    create_info.oldSwapchain = VK_NULL_HANDLE;

    vk_result(vkCreateSwapchainKHR(state->logical_device, &create_info, state->allocator, &state->swapchain));

    state->back_buffer_format = surface_format.format;
    // TODO: Multisampling?
    state->samples = VK_SAMPLE_COUNT_1_BIT;
    // TODO: Mip-mapping?
    state->mip_levels = 1;

    return true;
}

bool destroy_swapchain(internal_vulkan_renderer_state* state) {
    vkDestroySwapchainKHR(state->logical_device, state->swapchain, state->allocator);
    return true;
}

bool get_swapchain_back_buffers(internal_vulkan_renderer_state* state) {
    state->num_frames = 0;
    vk_result(vkGetSwapchainImagesKHR(state->logical_device, state->swapchain, &state->num_frames, nullptr));
    state->back_buffers.resize(state->num_frames);
    vk_result(vkGetSwapchainImagesKHR(state->logical_device, state->swapchain, &state->num_frames, state->back_buffers.data()));

    return true;
}

bool create_swapchain_back_buffer_views(internal_vulkan_renderer_state* state) {
    state->back_buffer_views.resize(state->num_frames);

    for (uint32_t i = 0; i < state->num_frames; i++) {
        VkImageViewCreateInfo create_info;
        create_info.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
        create_info.pNext = nullptr;
        create_info.flags = 0;
        create_info.image = state->back_buffers[i];
        create_info.viewType = VK_IMAGE_VIEW_TYPE_2D;
        create_info.format = state->back_buffer_format;
        // 0 = identity
        memset(&create_info.components, 0, sizeof(create_info.components));
        create_info.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
        create_info.subresourceRange.baseArrayLayer = 0;
        create_info.subresourceRange.baseMipLevel = 0;
        create_info.subresourceRange.layerCount = 1;
        create_info.subresourceRange.levelCount = 1;

        vk_result(vkCreateImageView(state->logical_device, &create_info, state->allocator, &state->back_buffer_views[i]));
    }

    return true;
}

bool destroy_swapchain_image_views(internal_vulkan_renderer_state* state) {
    for (VkImageView& view : state->back_buffer_views) {
        vkDestroyImageView(state->logical_device, view, state->allocator);
        view = nullptr;
    }

    return true;
}

bool create_swapchain_semaphores_and_fences(internal_vulkan_renderer_state* state) {
    state->fences.remove_all();
    state->last_image_fence.remove_all();
    state->image_acquired_semaphore.remove_all();
    state->queue_complete_semaphore.remove_all();

    state->fences.resize(state->num_frames);
    state->image_acquired_semaphore.resize(state->num_frames);
    state->queue_complete_semaphore.resize(state->num_frames);
    state->last_image_fence.resize(state->num_frames);
    memset(state->last_image_fence.data(), 0, sizeof(VkFence) * state->num_frames);

    for (uint32_t i = 0; i < state->num_frames; i++) {
        if (!create_fence(state, &state->fences[i], true)) {
            return false;
        }
        if (!create_semaphore(state, state->image_acquired_semaphore[i])) {
            return false;
        }
        if (!create_semaphore(state, state->queue_complete_semaphore[i])) {
            return false;
        }
    }

    return true;
}

bool destroy_swapchain_semaphores_and_fences(internal_vulkan_renderer_state* state) {
    for (VkFence fence : state->fences) {
        vkDestroyFence(state->logical_device, fence, state->allocator);
    }
    for (VkSemaphore semaphore : state->image_acquired_semaphore) {
        vkDestroySemaphore(state->logical_device, semaphore, state->allocator);
    }
    for (VkSemaphore semaphore : state->queue_complete_semaphore) {
        vkDestroySemaphore(state->logical_device, semaphore, state->allocator);
    }

    return true;
}


bool create_swapchain_framebuffers(internal_vulkan_renderer_state* state, const VkSurfaceCapabilitiesKHR& surface_capabilities) {
    state->swapchain_framebuffers.resize(state->num_frames);
    for (uint32_t i = 0; i < state->num_frames; i++) {
        VkImageView attachments[] = {
            state->back_buffer_views[i],
            state->depth_buffer.view
        };

        if (!create_framebuffer(
                state,
                state->main_renderpass, 
                std::size(attachments), 
                attachments, 
                state->surface_capabilities.currentExtent.width,
                state->surface_capabilities.currentExtent.height,
                &state->swapchain_framebuffers[i])) {
            return false;
        }
    }

    return true;
}

bool destroy_swapchain_framebuffers(internal_vulkan_renderer_state* state) {
    for (uint32_t i = 0; i < state->num_frames; i++) {
        vkDestroyFramebuffer(state->logical_device, state->swapchain_framebuffers[i], state->allocator);
        state->swapchain_framebuffers[i] = nullptr;
    }

    return true;
}

bool recreate_swapchain(internal_vulkan_renderer_state* state) {
    vkDeviceWaitIdle(state->logical_device);

    destroy_swapchain_framebuffers(state);
    destroy_render_pass(state);
    destroy_swapchain_image_views(state);
    destroy_swapchain(state);
    destroy_depth_buffer(state);
    state->swapchain = nullptr;

    vk_result(vkGetPhysicalDeviceSurfaceCapabilitiesKHR(
        state->physical_device, 
        state->surface, 
        &state->surface_capabilities));

    if (!create_swapchain(state, state->surface_capabilities)) {
        return false;
    }
    if (!get_swapchain_back_buffers(state)) {
        return false;
    }
    if (!create_swapchain_back_buffer_views(state)) {
        return false;
    }
    if (!create_depth_buffer(state, state->surface_capabilities)) {
        return false;
    }
    if (!create_render_pass(state)) {
        return false;
    }
    if (!create_swapchain_framebuffers(state, state->surface_capabilities)) {
        return false;
    }

    return true;    
}
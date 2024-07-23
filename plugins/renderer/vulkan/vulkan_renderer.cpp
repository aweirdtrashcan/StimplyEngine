#include "vulkan_renderer.h"
#include "vulkan_defines.h"
#include "vulkan_internals.h"

#include <core/logger.h>
#include <cstdint>
#include <iterator>
#include <renderer/renderer_exception.h>
#include <renderer/renderer_types.h>
#include <vulkan/vulkan_core.h>

#include <DirectXMath.h>
#include <DirectXMath/Extensions/DirectXMathAVX2.h>

extern "C" {

internal_vulkan_renderer_state* state = nullptr;

/* public functions */
bool vulkan_backend_initialize(uint64_t* required_size, HANDLE allocated_memory, const char* name, HANDLE sdl_window) noexcept(false) {
    if (!required_size) return false;
    if (*required_size == 0) {
        *required_size = sizeof(internal_vulkan_renderer_state);
        return true;
    }
    if (!allocated_memory) {
        return false;
    }

    state = new (allocated_memory) internal_vulkan_renderer_state;

    Logger::info("Initializing Vulkan Backend");

    if (!DirectX::XMVerifyCPUSupport()) {
        throw RendererException("Your CPU does not support SSE/SSE2 Instruction Sets, which is required by this application.");
    }

    if (!DirectX::AVX2::XMVerifyAVX2Support()) {
        throw RendererException("Your CPU does not support AVX2 Instruction Set, which is required by this application.");
    }

    state->window = sdl_window;

    if (!create_vulkan_instance(state, name)) {
        throw RendererException("Failed to create vulkan instance");
    }    

    if (!enable_validation_layer(state)) {
        throw RendererException("Validaiton Layer was not enabled!");
    }

    if (!select_physical_device(state)) {
        throw RendererException("Failed to select physical device");
    }

    VkPhysicalDeviceFeatures features{};
    features.depthClamp = VK_TRUE;

    if (!create_logical_device(state, &features)) {
        throw RendererException("Failed to create logical device");
    }

    if (!get_physical_device_queues(state)) {
        throw RendererException("Failed to get physical device queues");
    }

    if (!create_surface(state)) {
        throw RendererException("Failed to create surface");
    }

    vk_result(vkGetPhysicalDeviceSurfaceCapabilitiesKHR(
        state->physical_device, 
        state->surface, 
        &state->surface_capabilities));

    if (!create_swapchain(state, state->surface_capabilities)) {
        throw RendererException("Failed to create swapchain");
    }

    if (!get_swapchain_back_buffers(state)) {
        throw RendererException("Failed to get swapchain backbuffers");
    }

    if (!create_swapchain_back_buffer_views(state)) {
        throw RendererException("Failed to create swapchain backbuffer views");
    }

    if (!create_swapchain_semaphores_and_fences(state)) {
        throw RendererException("Failed to create swapchain semaphores and fences");
    }

    if (!create_depth_buffer(state, state->surface_capabilities)) {
        throw RendererException("Failed to create depth buffer");
    }

    if (!create_descriptor_pool(state, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, true, &state->uniform_descriptor_pool)) {
        throw RendererException("Failed to create uniform buffer descriptor pool");
    }

    if (!create_command_pool(state, &state->command_pool, state->graphics_queue_index)) {
        throw RendererException("Failed to create graphics command pool");
    }

    state->graphics_command_buffers.resize(state->num_frames);
    if (!allocate_command_buffers(state, state->command_pool, state->num_frames, state->graphics_command_buffers.data())) {
        throw RendererException("Failed to allocate graphics command buffers");
    }

    if (!create_render_pass(state)) {
        throw RendererException("Failed to create main render pass");
    }

    if (!create_swapchain_framebuffers(state, state->surface_capabilities)) {
        throw RendererException("Failed to create swapchain framebuffers");
    }

    if (!create_vulkan_shader(state, &state->light_shader)) {
        throw RendererException("Failed to load light shader");
    }

    VkClearValue color_clear_value{};
    color_clear_value.color.float32[0] = 0.8f;
    color_clear_value.color.float32[1] = 0.3f;
    color_clear_value.color.float32[2] = 0.8f;
    color_clear_value.color.float32[3] = 1.0f;

    VkClearValue depth_clear_value{};
    depth_clear_value.depthStencil.depth = 1.0f;

    state->clear_values[0] = color_clear_value;
    state->clear_values[1] = depth_clear_value;

    Logger::info("Vulkan Backend Initialized");

    return true;
}

void vulkan_backend_shutdown() {
    vkDeviceWaitIdle(state->logical_device);
    Logger::info("Shutting Down Vulkan Renderer");

    //destroy_pipeline(state, state->graphics_pipelines[PipelineTypeMVP]);
    //destroy_graphics_pipeline_layout(state, state->graphics_pipeline_layouts[PipelineTypeMVP]);
    //destroy_descriptor_set_layout(state, state->graphics_set_layouts[PipelineTypeMVP]);
    destroy_vulkan_shader(state, &state->light_shader);
    destroy_swapchain_framebuffers(state);
    destroy_render_pass(state);
    destroy_command_pool(state, state->command_pool);
    destroy_descriptor_pool(state, state->uniform_descriptor_pool);
    destroy_depth_buffer(state);
    destroy_swapchain_semaphores_and_fences(state);
    destroy_swapchain_image_views(state);
    destroy_swapchain(state);
    vkDestroySurfaceKHR(state->instance, state->surface, state->allocator);
    vkDestroyDevice(state->logical_device, state->allocator);

    if (state->messenger) {
        PFN_vkDestroyDebugUtilsMessengerEXT func =
            (PFN_vkDestroyDebugUtilsMessengerEXT)vkGetInstanceProcAddr(state->instance, "vkDestroyDebugUtilsMessengerEXT");

        if (func) {
            func(state->instance, state->messenger, state->allocator);
        }
    }

    vkDestroyInstance(state->instance, state->allocator);

    state->~internal_vulkan_renderer_state();

    memset(state, 0, sizeof(*state));
}

bool vulkan_begin_frame() {
    VkFence fence = state->fences[state->current_frame_index];
    VkSemaphore queue_complete = state->queue_complete_semaphore[state->current_frame_index];
    VkSemaphore image_acquired = state->image_acquired_semaphore[state->current_frame_index];
    VkCommandBuffer command_buffer = state->graphics_command_buffers[state->current_frame_index];
    VkRenderPass renderpass = state->main_renderpass;
    const VkSurfaceCapabilitiesKHR& surface_capabilities = state->surface_capabilities;
    const VkClearValue* clear_values = state->clear_values;

    vk_result(vkWaitForFences(state->logical_device, 1, &fence, VK_TRUE, UINT64_MAX));

    VkResult result;

    if (!state->swapchain) {
        recreate_swapchain(state);
        return false;
    }

    result = vkAcquireNextImageKHR(
        state->logical_device, 
        state->swapchain, 
        UINT64_MAX, 
        image_acquired, 
        VK_NULL_HANDLE, 
        &state->image_index);

    if (result != VK_SUCCESS) {
        // TODO: Resize
        recreate_swapchain(state);
        return false;
    }

    vk_result(vkResetFences(state->logical_device, 1, &fence));

    VkFramebuffer framebuffer = state->swapchain_framebuffers[state->image_index];

    VkViewport viewport;
    VkRect2D scissor;
    get_viewport_and_scissor(surface_capabilities, &viewport, &scissor);

    vkResetCommandBuffer(command_buffer, 0);
    begin_command_buffer(command_buffer);

    vkCmdSetViewport(command_buffer, 0, 1, &viewport);
    vkCmdSetScissor(command_buffer, 0, 1, &scissor);


    begin_render_pass(renderpass, command_buffer, framebuffer, scissor, (uint32_t)std::size(state->clear_values), clear_values);

    return true;
}

bool vulkan_draw_items() {
    VkCommandBuffer command_buffer = state->graphics_command_buffers[state->current_frame_index];

    for (uint32_t i = 0; i < PipelineTypeMAX; i++) {
        list<render_item*>& render_items = state->render_items[i];
        
        // vkCmdBindPipeline(
        //     command_buffer, 
        //     VK_PIPELINE_BIND_POINT_GRAPHICS, 
        //     state->graphics_pipelines[i]);
        
        // for (uint32_t j = 0; j < render_items.size_u32(); j++) {
        //     VkDeviceSize offset = 0;
        //     vkCmdBindIndexBuffer(command_buffer, render_items[j]->index_buffer.buffer, 0, VK_INDEX_TYPE_UINT32);
        //     vkCmdBindVertexBuffers(command_buffer, 0, 1, &render_items[j]->vertex_buffer.buffer, &offset);
        //     vkCmdDrawIndexed(command_buffer, render_items[j]->index_count, 1, 0, 0, 0);
        // }
    }

    return true;
}

bool vulkan_end_frame() {
    VkResult result;
    VkCommandBuffer command_buffer = state->graphics_command_buffers[state->current_frame_index];
    VkSemaphore image_acquired = state->image_acquired_semaphore[state->current_frame_index];
    VkSemaphore queue_completed = state->queue_complete_semaphore[state->current_frame_index];
    VkQueue queue = state->graphics_queue;
    VkFence fence = state->fences[state->current_frame_index];

    end_render_pass(command_buffer);
    end_command_buffer(command_buffer);

    result = submit_command_queue(image_acquired, queue_completed, fence, queue, command_buffer);

    if (result == VK_ERROR_OUT_OF_DATE_KHR || result == VK_SUBOPTIMAL_KHR) {
        recreate_swapchain(state);
        return false;
    } else if (result != VK_SUCCESS) {
        return false;
    }

    VkPresentInfoKHR present_info;
    present_info.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
    present_info.pNext = nullptr;
    present_info.waitSemaphoreCount = 1;
    present_info.pWaitSemaphores = &queue_completed;
    present_info.swapchainCount = 1;
    present_info.pSwapchains = &state->swapchain;
    present_info.pImageIndices = &state->image_index;
    present_info.pResults = nullptr;

    //Logger::debug("Presenting built frame %u", state->current_frame_index);
    //Logger::debug("Presenting image %u", state->image_index);

    result = vkQueuePresentKHR(queue, &present_info);
    state->current_frame_index = (state->current_frame_index + 1) % state->num_frames;

    if (result == VK_ERROR_OUT_OF_DATE_KHR || result == VK_SUBOPTIMAL_KHR) {
        recreate_swapchain(state);
        return false;
    } else if (result != VK_SUCCESS) {
        return false;
    }

    return true;
}

HANDLE vulkan_create_render_item(const RenderItemCreateInfo* pRenderItemCreateInfo) {
    if (pRenderItemCreateInfo == nullptr) {
        Logger::debug("vulkan_create_render_item: pRenderItemCreateInfo can't be nullptr");
        return nullptr;
    }
    if (pRenderItemCreateInfo->pIndices == 0 || pRenderItemCreateInfo->pVertices == 0) {
        Logger::debug("vulkan_create_render_item: RenderItemCreateInfo->pIndices and/or RenderItemCreateInfo->pMeshes can't be nullptr");
        return nullptr;
    }

    VkCommandBuffer command_buffer;
    create_one_time_command_buffer(state, &command_buffer);

    gpu_buffer vertex_uploader;
    gpu_buffer index_uploader;

    render_item* r_item = (render_item*)Platform::ualloc(sizeof(render_item));

    if (!create_uploader_buffer(state, pRenderItemCreateInfo->verticesSize, &vertex_uploader)) {
        Logger::debug("vulkan_create_render_item: Failed to create vertex buffer");
        return nullptr;
    }

    if (!create_uploader_buffer(state, pRenderItemCreateInfo->indicesSize, &index_uploader)) {
        Logger::debug("vulkan_create_render_item: Failed to create vertex buffer");
        return nullptr;
    }

    if (!create_gpu_buffer(
        state,
        pRenderItemCreateInfo->verticesSize, 
        VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_VERTEX_BUFFER_BIT,
        VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
        &r_item->vertex_buffer)) {
        Logger::debug("vulkan_create_render_item: Failed to create vertex buffer");
        return nullptr;
    }

    if (!create_gpu_buffer(
        state,
        pRenderItemCreateInfo->indicesSize, 
        VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_INDEX_BUFFER_BIT,
        VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
        &r_item->index_buffer)) {
        Logger::debug("vulkan_create_render_item: Failed to create index buffer");
        return nullptr;
    }

    copy_to_upload_buffer(state, pRenderItemCreateInfo->pVertices, pRenderItemCreateInfo->verticesSize, &vertex_uploader);
    copy_to_upload_buffer(state, pRenderItemCreateInfo->pIndices, pRenderItemCreateInfo->indicesSize, &index_uploader);

    copy_to_gpu_buffer(command_buffer, &vertex_uploader, &r_item->vertex_buffer);
    copy_to_gpu_buffer(command_buffer, &index_uploader, &r_item->index_buffer);

    end_one_time_command_buffer(state, command_buffer, state->graphics_queue);

    destroy_gpu_buffer(state, &vertex_uploader);
    destroy_gpu_buffer(state, &index_uploader);

    // state->render_items[pRenderItemCreateInfo->shaderBundle.pipelineType].push_back(r_item);

    return r_item;
}

void vulkan_destroy_render_item(HANDLE item_handle) {
    render_item* r_item = (render_item*)item_handle;
    destroy_gpu_buffer(state, &r_item->vertex_buffer);
    destroy_gpu_buffer(state, &r_item->index_buffer);

    Platform::zero_memory(r_item, sizeof(*r_item));

    for (uint32_t i = 0; i < PipelineTypeMAX; i++) {
        // if (state->render_items[r_item->pipeline_type][i] == r_item) {
        //     state->render_items[r_item->pipeline_type].remove_at(i);
        // }
    }

    Platform::ufree(r_item);
}

/****************************************** INTERNAL FUNCTIONS ********************************************* */
VkBool32 debug_utils_callback(VkDebugUtilsMessageSeverityFlagBitsEXT           messageSeverity,
    VkDebugUtilsMessageTypeFlagsEXT                  messageTypes,
    const VkDebugUtilsMessengerCallbackDataEXT*      pCallbackData,
    void*                                            pUserData) {

    switch (messageSeverity) {
        case VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT:
            Logger::debug("%s", pCallbackData->pMessage);
            break;
        case VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT:
            Logger::warning("%s", pCallbackData->pMessage);
            break;
        case VK_DEBUG_UTILS_MESSAGE_SEVERITY_INFO_BIT_EXT:
            Logger::info("%s", pCallbackData->pMessage);
        default:
            Logger::info("%s", pCallbackData->pMessage);
            break;
    }

    return VK_FALSE;
}

bool get_viewport_and_scissor(const VkSurfaceCapabilitiesKHR& surface_capabilities, VkViewport* out_viewport, VkRect2D* out_scissor) {
    float width = (float)surface_capabilities.currentExtent.width;
    float height = (float)surface_capabilities.currentExtent.height;

    out_viewport->x = 0.0f;
    out_viewport->y = height;
    out_viewport->width = width;
    out_viewport->height = -height;
    out_viewport->minDepth = 0.0f;
    out_viewport->maxDepth = 1.0f;

    out_scissor->extent = surface_capabilities.currentExtent;
    out_scissor->offset = {};

    return true;
}

}

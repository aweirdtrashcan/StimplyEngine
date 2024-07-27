#include "vulkan_renderer.h"
#include "DirectXMath.h"
#include "platform/platform.h"
#include "vulkan_defines.h"
#include "vulkan_internals.h"
#include "../render_utils.inl"

#include <core/logger.h>
#include <cstdint>
#include <cstring>
#include <renderer/renderer_exception.h>
#include <renderer/renderer_types.h>
#include <vulkan/vulkan_core.h>

// Don't change the order of the includes, because global_uniform_object.h
// includes DirectXMath, and DirectXMath on Unix systems *HAS* to be included
// after all the STL headers.

// UPDATE: I've removed the SAL code from the headers i'm actually using
// so it seems that the order doesn't matter anymore.
#include <DirectXColors.h>
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
        throw RendererException("Your CPU does not support SSE Instruction Set, which is required by this application.");
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
    features.samplerAnisotropy = VK_TRUE;

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

    uint32_t memory_properties = VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT;
    uint32_t buffer_usage = VK_BUFFER_USAGE_TRANSFER_SRC_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT;

    if (!create_gpu_buffer(state, sizeof(DirectX::XMFLOAT3) * 1024 * 1024, VK_BUFFER_USAGE_VERTEX_BUFFER_BIT | buffer_usage, memory_properties, &state->vertex_buffer)) {
        throw RendererException("Failed to create engine's global vertex buffer");
    }

    if (!create_gpu_buffer(state, sizeof(uint32_t) * 1024 * 1024, VK_BUFFER_USAGE_INDEX_BUFFER_BIT | buffer_usage, memory_properties, &state->index_buffer)) {
        throw RendererException("Failed to create engine's global index buffer");
    }

    state->global_ubo = (GlobalUniformObject*)Platform::AAlloc(16, sizeof(*state->global_ubo) * state->num_frames); 

    if (!create_gpu_buffer(
        state, 
        align_uniform_buffer_size(uint64_t(sizeof(*state->global_ubo) * state->num_frames), state->min_ubo_alignment), 
        VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_TRANSFER_SRC_BIT | VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,
        VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
        &state->global_uniform_buffer)) {
        throw RendererException("Failed to create global uniform buffer");
    }

    VkClearValue color_clear_value = *(VkClearValue*)&DirectX::Colors::BlueViolet;

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

    destroy_gpu_buffer(state, &state->global_uniform_buffer);
    Platform::AFree(state->global_ubo);
    destroy_gpu_buffer(state, &state->index_buffer);
    destroy_gpu_buffer(state, &state->vertex_buffer);
    destroy_swapchain_framebuffers(state);
    destroy_render_pass(state);
    destroy_command_pool(state, state->command_pool);
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

FrameStatus vulkan_begin_frame() {
    VkFence fence = state->fences[state->current_frame_index];
    VkSemaphore queue_complete = state->queue_complete_semaphore[state->current_frame_index];
    VkSemaphore image_acquired = state->image_acquired_semaphore[state->current_frame_index];
    VkCommandBuffer command_buffer = state->graphics_command_buffers[state->current_frame_index];
    VkRenderPass renderpass = state->main_renderpass;
    const VkSurfaceCapabilitiesKHR& surface_capabilities = state->surface_capabilities;
    const VkClearValue* clear_values = state->clear_values;

    VkResult result;

    result = vkWaitForFences(state->logical_device, 1, &fence, VK_TRUE, UINT64_MAX);

    if (result != VK_SUCCESS) {
        return FRAME_STATUS_FAILED;
    }

    if (!state->swapchain) {
        recreate_swapchain(state);
        return FRAME_STATUS_SKIP;
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
        return FRAME_STATUS_SKIP;
    }

    result = vkResetFences(state->logical_device, 1, &fence);

    if (result != VK_SUCCESS) {
        return FRAME_STATUS_FAILED;
    }

    VkFramebuffer framebuffer = state->swapchain_framebuffers[state->image_index];

    vkResetCommandBuffer(command_buffer, 0);
    begin_command_buffer(command_buffer);

    set_viewport_and_scissor(command_buffer, &state->viewport, &state->scissor);

    begin_render_pass(renderpass, 
        command_buffer, 
        framebuffer, 
        state->scissor, 
        (uint32_t)std::size(state->clear_values), 
        clear_values);

    return FRAME_STATUS_SUCCESS;
}

FrameStatus vulkan_draw_items() {
    VkCommandBuffer command_buffer = state->graphics_command_buffers[state->current_frame_index];

    state->current_shader = nullptr;
    update_uniform_buffer();

    for (uint32_t i = 0; i < state->render_items.size_u32(); i++) {
        render_item* item = &state->render_items[i];
        
        vkCmdBindIndexBuffer(command_buffer, state->index_buffer.buffer, item->index_buffer_offset, VK_INDEX_TYPE_UINT32);
        vkCmdBindVertexBuffers(command_buffer, 0, 1, &state->vertex_buffer.buffer, &item->vertex_buffer_offset);
        vulkan_shader_bundle* shader = &item->shader_bundle;
        if (shader != state->current_shader) {
            vulkan_shader_use(state, command_buffer, shader);
            state->current_shader = shader;
        }    

        vkCmdPushConstants(
            command_buffer, 
            shader->pipeline.layout, 
            VK_SHADER_STAGE_VERTEX_BIT, 
            0, 
            sizeof(item->model), 
            &item->model);

        vkCmdDrawIndexed(command_buffer, item->index_count, 1, 0, 0, 0);
    }
    
    return FRAME_STATUS_SUCCESS;
}

FrameStatus vulkan_end_frame() {
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
        return FRAME_STATUS_SKIP;
    } else if (result != VK_SUCCESS) {
        return FRAME_STATUS_FAILED;
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
        return FRAME_STATUS_SKIP;
    } else if (result != VK_SUCCESS) {
        return FRAME_STATUS_FAILED;
    }

    return FRAME_STATUS_SUCCESS;
}

HANDLE vulkan_create_render_item(const RenderItemCreateInfo* pRenderItemCreateInfo) {
    if (pRenderItemCreateInfo == nullptr) {
        Logger::warning("vulkan_create_render_item: pRenderItemCreateInfo can't be nullptr");
        return nullptr;
    }
    if (pRenderItemCreateInfo->pIndices == 0 || pRenderItemCreateInfo->pVertices == 0) {
        Logger::warning("vulkan_create_render_item: RenderItemCreateInfo->pIndices and/or RenderItemCreateInfo->pMeshes can't be nullptr");
        return nullptr;
    }

    VkCommandBuffer command_buffer;
    create_one_time_command_buffer(state, &command_buffer);

    gpu_buffer vertex_uploader;
    gpu_buffer index_uploader;

    // push an empty render item to the list, and the get the index of it.
    state->render_items.push_back(render_item());
    render_item* r_item = &state->render_items[state->render_items.size() - 1];
    
    r_item->index_count = pRenderItemCreateInfo->indicesCount;
    r_item->vertices_count = pRenderItemCreateInfo->verticesCount;
    r_item->vertex_buffer_offset = state->geometry_vertex_offset;
    r_item->index_buffer_offset = state->geometry_index_offset;
    r_item->texture = (vulkan_texture*)pRenderItemCreateInfo->texture;

    state->geometry_vertex_offset += pRenderItemCreateInfo->vertexSize;
    state->geometry_index_offset += pRenderItemCreateInfo->indexSize;

    if (!create_uploader_buffer(state, pRenderItemCreateInfo->vertexSize, &vertex_uploader)) {
        Logger::warning("vulkan_create_render_item: Failed to create vertex buffer");
        return nullptr;
    }
    
    if (!create_uploader_buffer(state, pRenderItemCreateInfo->indexSize, &index_uploader)) {
        Logger::warning("vulkan_create_render_item: Failed to create vertex buffer");
        vulkan_destroy_render_item(r_item);
        return nullptr;
    }

    VkShaderStageFlagBits stages[2] = {
        VK_SHADER_STAGE_VERTEX_BIT,
        VK_SHADER_STAGE_FRAGMENT_BIT
    };

    list<const char*> paths;
    paths.push_back("mvp_vert.spv");

    list<vulkan_shader_binding_create_info> bindings;

    if (pRenderItemCreateInfo->texture) {
        paths.push_back("tex_frag.spv");
        uint64_t index = bindings.size();
        bindings.push_back(vulkan_shader_binding_create_info());
        bindings[index].binding = 1;
        bindings[index].descriptor_count = 1;
        bindings[index].descriptor_type = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
        bindings[index].stage = VK_SHADER_STAGE_FRAGMENT_BIT;
    } else {
        paths.push_back("static_frag.spv");
    }

    vulkan_shader_bundle_create_info shader_create_info;
    shader_create_info.shader_count = 2;
    shader_create_info.stages = stages;
    shader_create_info.entry_point = "main";
    shader_create_info.paths = paths.data();
    shader_create_info.binding_count = bindings.size_u32();
    shader_create_info.bindings = bindings.data();

    if (!create_vulkan_shader_bundle(state, &shader_create_info, &r_item->shader_bundle)) {
        Logger::warning("vulkan_create_render_item: Failed to create render item shader");
        vulkan_destroy_render_item(r_item);
        return nullptr;
    }

    for (uint32_t i = 0; i < state->num_frames; i++) {
        // TODO: Move this to shader
        update_descriptor_set_for_buffer(
            state, 
            state->global_uniform_buffer.buffer, 
            0, 
            state->global_uniform_buffer.size, 
            &r_item->shader_bundle.shader_descriptor_sets[i], 
            0
        );

        update_descriptor_set_for_texture(
            state, 
            r_item->texture, 
            &r_item->shader_bundle.shader_descriptor_sets[i], 
            1
        );
    }

    copy_to_upload_buffer(state, pRenderItemCreateInfo->pVertices, pRenderItemCreateInfo->vertexSize, &vertex_uploader);
    copy_to_upload_buffer(state, pRenderItemCreateInfo->pIndices, pRenderItemCreateInfo->indexSize, &index_uploader);

    copy_to_gpu_buffer(command_buffer, &vertex_uploader, &state->vertex_buffer, r_item->vertex_buffer_offset);
    copy_to_gpu_buffer(command_buffer, &index_uploader, &state->index_buffer, r_item->index_buffer_offset);

    end_one_time_command_buffer(state, command_buffer, state->graphics_queue);

    destroy_gpu_buffer(state, &vertex_uploader);
    destroy_gpu_buffer(state, &index_uploader);

    return r_item;
}

void vulkan_destroy_render_item(HANDLE item_handle) {
    vkDeviceWaitIdle(state->logical_device);
    render_item* r_item = (render_item*)item_handle;

    destroy_vulkan_shader_bundle(state, &r_item->shader_bundle);

    Platform::ZeroMemory(r_item, sizeof(*r_item));

    r_item->index_buffer_offset = -1;
    r_item->vertex_buffer_offset = -1;
}

void vulkan_set_view_projection(DirectX::XMMATRIX view_matrix, DirectX::CXMMATRIX projection_matrix) {
    state->global_ubo->view = view_matrix;
    state->global_ubo->projection = projection_matrix;
}

void vulkan_update_render_item(HANDLE render_item, const DirectX::XMFLOAT4X4* render_data) {
    struct render_item* item = (struct render_item*)render_item;
    item->model = *render_data;
}

HANDLE vulkan_create_texture(const char* name, bool auto_release, uint32_t width, uint32_t height, 
                             uint32_t channel_count, const uint8_t* pixels, bool has_transparency) {
    VkCommandBuffer command_buffer;

    // TODO: Change this in the future.
    vulkan_texture* texture = (vulkan_texture*)Platform::UAlloc(sizeof(vulkan_texture));
    texture->image.name = name;

    VkExtent3D image_extent;
    image_extent.width = width;
    image_extent.height = height;
    image_extent.depth = 1;

    uint64_t image_size = width * height * channel_count;

    create_one_time_command_buffer(state, &command_buffer);

    create_image(state, VK_IMAGE_TYPE_2D, VK_FORMAT_B8G8R8A8_UNORM, 
        &image_extent, state->mip_levels, VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | 
        VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT, 
        VK_IMAGE_LAYOUT_UNDEFINED, &texture->image);

    create_image_view(state, &texture->image, VK_IMAGE_VIEW_TYPE_2D, VK_IMAGE_ASPECT_COLOR_BIT);

    transition_image_layout(state, command_buffer, &texture->image, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL);

    gpu_buffer upload_buffer;
    create_uploader_buffer(state, image_size, &upload_buffer);
    copy_to_upload_buffer(state, pixels, image_size, &upload_buffer);

    copy_buffer_to_image(command_buffer, &texture->image, &upload_buffer);

    transition_image_layout(state, command_buffer, &texture->image, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);

    end_one_time_command_buffer(state, command_buffer, state->graphics_queue);

    destroy_gpu_buffer(state, &upload_buffer);

    VkSamplerCreateInfo create_info;
    create_info.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
    create_info.pNext = nullptr;
    create_info.flags = 0;
    create_info.magFilter = VK_FILTER_LINEAR;
    create_info.minFilter = VK_FILTER_LINEAR;
    create_info.mipmapMode = VK_SAMPLER_MIPMAP_MODE_NEAREST;
    create_info.addressModeU = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
    create_info.addressModeV = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
    create_info.addressModeW = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
    create_info.mipLodBias = 0.0f;
    create_info.anisotropyEnable = VK_TRUE;
    create_info.maxAnisotropy = 16.0f;
    create_info.compareEnable = VK_FALSE;
    create_info.compareOp = VK_COMPARE_OP_LESS;
    create_info.minLod = 1.0f;
    create_info.maxLod = 16.0f;
    create_info.borderColor = VK_BORDER_COLOR_FLOAT_TRANSPARENT_BLACK;
    create_info.unnormalizedCoordinates = VK_FALSE;

    VkResult res = vkCreateSampler(state->logical_device, &create_info, state->allocator, &texture->sampler);

    if (res != VK_SUCCESS) {
        vulkan_destroy_texture(texture);
        Logger::warning("vulkan_create_texture: Failed to create texture sampler!");
        return nullptr;
    }

    return texture;
}

void vulkan_destroy_texture(HANDLE _texture) {
    vulkan_texture* texture = (vulkan_texture*)_texture;
    vulkan_image* image = &texture->image;

    destroy_image_view(state, image);
    destroy_image(state, image);
    vkDestroySampler(state->logical_device, texture->sampler, state->allocator);
    Platform::UFree(texture);
}

void vulkan_wait_device_idle() {
    vkDeviceWaitIdle(state->logical_device);
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

bool set_viewport_and_scissor(VkCommandBuffer command_buffer, const VkViewport* viewport, const VkRect2D* scissor) {
    vkCmdSetViewport(command_buffer, 0, 1, viewport);
    vkCmdSetScissor(command_buffer, 0, 1, scissor);

    return true;
}

bool update_uniform_buffer() {
    uint8_t* memory_pointer = (uint8_t*)state->global_uniform_buffer.memory_pointer;
    memory_pointer += state->current_frame_index * sizeof(*state->global_ubo);

    memcpy(memory_pointer, state->global_ubo, sizeof(*state->global_ubo));

    return true;
}

}

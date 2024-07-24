#pragma once

#include <renderer/renderer_types.h>
#include <defines.h>

#include <cstdint>

struct RenderItemCreateInfo;

extern "C" {
DYNAMIC_RENDERER bool vulkan_backend_initialize(uint64_t* required_size, HANDLE allocated_memory, const char* name, HANDLE sdl_window) noexcept(false);
DYNAMIC_RENDERER void vulkan_backend_shutdown();
DYNAMIC_RENDERER bool vulkan_begin_frame();
DYNAMIC_RENDERER bool vulkan_draw_items();
DYNAMIC_RENDERER bool vulkan_end_frame();
DYNAMIC_RENDERER HANDLE vulkan_create_render_item(const RenderItemCreateInfo* pRenderItemCreateInfo);
DYNAMIC_RENDERER void vulkan_destroy_render_item(HANDLE render_item);
DYNAMIC_RENDERER void set_view_projection(const void* view_matrix, const void* projection_matrix);
DYNAMIC_RENDERER HANDLE vulkan_create_texture(HANDLE state, uint32_t width, uint32_t height);
}
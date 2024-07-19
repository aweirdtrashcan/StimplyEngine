#pragma once

#include <cstdint>

struct RenderItemCreateInfo;

extern "C" {
DYNAMIC_RENDERER bool vulkan_backend_initialize(uint64_t* required_size, void* allocated_memory, const char* name, void* sdl_window);
DYNAMIC_RENDERER void vulkan_backend_shutdown();
DYNAMIC_RENDERER bool vulkan_begin_frame();
DYNAMIC_RENDERER bool vulkan_end_frame();
DYNAMIC_RENDERER void* vulkan_create_render_item(const RenderItemCreateInfo* pRenderItemCreateInfo);
DYNAMIC_RENDERER void vulkan_destroy_render_item(void* render_item);
DYNAMIC_RENDERER void* vulkan_create_texture(void* state, uint32_t width, uint32_t height);
}
#pragma once

#include "DirectXMath.h"
#include <renderer/renderer_types.h>
#include <defines.h>

#include <cstdint>

struct RenderItemCreateInfo;

extern "C" {
DYNAMIC_RENDERER bool vulkan_backend_initialize(uint64_t* required_size, HANDLE allocated_memory, const char* name, HANDLE sdl_window) noexcept(false);
DYNAMIC_RENDERER void vulkan_backend_shutdown();
DYNAMIC_RENDERER FrameStatus vulkan_begin_frame();
DYNAMIC_RENDERER FrameStatus vulkan_draw_items();
DYNAMIC_RENDERER FrameStatus vulkan_end_frame();
DYNAMIC_RENDERER HANDLE vulkan_create_render_item(const RenderItemCreateInfo* pRenderItemCreateInfo);
DYNAMIC_RENDERER void vulkan_destroy_render_item(HANDLE render_item);
DYNAMIC_RENDERER void vulkan_set_view_projection(DirectX::XMMATRIX view_matrix, DirectX::CXMMATRIX projection_matrix);
DYNAMIC_RENDERER void vulkan_set_render_item_model(HANDLE render_item, const DirectX::XMFLOAT4X4* model_matrix);
DYNAMIC_RENDERER HANDLE vulkan_create_texture(const char* name, bool auto_release, uint32_t width, uint32_t height, 
                                     		  uint32_t channel_count, const uint8_t* pixels, bool has_transparency);
DYNAMIC_RENDERER void vulkan_destroy_texture(HANDLE texture);
}
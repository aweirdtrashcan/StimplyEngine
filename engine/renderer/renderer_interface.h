#pragma once

#include <cstdint>
#include "DirectXMath.h"
#include "defines.h"
#include "renderer/renderer_types.h"

typedef HANDLE renderer_state;
typedef HANDLE Texture;
typedef void(*PFN_logger)(const char* message, ...);

/* In the vulkan renderer, make sure that the extra parameter is a pointer to an array of required instance layers. */
typedef bool(*PFN_renderer_backend_initialize)(uint64_t* required_size, HANDLE allocated_memory, const char* name, HANDLE sdl_window);
typedef void(*PFN_renderer_backend_shutdown)();
typedef FrameStatus(*PFN_renderer_begin_frame)();
typedef FrameStatus(*PFN_renderer_end_frame)();
typedef Texture(*PFN_renderer_create_texture)(const char* name, bool auto_release, uint32_t width, uint32_t height, 
                                     uint32_t channel_count, const uint8_t* pixels, bool has_transparency);
typedef Texture(*PFN_renderer_destroy_texture)(Texture texture);
typedef HANDLE(*PFN_renderer_create_render_item)(const struct RenderItemCreateInfo* render_item);
typedef void(*PFN_renderer_destroy_render_item)(HANDLE render_item);
typedef FrameStatus(*PFN_renderer_draw_items)();
typedef void(*PFN_renderer_set_view_projection)(DirectX::XMMATRIX view_matrix, DirectX::CXMMATRIX projection_matrix);
typedef void(*PFN_renderer_set_render_item_model)(HANDLE render_item, const DirectX::XMFLOAT4X4* model_matrix);

typedef struct renderer_interface {
    PFN_renderer_backend_initialize renderer_initialize;
    PFN_renderer_backend_shutdown renderer_shutdown;
    PFN_renderer_begin_frame renderer_begin_frame;
    PFN_renderer_end_frame renderer_end_frame;
    PFN_renderer_create_texture renderer_create_texture;
    PFN_renderer_destroy_texture renderer_destroy_texture;
    PFN_renderer_create_render_item renderer_create_render_item;
    PFN_renderer_destroy_render_item renderer_destroy_render_item;
    PFN_renderer_draw_items renderer_draw_items;
    PFN_renderer_set_view_projection renderer_set_view_projection;
    PFN_renderer_set_render_item_model renderer_set_render_item_model;
} renderer_interface;
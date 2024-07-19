#pragma once

#include <cstdint>

typedef void* renderer_state;
typedef void* renderer_image;
typedef void(*PFN_logger)(const char* message, ...);

/* In the vulkan renderer, make sure that the extra parameter is a pointer to an array of required instance layers. */
typedef bool(*PFN_renderer_backend_initialize)(uint64_t* required_size, void* allocated_memory, const char* name, void* sdl_window);
typedef void(*PFN_renderer_backend_shutdown)();
typedef bool(*PFN_renderer_begin_frame)();
typedef bool(*PFN_renderer_end_frame)();
typedef renderer_image(*PFN_create_texture)(void* state, uint32_t width, uint32_t height);
typedef void*(*PFN_renderer_create_render_item)(const struct RenderItemCreateInfo* render_item);
typedef void(*PFN_renderer_destroy_render_item)(void* render_item);

typedef struct renderer_interface {
    PFN_renderer_backend_initialize initialize;
    PFN_renderer_backend_shutdown shutdown;
    PFN_renderer_begin_frame begin_frame;
    PFN_renderer_end_frame end_frame;
    PFN_create_texture create_texture;
    PFN_renderer_create_render_item renderer_create_render_item;
    PFN_renderer_destroy_render_item renderer_destroy_render_item;
} renderer_interface;
#pragma once

#include <cstdint>

extern "C" {
DYNAMIC_RENDERER bool d3d12_backend_initialize(uint64_t* required_size, void* allocated_memory, const char* name, void* sdl_window) noexcept(false);
DYNAMIC_RENDERER void d3d12_backend_shutdown();
DYNAMIC_RENDERER bool d3d12_begin_frame();
DYNAMIC_RENDERER bool d3d12_end_frame();
DYNAMIC_RENDERER void* d3d12_create_texture(void* state, uint32_t width, uint32_t height);
}
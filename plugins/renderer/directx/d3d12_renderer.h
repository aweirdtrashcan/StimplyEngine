#pragma once

#include <cstdint>

extern "C" {
typedef void(*PFN_logger)(const char* message, ...);
DYNAMIC_RENDERER bool d3d12_backend_initialize(uint64_t* required_size, void* allocated_memory, const char* name,
    PFN_logger pfn_fatal, PFN_logger pfn_warning, PFN_logger pfn_debug, PFN_logger pfn_info, void* sdl_window);
DYNAMIC_RENDERER void d3d12_backend_shutdown();
DYNAMIC_RENDERER bool d3d12_begin_frame();
DYNAMIC_RENDERER bool d3d12_end_frame();
DYNAMIC_RENDERER void* d3d12_create_texture(void* state, uint32_t width, uint32_t height);
}
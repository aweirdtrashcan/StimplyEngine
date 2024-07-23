#pragma once

#include <containers/list.h>

#include "vulkan/vulkan_defines.h"

struct render_item {
    gpu_buffer vertex_buffer;
    gpu_buffer index_buffer;
    uint32_t index_count;
    uint32_t vertices_count;
    vulkan_shader* shader_object;
};
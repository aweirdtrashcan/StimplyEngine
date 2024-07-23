#pragma once

#include <containers/list.h>

#include "vulkan/vulkan_defines.h"

struct render_item {
    uint32_t index_count;
    uint32_t vertices_count;
    vulkan_shader* shader_object;
    uint64_t vertex_buffer_offset;
    uint64_t index_buffer_offset;
};
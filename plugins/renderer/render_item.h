#pragma once

#include "vulkan/vulkan_defines.h"

struct render_item {
    gpu_buffer vertex_buffer;
    gpu_buffer index_buffer;
};
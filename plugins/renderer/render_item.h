#pragma once

#include <defines.h>

#include "vulkan/vulkan_defines.h"

#include <renderer/renderer_types.h>

struct render_item {
    uint32_t index_count;
    uint32_t vertices_count;
    uint64_t vertex_buffer_offset;
    uint64_t index_buffer_offset;
    DirectX::XMFLOAT4X4 model;
    LocalUniformObject object_buffer;
    vulkan_texture* texture;
    vulkan_shader_bundle shader_bundle;
};